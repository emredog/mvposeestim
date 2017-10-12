#ifndef FEATURE_PYRAMID_H
#define FEATURE_PYRAMID_H

#include "flags.h"
#include "myarray.hpp"
#include "model.h"

struct Feature_pyramid
{
	myArray<FPTYPE>** feat; // array of myArray<>'s
	myArray<FPTYPE>* scale; // one myArray
	int max_scale; // = Matlab 'length(pyra.feat)'
	int interval;
	int imy;
	int imx;
	int pady;
	int padx;
};

// compute feature pyramid.
Feature_pyramid* featpyramid_C(const myArray<unsigned char> *im, const Model *model);

// delete feature pyramid
void delete_featpyramid(Feature_pyramid **pyra);

#endif // FEATURE_PYRAMID_H
