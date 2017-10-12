#ifndef PASSMSG_H
#define PASSMSG_H


#include "flags.h"
#include "myarray.hpp"
struct Component; // forward declaration

enum PromisingPartRowIndices
{
    COL = 0,
    ROW = 1,
    FI  = 2,
    SCORE = 3
};


// Second version of passmsg, WITHOUT the distance transform
// as input arg, it doesn't take all scores in the matrix, but only the
// the ones that we picked as "promising"
void passmsg_noDT(int childId, int parentId, const Component *parts, myArray<FPTYPE> *promisingPositions[], /*outputs*/ myArray<FPTYPE> **score, myArray<int> **_IchilPos);

// C version of passmsg() function defined in detect_fast.m
// Given a 2D array of filter scores 'child',
// (1) Apply distance transform
// (2) Shift by anchor position of part wrt parent
// (3) Downsample if necessary
void passmsg_C(int childId, int parentId, const Component *parts, myArray<FPTYPE> **parts_score[], /*outputs*/ myArray<FPTYPE> **score, myArray<int> **_Ix, myArray<int> **_Iy, myArray<int> **_Ik);

#endif // PASSMSG_H
