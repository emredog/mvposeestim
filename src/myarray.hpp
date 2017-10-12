#ifndef MYARRAY_HPP
#define MYARRAY_HPP


#if WITH_MATLAB
#include "mex.h"
#endif

#if WITH_OPENCV
#include <opencv2/opencv.hpp>
#endif

#ifdef USE_CUDA
#include <cuda_runtime_api.h>
#include <string.h>
#endif

#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <fstream>



template<typename T> class myArray
{
public:

#if WITH_MATLAB
    /*
     * Constructor from Matlab mxArray.
     */
    myArray(const mxArray* mxarray);

    /*
     * Constructor from Matlab 'struct array'.
     */
    myArray(const mxArray* structarray, int index, const char *field);
#endif

#if WITH_OPENCV
    /*
     * Constructor from OpenCV cv::Mat.
     */
    myArray(const cv::Mat *cvmat, bool cuda_pinned_mem = false);
#endif

    /*
     * Constructor from another myArray.
     * Make a deep copy of the input array.
     * This is NOT a copy constructor because of the signature.
     */
    myArray(const myArray<T>* myarray, bool cuda_pinned_mem = false);

    /*
     * Constructor from a stream (a file for example).
     */
    myArray(std::istream& in, bool cuda_pinned_mem = false);

    /*
     * Constructor for simple C++ array of fixed size.
     */
    myArray(int rows, int cols, int planes = 1, bool cuda_pinned_mem = false);

    /*
     * Destructor.
     */
    ~myArray();

    /*
     * Getters.
     */
    int getRows(void) const  { return rows; }
    int getCols(void) const  { return cols; }
    int getPlanes(void) const  { return planes; }
    int getNumberOfElements(void) const  { return eltsNbr; }

    /*
     * Get pointer to data at given position.
     */
    T *getPtr(int row = 0, int col = 0, int plane = 0) const;

    /*
     * Get value of data at given position.
     */
    T get(int row = 0, int col = 0, int plane = 0) const;

    /*
     * Set value of data at given position.
     */
    void set(T value, int row = 0, int col = 0, int plane = 0);

protected:
#if WITH_MATLAB
    /*
     * Initialize from mxArray. Do NOT allocate memory.
     */
    void init(const mxArray *mxarray);
#endif

    /*
     * Initialize from size. Do allocate memory.
     */
    void init(int rows, int cols, int planes, bool cuda_pinned_mem);

    T *data; // in line data (column major, row major, plane)
    int rows;
    int cols;
    int planes;
    int eltsNbr; // number of elements = rows * cols * planes
    bool isMxArray;
    bool pinnedMemory;
};


#if WITH_MATLAB
template<typename T> myArray<T>::myArray(const mxArray* mxarray)
{
    init(mxarray);
}
#endif

#if WITH_MATLAB
template<typename T> myArray<T>::myArray(const mxArray* structarray, int index, const char *field)
{
    mxArray *fieldArray = mxGetField(structarray, index, field);

    if( ! fieldArray )
    {
        std::cout << "index: " << index << "\n";
        std::cout << "field: " << field << "\n";
        throw std::runtime_error("In myArray::myArray(): invalid array (in the constructor)");
    }

    init(fieldArray);
}
#endif

template<typename T> myArray<T>::myArray(int rows, int cols, int planes, bool cuda_pinned_mem)
{
    init(rows, cols, planes, cuda_pinned_mem);
}


template<typename T> myArray<T>::myArray(const myArray<T>* src, bool cuda_pinned_mem)
{
    // create storage
    init(src->getRows(), src->getCols(), src->getPlanes(), cuda_pinned_mem);

    // copy data
    memcpy( data, src->getPtr(), eltsNbr * sizeof(T));
}

#if WITH_OPENCV
template<typename T> myArray<T>::myArray(const cv::Mat* cvmat, bool cuda_pinned_mem)
{
    if( cvmat->depth() != CV_8U )
        throw std::runtime_error("In myArray::myArray(cv::Mat*): invalid cv::Mat depth");

    // create storage
    int channels = cvmat->channels();

    // workaround for png images with alpha files -- ED - 20160915
    if (channels == 4)
        channels--; // just drop the alpha channel -- ED - 20160915

    init(cvmat->rows, cvmat->cols, channels, cuda_pinned_mem);

    // copy data
    unsigned char *ptr = cvmat->data;
    for(int r = 0; r < cvmat->rows; r++)
    {
        for(int c = 0; c < cvmat->cols; c++)
        {
            for(int ch = 0; ch < channels; ch++)
            {
                // cv::Mat is BGR
                // Matlab image is RGB, hence the inversion of channels order
                set(*ptr, r, c, channels-1-ch);
                ptr++;
            }
        }
    }
}
#endif

template<typename T> myArray<T>::myArray(std::istream& in, bool cuda_pinned_mem)
{
    // get size
    int _rows, _cols, _planes;
    in >> _rows >> _cols >> _planes;

    // create storage
    init(_rows, _cols, _planes, cuda_pinned_mem);

    // read data
    for(int p = 0; p < _planes; p++)
    {
        for(int c = 0; c < _cols; c++)
        {
            for(int r = 0; r < _rows; r++)
            {
                T value;
                in >> value;
                set(value, r, c, p);
            }
        }
    }
}


template<typename T> myArray<T>::~myArray()
{
    #ifdef USE_CUDA
    if( ! isMxArray )
    {
        if( pinnedMemory )
            cudaFreeHost(data);
        else
            free(data);
    }
    #else
    if( ! isMxArray )
        free(data);
    #endif
}


#if WITH_MATLAB
template<typename T> void myArray<T>::init(const mxArray *mxarray)
{
    int dimsNbr = mxGetNumberOfDimensions(mxarray);
    if( dimsNbr != 2  &&  dimsNbr != 3 )
        throw std::runtime_error("In myArray::init(): invalid array dimension");

    const int *dims = mxGetDimensions(mxarray);
    rows = dims[0];
    cols = dims[1];
    if( dimsNbr == 3 )
        planes = dims[2];
    else
        planes = 1;
    eltsNbr = mxGetNumberOfElements(mxarray);

    data = (T*) mxGetData(mxarray);
    if( ! data )
        throw std::runtime_error("In myArray::init(): invalid array (in the ::init(..) function )");

    isMxArray = true;
}
#endif


template<typename T> void myArray<T>::init(int _rows, int _cols, int _planes, bool cuda_pinned_mem)
{
    rows = _rows;
    cols = _cols;
    planes = _planes;
    eltsNbr = rows*cols*planes;
    pinnedMemory = cuda_pinned_mem;

#ifdef USE_CUDA
    if( pinnedMemory )
    {
        cudaError_t status = cudaHostAlloc((void **) &data, eltsNbr*sizeof(T), 0);
        if( status == cudaSuccess )
            memset(data, 0, eltsNbr*sizeof(T));
        else
            printf("cudaHostAlloc failed with error '%s'.\n", cudaGetErrorString(status));
    }
    else
        data = (T*) calloc(eltsNbr, sizeof(T));
#else
    data = (T*) calloc(eltsNbr, sizeof(T));
#endif

    if( ! data )
        throw std::runtime_error("In myArray::myArray(): failed to allocate memory");

    isMxArray = false;
}


template<typename T> T* myArray<T>::getPtr(int row, int col, int plane) const
{
#if DEBUG
    if( row >= rows  ||  col >= cols  ||  plane >= planes )
    {
        char buffer[255];
        snprintf(buffer, sizeof(buffer),
            "(rows=%d, cols=%d, planes=%d, row=%d, col=%d, plane=%d)",
            rows, cols, planes, row, col, plane);
        throw std::runtime_error(
            std::string("In myArray::getPtr(): index out of range ")
            + std::string(buffer) );
    }
#endif

    // column major, row major storage
    return data + plane*rows*cols + col*rows + row;
}


template<typename T> T myArray<T>::get(int row, int col, int plane) const
{
    T *ptr = getPtr(row, col, plane);
    return *ptr;
}


template<typename T> void myArray<T>::set(T value, int row, int col, int plane)
{
    T *ptr = getPtr(row, col, plane);
    *ptr = value;
}


/*
 * Stream output.
 */
template<typename T> std::ostream& operator<<(std::ostream& out, const myArray<T>& a)
{
    out << a.getRows() << " " << a.getCols() << " " << a.getPlanes() << " ";
    for(int p = 0; p < a.getPlanes(); p++)
        for(int c = 0; c < a.getCols(); c++)
            for(int r = 0; r < a.getRows(); r++)
                out << a.get(r, c, p) << " ";

    return out;
}

#endif // MYARRAY_HPP
