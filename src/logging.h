#ifndef LOGGING_H
#define LOGGING_H

// logging functions

#include "flags.h"
#include "myarray.hpp"

extern char buffer[255];


/*
 * Initialize logging.
 */
void initLog(const char *_logFileName);

/*
 * Write text to log file.
 */
void writeLog(const char *text);

/*
 * Write FPTYPE value to log file.
 */
void writeLog(FPTYPE value);

#if WITH_MATLAB
/*
 * Write Matlab array data to log file.
 */
void writeLog(mxArray* data);
#endif

/*
 * Write myArray data to log file.
 */
void writeLog(const myArray<unsigned char>* myarray);

/*
 * Write myArray data to log file.
 */
void writeLog(const myArray<int>* myarray);

/*
 * Write myArray data to log file.
 */
void writeLog(const myArray<FPTYPE>* myarray);

/*
 * Write std::vector<T> to log file.
 * The std::vector is supposed to be in row-col-plane order
 * as opposed to myArrays which are in col-row-plane order
 */
template<typename T> void writeLog(const std::vector<T>* v, int rows, int cols = 1, int planes = 1)
{
	// convert std::vector to myArray
	myArray<T> array(rows, cols, planes);
	for(int r = 0; r < rows; r++)
		for(int c = 0; c < cols; c++)
			for(int p = 0; p < planes; p++)
				array.set((*v)[p*rows*cols + r*cols + c], r, c, p);

	// log to file
	writeLog(&array);
}

#endif // LOGGING_H

