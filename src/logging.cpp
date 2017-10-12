#include "logging.h"


char buffer[255];
const char *logFileName;

void initLog(const char *_logFileName)
{
#if WITH_MATLAB
	mxArray *in[1];
	in[0] = mxCreateString(_logFileName);
	mexCallMATLAB(0, NULL, 1, in, "initLog");
#else
	logFileName = _logFileName;

	// reset log file
	FILE *logFile = fopen(logFileName, "w");
	if( ! logFile )
		throw std::runtime_error(std::string(" in initLog(), failed to open file '") + logFileName + "'");
	fclose(logFile);
#endif
}

/*
 * Write text to log file.
 */
void writeLog(const char *text)
{
#if WITH_MATLAB
	mxArray *in[1];
	in[0] = mxCreateString(text);
	mexCallMATLAB(0, NULL, 1, in, "writeLog");
#else
	FILE *logFile = fopen(logFileName, "a");
	if( ! logFile )
		throw std::runtime_error(std::string(" in writeLog(), failed to open file '") + logFileName + "'");
	fprintf(logFile, "%s\n", text);
	fclose(logFile);
#endif
}

/*
 * Write FPTYPE value to log file.
 */
void writeLog(FPTYPE value)
{
	snprintf(buffer, sizeof(buffer), "%.5g", value);
	writeLog(buffer);
}

/*
 * Write mxArray data to log file.
 */
#if WITH_MATLAB
void writeLog(mxArray* data)
{
	mxArray *in[1];
	in[0] = data;
	mexCallMATLAB(0, NULL, 1, in, "writeLog");
}
#endif

/*
 * Write array to log file with given format.
 */
template<typename T> void writeLog(const char *format, const myArray<T>* myarray)
{
	std::string text;

	for( int r = 0; r < myarray->getRows(); r++)
	{
		for( int p = 0; p < myarray->getPlanes(); p++)
		{
			for( int c = 0; c < myarray->getCols(); c++)
			{
				snprintf(buffer, sizeof(buffer), format, myarray->get(r, c, p));
				text += buffer;
				text += ",";
			}
		}

		// remove the last ','
		text.resize(text.size()-1);

		writeLog(text.c_str());
		text.clear();
	}
}

/*
 * Write myArray data to log file.
 */
void writeLog(const myArray<unsigned char>* myarray)
{
	writeLog("%d", myarray);
}

/*
 * Write myArray data to log file.
 */
void writeLog(const myArray<int>* myarray)
{
	writeLog("%d", myarray);
}

/*
 * Write myArray data to log file.
 */
void writeLog(const myArray<FPTYPE>* myarray)
{
	writeLog("%.5g", myarray);
}

/*
// Write a matrix to a csv file.
// Slow but easy implementation that calls 
// Matlab csvwrite(filename,M) function
void mcsvwrite( const char *filename, mxArray *matrix)
{
	mxArray *in[2], *out[1];
	in[0] = mxCreateString(filename);
	in[1] = matrix;
	mexCallMATLAB(0, out, 2, in, "csvwrite");
}
*/

