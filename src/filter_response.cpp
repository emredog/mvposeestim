#include "filter_response.h"

#if 1

// original version

//--------------------------------------------------------------------

// filter response computation

// source: mex_unix/fconvMT.cc

/*
 * This code is used for computing filter responses.  It computes the
 * response of a set of filters with a feature map.  
 *
 * Multithreaded version.
 */

struct thread_data
{
	FPTYPE *A;
	FPTYPE *B;
	FPTYPE *C;
	myArray<FPTYPE> *mxC;
	int A_dims[3];
	int B_dims[3];
	int C_dims[2];
};

// convolve A and B
void *process_fconvMT(void *thread_arg)
{
	thread_data *args = (thread_data *)thread_arg;
	FPTYPE *A = args->A; // A is of dimension 3
	FPTYPE *B = args->B; // A is of dimension 3
	FPTYPE *C = args->C;
	const int *A_dims = args->A_dims;
	const int *B_dims = args->B_dims;
	const int *C_dims = args->C_dims;
	int num_features = args->A_dims[2];

	for (int f = 0; f < num_features; f++) {
		FPTYPE *dst = C;
		FPTYPE *A_src = A + f*A_dims[0]*A_dims[1];      
		FPTYPE *B_src = B + f*B_dims[0]*B_dims[1];
		for (int x = 0; x < C_dims[1]; x++) {
			for (int y = 0; y < C_dims[0]; y++) {
				FPTYPE val = 0;
				for (int xp = 0; xp < B_dims[1]; xp++) {
					FPTYPE *A_off = A_src + (x+xp)*A_dims[0] + y;
					FPTYPE *B_off = B_src + xp*B_dims[0];
					switch(B_dims[0]) {
						case 20: val += A_off[19] * B_off[19];
						case 19: val += A_off[18] * B_off[18];
						case 18: val += A_off[17] * B_off[17];
						case 17: val += A_off[16] * B_off[16];
						case 16: val += A_off[15] * B_off[15];
						case 15: val += A_off[14] * B_off[14];
						case 14: val += A_off[13] * B_off[13];
						case 13: val += A_off[12] * B_off[12];
						case 12: val += A_off[11] * B_off[11];
						case 11: val += A_off[10] * B_off[10];
						case 10: val += A_off[9] * B_off[9];
						case 9: val += A_off[8] * B_off[8];
						case 8: val += A_off[7] * B_off[7];
						case 7: val += A_off[6] * B_off[6];
						case 6: val += A_off[5] * B_off[5];
						case 5: val += A_off[4] * B_off[4];
						case 4: val += A_off[3] * B_off[3];
						case 3: val += A_off[2] * B_off[2];
						case 2: val += A_off[1] * B_off[1];
						case 1: val += A_off[0] * B_off[0];
								break;
						default:	    	      
								for (int yp = 0; yp < B_dims[0]; yp++) {
									val += *(A_off++) * *(B_off++);
								}
					}
				}
				*(dst++) += val;
			}
		}
	}
	pthread_exit(NULL);
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

/*
 * Compute filter response.
 */
myArray<FPTYPE>** fconvMT_C(const myArray<FPTYPE> *pyra_feat_level, myArray<FPTYPE> **filters, int start, int end)
{
	// get A
	const myArray<FPTYPE> *mxA = pyra_feat_level;

	// get B and start/end
	myArray<FPTYPE> **cellB = filters;
	start = start -1;
	end = end -1;
	int len = end-start+1;

	// start threads
	thread_data *td = (thread_data *) calloc(len, sizeof(thread_data));
	pthread_t *ts = (pthread_t *) calloc(len, sizeof(pthread_t));
	FPTYPE *A = mxA->getPtr();
	for (int i = 0; i < len; i++)
	{
		const myArray<FPTYPE> *mxB = cellB[i+start];
		td[i].A_dims[0] = mxA->getRows();
		td[i].A_dims[1] = mxA->getCols();
		td[i].A_dims[2] = mxA->getPlanes();
		td[i].A = A;
		td[i].B_dims[0] = mxB->getRows();
		td[i].B_dims[1] = mxB->getCols();
		td[i].B_dims[2] = mxB->getPlanes();
		td[i].B = mxB->getPtr();
		if( td[i].A_dims[2] != td[i].B_dims[2] )
			throw std::runtime_error("fconvMT_C(): invalid input B");

		// compute size of output
		int height = td[i].A_dims[0] - td[i].B_dims[0] + 1;
		int width = td[i].A_dims[1] - td[i].B_dims[1] + 1;
		if (height < 1 || width < 1)
			throw std::runtime_error("fconvMT_C(): invalid input: B should be smaller than A");
		td[i].C_dims[0] = height;
		td[i].C_dims[1] = width;
		td[i].mxC = new myArray<FPTYPE>(height, width);
		td[i].C = td[i].mxC->getPtr();

		if (pthread_create(&ts[i], NULL, process_fconvMT, (void *)&td[i]))
			throw std::runtime_error("fconvMT_C(): error creating thread");
	}

	// wait for the treads to finish and set return values
	void *status;
	myArray<FPTYPE> **C = (myArray<FPTYPE>**) calloc(len, sizeof(myArray<FPTYPE>*));

	for (int i = 0; i < len; i++) {
		pthread_join(ts[i], &status);
		C[i] = td[i].mxC;
	}
	free(td);
	free(ts);

	return C;
}

#else

// simpler (and slower) version

myArray<FPTYPE>** fconvMT_C(const myArray<FPTYPE> *pyra_feat_level, myArray<FPTYPE> **filters, int start, int end)
{
	// get A
	const myArray<FPTYPE> *mxA = pyra_feat_level;
	FPTYPE *A = mxA->getPtr();
	int num_features = mxA->getPlanes();
	int A_dim0 = mxA->getRows();
	int A_dim1 = mxA->getCols();
	int A_dim2 = mxA->getPlanes();

	// get B and start/end
	myArray<FPTYPE> **cellB = filters;
	start = start -1;
	end = end -1;
	int len = end-start+1;

	// allocate output
	myArray<FPTYPE> **C = (myArray<FPTYPE>**) calloc(len, sizeof(myArray<FPTYPE>*));

	for (int i = 0; i < len; i++)
	{
		const myArray<FPTYPE> *mxB = cellB[i+start];
		FPTYPE *B = mxB->getPtr();
		int B_dim0 = mxB->getRows();
		int B_dim1 = mxB->getCols();
		int B_dim2 = mxB->getPlanes();
		if( A_dim2 != B_dim2 )
			throw std::runtime_error("fconvMT_C(): invalid input B");

		// compute size of output
		int height = A_dim0 - B_dim0 + 1;
		int width = A_dim1 - B_dim1 + 1;
		if( height < 1 || width < 1 )
			throw std::runtime_error("fconvMT_C(): invalid input: B should be smaller than A");
		int C_dim0 = height;
		int C_dim1 = width;
		C[i] = new myArray<FPTYPE>(height, width);
		FPTYPE *Ci = C[i]->getPtr();

		
		myArray<FPTYPE> *Ci_allf = new myArray<FPTYPE>(height, width, num_features);
		FPTYPE *Ci_allf_ptr = Ci_allf->getPtr();

		// compute convolution independently for each feature
		for (int f = 0; f < num_features; f++) 
		{
			FPTYPE *A_src = A + f*A_dim0*A_dim1;      
			FPTYPE *B_src = B + f*B_dim0*B_dim1;
			for (int x = 0; x < C_dim1; x++) 
			{
				for (int y = 0; y < C_dim0; y++) 
				{
					FPTYPE val = 0;
					for (int xp = 0; xp < B_dim1; xp++) 
					{
						FPTYPE *A_off = A_src + (x+xp)*A_dim0 + y;
						FPTYPE *B_off = B_src + xp*B_dim0;
						for (int yp = 0; yp < B_dim0; yp++) 
						{
							val += *(A_off++) * *(B_off++);
						}
					}
					Ci_allf_ptr[f*C_dim0*C_dim1 + x*C_dim0 + y] = val;
				}
			}
		}

		// sum features contributions
		for (int x = 0; x < C_dim1; x++) 
		{
			for (int y = 0; y < C_dim0; y++) 
			{
				FPTYPE val = 0;
				for (int f = 0; f < num_features; f++) 
					val	+= Ci_allf_ptr[f*C_dim0*C_dim1 + x*C_dim0 + y];
				Ci[x*C_dim0 + y] = val;
			}
		}
	}

	return C;
}

#endif






