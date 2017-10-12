#ifndef PARTTYPECOMPAT_H
#define PARTTYPECOMPAT_H

#include "myarray.hpp"


class PartTypeCompat
{
public:

#if defined(HUMANEVA)
    PartTypeCompat(const char *csvFileName, int nbOfPartTypes = 6, int nbOfParts = 26);
#elif defined(UMPM)
    PartTypeCompat(const char *csvFileName, int nbOfPartTypes = 6, int nbOfParts = 26);
#else
    PartTypeCompat(const char *csvFileName, int nbOfPartTypes = 6, int nbOfParts);
#endif
    ~PartTypeCompat();
    const myArray<FPTYPE>* const getAllScores() const {return this->partTypeScores;} //const pointer to const myArray: i wanted it to be read only


protected:
    myArray<FPTYPE> *partTypeScores;
	int nbOfPartTypes;
	int nbOfParts;

	void init(const char *csvFileName);
};

#endif // PARTTYPECOMPAT_H
