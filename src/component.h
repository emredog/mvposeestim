#ifndef COMPONENT_H
#define COMPONENT_H

#include "flags.h"
#include "myarray.hpp"

struct Model; // forward declaration
struct Feature_pyramid; // forward declaration
struct Component; // forward declaration

struct Component
{
	myArray<int> *biasid; // 6x6x26 except first one 1x1x(1)
	myArray<int> *filterid; // 26x6
	myArray<int> *defid; // 26x6 except first one []
	myArray<int> *parent; // 26x1

	myArray<FPTYPE> *b; // 6x6x26 except first one 1x1x(1)
	myArray<FPTYPE> *biasI; // 6x6x26
	myArray<FPTYPE> *sizex; // 6x1x26
	myArray<FPTYPE> *sizey; // 6x1x26
	myArray<FPTYPE> *w; // 4x6x26
	myArray<FPTYPE> *defI; // 6x1x26
	myArray<FPTYPE> *scale; // 26x1
	myArray<FPTYPE> *startx; // 6x1x26
	myArray<FPTYPE> *starty; // 6x1x26
	myArray<FPTYPE> *step; // 26x1
};


/*
 * Initialize a Component structure and filters
 * from a model and a features pyramid.
 */
void modelcomponents_C(const Model *model, const Feature_pyramid *pyra, /*outputs*/ Component *components, myArray<FPTYPE> ***filters);

/*
 * Delete a Component structure and release memory.
 */
void delete_components(Component *components, int componentsNbr);

#endif // COMPONENT_H
