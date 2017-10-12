#include "model.h"

#include <iostream>
#include <fstream>
#include <cfloat>


/*
 * Initialize C++ model from Matlab model.
 */
#if WITH_MATLAB
void init_model(const mxArray *matlabModel, /*output*/ Model *model)
{
	// bias
	const mxArray *mxbias = mxGetField(matlabModel, 0, "bias");
	model->biasSz = mxGetNumberOfElements(mxbias);
	model->bias_w = new myArray<FPTYPE>(model->biasSz, 1); 
	model->bias_i = new myArray<FPTYPE>(model->biasSz, 1); 
	for(int j = 0; j < model->biasSz; j++)
	{
		model->bias_w->set(myArray<FPTYPE>(mxbias, j, "w").get(), j);
		model->bias_i->set(myArray<FPTYPE>(mxbias, j, "i").get(), j);
	}

	// filters
	const mxArray *mxfilters = mxGetField(matlabModel, 0, "filters");
	model->filtersSz = mxGetNumberOfElements(mxfilters);
	model->filters_w = (myArray<FPTYPE>**) calloc(model->filtersSz, sizeof(myArray<FPTYPE>*));
	model->filters_i = new myArray<FPTYPE>(model->filtersSz, 1);
	for(int j = 0; j < model->filtersSz; j++)
	{
		myArray<FPTYPE> mxw(mxfilters, j, "w");
		model->filters_w[j] = new myArray<FPTYPE>(mxw);

		model->filters_i->set(myArray<FPTYPE>(mxfilters, j, "i").get(), j);
	}

	// defs
	const mxArray *mxdefs = mxGetField(matlabModel, 0, "defs");
	model->defsSz = mxGetNumberOfElements(mxdefs);

	model->defs_w = new myArray<FPTYPE>(model->defsSz, myArray<FPTYPE>(mxdefs, 1, "w").getCols());
	model->defs_i = new myArray<FPTYPE>(model->defsSz, 1); 
	model->defs_anchor = new myArray<FPTYPE>(model->defsSz, myArray<FPTYPE>(mxdefs, 1, "anchor").getCols());

	for(int j = 0; j < model->defsSz; j++)
	{
		model->defs_i->set(myArray<FPTYPE>(mxdefs, j, "i").get(), j);

		for(int k = 0; k < model->defs_w->getCols(); k++)
			model->defs_w->set(myArray<FPTYPE>(mxdefs, j, "w").get(0, k), j, k);

		for(int k = 0; k < model->defs_anchor->getCols(); k++)
			model->defs_anchor->set(myArray<FPTYPE>(mxdefs, j, "anchor").get(0, k), j, k);
	}

	// pa
	myArray<FPTYPE> mxpa(matlabModel, 0, "pa"); 
	model->pa = new myArray<FPTYPE>(mxpa);
	model->partsNbr = model->pa->getCols();

	// components
	const mxArray *mxcomponents = mxGetField(matlabModel, 0, "components");
	model->componentsNbr = mxGetNumberOfElements(mxcomponents);
	const mxArray *mxcomponent0 = mxGetCell(mxcomponents, 0);
	model->componentSz = mxGetNumberOfElements(mxcomponent0);
	if( model->componentSz != model->partsNbr )
		throw std::runtime_error("init_model(): component size does not match parts number");

	model->components_biasid = (myArray<int>**) calloc(model->componentsNbr, sizeof(myArray<int>*));
	model->components_filterid = (myArray<int>**) calloc(model->componentsNbr, sizeof(myArray<int>*));
	model->components_defid = (myArray<int>**) calloc(model->componentsNbr, sizeof(myArray<int>*));
	model->components_parent = new myArray<int>(model->componentSz, model->componentsNbr);

	model->typesNbr = mxGetNumberOfElements(mxGetField(mxcomponent0, 0, "filterid"));

	for(int c = 0; c < model->componentsNbr; c++)
	{
		const mxArray *mxcomponent = mxGetCell(mxcomponents, c);

		model->components_biasid[c] = new myArray<int>(model->typesNbr, model->typesNbr, model->componentSz);
		model->components_filterid[c] = new myArray<int>(model->componentSz, model->typesNbr);
		model->components_defid[c] = new myArray<int>(model->componentSz, model->typesNbr);

		for(int p = 0; p < model->partsNbr; p++)
		{
			myArray<FPTYPE> biasid(mxcomponent, p, "biasid");
			if( p == 0 )
			{
				// the first array is [1x1] only !
				model->components_biasid[c]->set(biasid.get(), 0, 0, p);
			}
			else
			{
				for(int i = 0; i < model->typesNbr; i++)
					for(int j = 0; j < model->typesNbr; j++)
						model->components_biasid[c]->set(biasid.get(i, j), i, j, p);
			}

			myArray<FPTYPE> filterid(mxcomponent, p, "filterid");
			for(int i = 0; i < model->typesNbr; i++)
				model->components_filterid[c]->set(filterid.get(0, i), p, i);

			// the first defid array is [] !
			if( p != 0 )
			{
				myArray<FPTYPE> defid(mxcomponent, p, "defid");
				for(int i = 0; i < model->typesNbr; i++)
					model->components_defid[c]->set(defid.get(0, i), p, i);
			}

			myArray<FPTYPE> parent(mxcomponent, p, "parent"); // single value
			model->components_parent->set(parent.get(), p, c);
		}
	}

	// maxsize
	myArray<FPTYPE> mxmaxsize(matlabModel, 0, "maxsize"); 
	model->maxsize = new myArray<FPTYPE>(mxmaxsize);
	
	// interval
	myArray<FPTYPE> mxinterval(matlabModel, 0, "interval"); // single value
	model->interval = mxinterval.get();

	// sbin
	myArray<FPTYPE> mxsbin(matlabModel, 0, "sbin"); // single value
	model->sbin = mxsbin.get();

	// len
	myArray<FPTYPE> mxlen(matlabModel, 0, "len"); // single value
	model->len = mxlen.get();

	// thresh
	myArray<FPTYPE> mxthresh(matlabModel, 0, "thresh"); // single value
	model->thresh = mxthresh.get();

	// obj
    // ED: If you manually train a model, there is no "obj". Also, AFAIK this field is never used in detect_fast_C
    // so, first check if this field exists:
    int field_num = mxGetFieldNumber(matlabModel, "obj"); //check if this field exists
    if (field_num >= 0) // if there's no field, it returns -1
    {
        myArray<FPTYPE> mxobj(matlabModel, 0, "obj"); // single value
        model->obj = mxobj.get();
    }
    else
    {
        model->obj = -DBL_MAX;
    }
}
#endif


/*
 * Delete C++ model. Set the pointer to NULL.
 */
void delete_model(Model **model)
{
	if( ! *model )
		return;

	// bias
	delete (*model)->bias_w;
	delete (*model)->bias_i;

	// filters
	for(int j = 0; j < (*model)->filtersSz; j++)
		delete (*model)->filters_w[j];
	free((*model)->filters_w);
	delete (*model)->filters_i;

	// defs
	delete (*model)->defs_w;
	delete (*model)->defs_i;
	delete (*model)->defs_anchor;

	// pa
	delete (*model)->pa;

	// components
	for(int c = 0; c < (*model)->componentsNbr; c++)
	{
		delete (*model)->components_biasid[c];
		delete (*model)->components_filterid[c];
		delete (*model)->components_defid[c];
	}
	free((*model)->components_biasid);
	free((*model)->components_filterid);
	free((*model)->components_defid);
	delete (*model)->components_parent;

	// maxsize
	delete (*model)->maxsize;

	// model
	delete *model;
	*model = NULL;
}


/*
 * Save C++ model to file.
 */
void save_model(const Model *model, const char *fileName)
{
	std::ofstream file(fileName);
	if( ! file.is_open() )
		throw std::runtime_error(std::string(" in save_model(), failed to open file '") + fileName + "'");
	file.precision(15);

	file << "partsNbr " << model->partsNbr << "\n";
	file << "typesNbr " << model->typesNbr << "\n";

	file << "biasSz " << model->biasSz << "\n";
	file << "bias_w " << *(model->bias_w) << "\n";
	file << "bias_i " << *(model->bias_i) << "\n";

	file << "filtersSz " << model->filtersSz << "\n";
	file << "filters_w ";
	for(int i = 0; i < model->filtersSz; i++)
		file << *(model->filters_w[i]);
	file << "\n";
	file << "filters_i " << *(model->filters_i) << "\n";

	file << "defsSz " << model->defsSz << "\n";
	file << "defs_w " << *(model->defs_w) << "\n";
	file << "defs_i " << *(model->defs_i) << "\n";
	file << "defs_anchor " << *(model->defs_anchor) << "\n";

	file << "componentsNbr " << model->componentsNbr << "\n";
	file << "componentSz " << model->componentSz << "\n";
	file << "components_biasid ";
	for(int i = 0; i < model->componentsNbr; i++)
		file << *(model->components_biasid[i]);
	file << "\n";
	file << "components_filterid ";
	for(int i = 0; i < model->componentsNbr; i++)
		file << *(model->components_filterid[i]);
	file << "\n";
	file << "components_defid ";
	for(int i = 0; i < model->componentsNbr; i++)
		file << *(model->components_defid[i]);
	file << "\n";
	file << "components_parent " << *(model->components_parent) << "\n";

	file << "pa " << *(model->pa) << "\n";
	file << "maxsize " << *(model->maxsize) << "\n";
	file << "interval " << model->interval << "\n";
	file << "sbin " << model->sbin << "\n";
	file << "len " << model->len << "\n";
	file << "thresh " << model->thresh << "\n";
	file << "obj " << model->obj << "\n";

	file.close();
}


/*
 * Load C++ model from file.
 */
Model* load_model(const char *fileName)
{
	std::ifstream file(fileName);
	if( ! file.is_open() )
		throw std::runtime_error(std::string(" in load_model(), failed to open file '") + fileName + "'");

	Model *model = new Model;

	std::string title;
	file >> title >> model->partsNbr;
	file >> title >> model->typesNbr;
	
	file >> title >> model->biasSz;
	file >> title; // "bias_w"
	model->bias_w = new myArray<FPTYPE>(file);
	file >> title; // "bias_i"
	model->bias_i = new myArray<FPTYPE>(file);

	file >> title >> model->filtersSz;
	model->filters_w = (myArray<FPTYPE>**) calloc(model->filtersSz, sizeof(myArray<FPTYPE>*));
	file >> title; // "filters_w "
	for(int i = 0; i < model->filtersSz; i++)
		model->filters_w[i] = new myArray<FPTYPE>(file, true);
	file >> title; // "filters_i "
	model->filters_i = new myArray<FPTYPE>(file);
	
	file >> title >> model->defsSz;
	file >> title; // "defs_w "
	model->defs_w = new myArray<FPTYPE>(file);
	file >> title; // "defs_i "
	model->defs_i = new myArray<FPTYPE>(file);
	file >> title; // "defs_anchor "
	model->defs_anchor = new myArray<FPTYPE>(file);

	file >> title >> model->componentsNbr;
	file >> title >> model->componentSz;
	model->components_biasid = (myArray<int>**) calloc(model->componentsNbr, sizeof(myArray<int>*));
	file >> title; // "components_biasid "
	for(int i = 0; i < model->componentsNbr; i++)
		model->components_biasid[i] = new myArray<int>(file);
	model->components_filterid = (myArray<int>**) calloc(model->componentsNbr, sizeof(myArray<int>*));
	file >> title; // "components_filterid "
	for(int i = 0; i < model->componentsNbr; i++)
		model->components_filterid[i] = new myArray<int>(file);
	model->components_defid = (myArray<int>**) calloc(model->componentsNbr, sizeof(myArray<int>*));
	file >> title; // "components_defid "
	for(int i = 0; i < model->componentsNbr; i++)
		model->components_defid[i] = new myArray<int>(file);
	file >> title; // "components_parent "
	model->components_parent = new myArray<int>(file);

	file >> title; // "pa "
	model->pa = new myArray<FPTYPE>(file);
	file >> title; // "maxsize "
	model->maxsize = new myArray<FPTYPE>(file);
	file >> title >> model->interval;
	file >> title >> model->sbin;
	file >> title >> model->len;
	file >> title >> model->thresh;
	file >> title >> model->obj;

	return model;
}
