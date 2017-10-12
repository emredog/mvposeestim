#ifndef MODEL_H
#define MODEL_H

#include "flags.h"
#include "myarray.hpp"

/*
 * Pose estimation C++ model.
 */
struct Model
{
	int partsNbr; // number of parts =26
	int typesNbr; // number of part types =6

	// bias
	int	biasSz; // =901
	myArray<FPTYPE> *bias_w; // [901] 
	myArray<FPTYPE> *bias_i; // [901]

	// filters
	int filtersSz; //=156
	myArray<FPTYPE> **filters_w; // [156x[5 5 32]]
	myArray<FPTYPE> *filters_i; // [156]

	// defs
	int defsSz; //=150
	myArray<FPTYPE> *defs_w; // [150x[1x4]]
	myArray<FPTYPE> *defs_i; // [150]
	myArray<FPTYPE> *defs_anchor; // [150x[1x3]]

	// components
	int componentsNbr; // number of components =1
	int componentSz; // size of one component =1x26
	myArray<int> **components_biasid; // [1x26x[6x6]] except first one [1x1]
	myArray<int> **components_filterid; // [1x26x[1x6]]
	myArray<int> **components_defid; // [1x26x[1x6]] except first one []
	myArray<int> *components_parent; // [1x26]

	// pa
	myArray<FPTYPE> *pa; // [26]

	// maxsize
	myArray<FPTYPE> *maxsize; // 1 line, 2 cols

	// interval
	FPTYPE interval;

	// sbin
	FPTYPE sbin;

	// len
	FPTYPE len;

	// thresh
	FPTYPE thresh;

	// obj
	FPTYPE obj;
};


/*
 * Initialize C++ model from Matlab model.
 */
#if WITH_MATLAB
void init_model(const mxArray *matlabModel, /*output*/ Model *model);
#endif

/*
 * Delete C++ model. Set the pointer to NULL.
 */
void delete_model(Model **model);

/*
 * Save C++ model to file.
 */
void save_model(const Model *model, const char *fileName);

/*
 * Load C++ model from file.
 */
Model* load_model(const char *fileName);

#endif // MODEL_H
