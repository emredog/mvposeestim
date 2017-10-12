#include "filter_response.h"
//#include "logging.h"


//---------------------------------------------------------

__global__ void kernel_fconv(FPTYPE *gpuA, int A_dim0, int A_dim1, int A_dim2, FPTYPE *gpuB, int B_dim0, int B_dim1, int B_dim2, FPTYPE *gpuCi, int C_dim0, int C_dim1, int num_features)
{
	int f = blockIdx.z*blockDim.z + threadIdx.z;
    int x = blockIdx.x*blockDim.x + threadIdx.x;
    int y = blockIdx.y*blockDim.y + threadIdx.y;

	if( x >= C_dim1 || y >= C_dim0 || f >= num_features )
		return; // skip padding thread

	FPTYPE *dst = gpuCi + f*C_dim0*C_dim1 ;
	FPTYPE *A_src = gpuA + f*A_dim0*A_dim1;      
	FPTYPE *B_src = gpuB + f*B_dim0*B_dim1;
	FPTYPE val = 0;
	for(int xp = 0; xp < B_dim1; xp++) 
	{
		FPTYPE *A_off = A_src + (x+xp)*A_dim0 + y;
		FPTYPE *B_off = B_src + xp*B_dim0;
		for(int yp = 0; yp < B_dim0; yp++)
			val += *(A_off++) * *(B_off++);
	}
	dst[x*C_dim0 + y] = val;
}

__global__ void kernel_sum_Ci(FPTYPE *gpuCi, int C_dim0, int C_dim1, int num_features)
{
    int x = blockIdx.x*blockDim.x + threadIdx.x;
    int y = blockIdx.y*blockDim.y + threadIdx.y;

	if( x >= C_dim1 || y >= C_dim0 )
		return; // skip padding thread

	FPTYPE val = 0;
	for(int f = 0; f < num_features; f++)
		val += gpuCi[f*C_dim0*C_dim1 + x*C_dim0 + y];

	gpuCi[x*C_dim0 + y] = val;
}

//---------------------------------------------------------

#define BKSIZE 4
const int STREAMSNBR = 2;

// divide x by y and round up result
#define divup(x,y)  1+(((x)-1)/(y))  // if x != 0 only

cudaStream_t stream[STREAMSNBR];

void initCuda(void)
{
	for(int i = 0; i < STREAMSNBR; i++)
    	cudaStreamCreate(&stream[i]);
}

void releaseCuda(void)
{
	for(int i = 0; i < STREAMSNBR; i++)
		cudaStreamDestroy(stream[i]);
	// cudaDeviceReset causes the driver to clean up all state. While
	// not mandatory in normal operation, it is good practice.  It is also
	// needed to ensure correct operation when the application is being
	// profiled. Calling cudaDeviceReset causes all profile data to be
	// flushed before the application exits
	cudaDeviceReset();
}

myArray<FPTYPE>** fconv_cuda(const myArray<FPTYPE> *pyra_feat_level, myArray<FPTYPE> **filters, int start, int end)
{
	// get A (features)
	const myArray<FPTYPE> *mxA = pyra_feat_level;
	FPTYPE *A = mxA->getPtr();
	int num_features = mxA->getPlanes();
	int A_dim0 = mxA->getRows();
	int A_dim1 = mxA->getCols();
	int A_dim2 = mxA->getPlanes();

	// get B (filters) and start/end
	myArray<FPTYPE> **cellB = filters;
	start = start - 1;
	end = end - 1;
	int len = end - start + 1;

	// allocate output
	myArray<FPTYPE> **C = (myArray<FPTYPE>**) calloc(len, sizeof(myArray<FPTYPE>*));

	// use 2 streams to overlapp memory transfer and computation
	int currentStream = 0;
	int nextStream = 1;

	// copy A on GPU
	int A_bytes = mxA->getNumberOfElements() * sizeof(FPTYPE);
	FPTYPE *gpuA;
	cudaMalloc((void**) &gpuA, A_bytes);
	cudaMemcpyAsync(gpuA, A, A_bytes, cudaMemcpyHostToDevice, stream[currentStream]);

	// memory for B anc Ci on GPU is allocated only at the first iteration
	// because B and Ci size decrease

	// allocate memory for output C[i] on GPU (1 C[i] per feature)
	// C[i] is smaller than A, so we use the size of A to over-allocate
	// memory for C
	// allocate gpu memory just the first time, and not at each iteration
	FPTYPE *gpuCi[STREAMSNBR] = { NULL };
	int Ci_bytes_max = A_dim0 * A_dim1 * num_features * sizeof(FPTYPE);
	for(int i = 0; i < STREAMSNBR; i++)
	{
		cudaMalloc((void**) &gpuCi[i], Ci_bytes_max);
		cudaMemset((void*) gpuCi[i], 0, Ci_bytes_max);
	}

	// pre-load data on GPU for first iteration
	FPTYPE *gpuB[STREAMSNBR] = { NULL };
	const myArray<FPTYPE> *mxB = cellB[0+start];
	FPTYPE *B = mxB->getPtr();
	int B_dim0 = mxB->getRows();
	int B_dim1 = mxB->getCols();
	int B_dim2 = mxB->getPlanes();

	// copy first B array on GPU
	// allocate gpu memory just the first time, and not at each iteration
	int B_bytes_first = mxB->getNumberOfElements() * sizeof(FPTYPE);
	for(int i = 0; i < STREAMSNBR; i++)
		cudaMalloc((void**) &gpuB[i], B_bytes_first);
	cudaMemcpyAsync(gpuB[currentStream], B, B_bytes_first, cudaMemcpyHostToDevice, stream[currentStream]);

	// loop over parts and filters
	for (int i = 0; i < len; i++)
	{
		if( A_dim2 != B_dim2 )
			throw std::runtime_error("fconv_cuda(): invalid input B");

		nextStream = (currentStream+1) % STREAMSNBR;

		// pre-load B on GPU for next iteration
		if( i < len-1 )
		{
			mxB = cellB[i+1+start];
			B = mxB->getPtr();
			int B_bytes = mxB->getNumberOfElements() * sizeof(FPTYPE);
			cudaMemcpyAsync(gpuB[nextStream], B, B_bytes, cudaMemcpyHostToDevice, stream[nextStream]);
		}

		// compute size of output
		int height = A_dim0 - B_dim0 + 1;
		int width = A_dim1 - B_dim1 + 1;
		if( height < 1 || width < 1 )
			throw std::runtime_error("fconv_cuda(): invalid input: B should be smaller than A");
		int C_dim0 = height;
		int C_dim1 = width;

		// compute C[i] for all features
		dim3 dimBlock(BKSIZE, BKSIZE, 32);
		dim3 dimGrid(divup(width, dimBlock.x), divup(height, dimBlock.y), divup(num_features,dimBlock.z));
		//printf("w, h, f = %d %d %d\n", width, height, num_features);
		//printf("dimBlock = %d %d %d\n", dimBlock.x, dimBlock.y, dimBlock.z);
		//printf("dimGrid = %d %d %d\n", dimGrid.x, dimGrid.y, dimGrid.z);
		kernel_fconv<<< dimGrid, dimBlock, 0, stream[currentStream] >>>(gpuA, A_dim0, A_dim1, A_dim2, gpuB[currentStream], B_dim0, B_dim1, B_dim2, gpuCi[currentStream], C_dim0, C_dim1, num_features);

		//DBG
		/*
		myArray<FPTYPE> *tmpCis = new myArray<FPTYPE>(A_dim0, A_dim1, num_features);
		FPTYPE *tmpCisPtr = tmpCis->getPtr();
		cudaMemcpyAsync(tmpCisPtr, gpuCi, Ci_bytes_max, cudaMemcpyDeviceToHost, stream[currentStream]);
		writeLog("fconv_cuda tmpCis=");
		writeLog(tmpCis);
		delete tmpCis;
		*/

		// sum features contributions
		dim3 dimBlock2(BKSIZE, BKSIZE);
		dim3 dimGrid2(divup(width, dimBlock.y), divup(height, dimBlock.x));
		kernel_sum_Ci<<< dimGrid2, dimBlock2, 0, stream[currentStream] >>>(gpuCi[currentStream], C_dim0, C_dim1, num_features);

		// transfer result from GPU memory to CPU memory
		C[i] = new myArray<FPTYPE>(height, width, 1, true);
		FPTYPE *Ci = C[i]->getPtr();
		int Ci_bytes = C[i]->getNumberOfElements() * sizeof(FPTYPE);
		cudaMemcpyAsync(Ci, gpuCi[currentStream], Ci_bytes, cudaMemcpyDeviceToHost, stream[currentStream]);

		// prepare next iteration
		currentStream = nextStream;
		B_dim0 = mxB->getRows();
		B_dim1 = mxB->getCols();
		B_dim2 = mxB->getPlanes();
	}

	for(int i = 0; i < STREAMSNBR; i++)
		cudaStreamSynchronize(stream[i]);

	// release GPU memory
	for(int i = 0; i < STREAMSNBR; i++)
	{
		cudaFree(gpuCi[i]);
		cudaFree(gpuB[i]);
	}
	cudaFree(gpuA);

	return C;
}

void cudaWakeUp(void)
{
	// make a dummy memory allocation to wake up NVIDIA driver
	// before starting time measurement, in order to make fair comparison
	int *gpuWakeUp;
	cudaMalloc((void**) &gpuWakeUp, 1);
	cudaFree(gpuWakeUp);
}
