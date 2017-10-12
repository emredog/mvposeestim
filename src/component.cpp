#include "component.h"

#include "model.h"
#include "feature_pyramid.h"
#include "component.h"

#include <math.h>


void modelcomponents_C(const Model *model, const Feature_pyramid *pyra, /*outputs*/ Component *components, myArray<FPTYPE> ***filters)
{
	for(int c = 1; c <= model->componentsNbr; c++)
	{
		Component *p = &(components[c-1]);
		p->biasid = model->components_biasid[c-1];
		p->filterid = model->components_filterid[c-1];
		p->defid = model->components_defid[c-1];
		p->parent = model->components_parent;

		p->b = new myArray<FPTYPE>(model->typesNbr, model->typesNbr, model->partsNbr);
		p->biasI = new myArray<FPTYPE>(model->typesNbr, model->typesNbr, model->partsNbr);
		p->sizex = new myArray<FPTYPE>(model->typesNbr, 1, model->partsNbr);
		p->sizey = new myArray<FPTYPE>(model->typesNbr, 1, model->partsNbr);

		int defs_w_sz = model->defs_w->getCols();
		p->w = new myArray<FPTYPE>(defs_w_sz, model->typesNbr, model->partsNbr);
		p->defI = new myArray<FPTYPE>(model->typesNbr, 1, model->partsNbr);
		p->scale = new myArray<FPTYPE>(model->partsNbr, 1);
		p->startx = new myArray<FPTYPE>(model->typesNbr, 1, model->partsNbr);
		p->starty = new myArray<FPTYPE>(model->typesNbr, 1, model->partsNbr);
		p->step = new myArray<FPTYPE>(model->partsNbr, 1);

		for(int k = 1; k <= model->componentSz; k++)
		{
			// store the scale of each part relative to the component root

			int par = p->parent->get(k-1);
			if( par >= k )
				throw std::runtime_error("modelcomponents_C(): invalid parent value");

			int colmax = p->b->getCols();
			int rowmax = p->b->getRows();
			if( k == 1 )
			{
				// root special case, biasid is 1x1
				colmax = 1;
				rowmax = 1;
			}
			for(int col = 0; col < colmax; col++)
			{
				for(int row = 0; row < rowmax; row++)
				{
					int biasid = p->biasid->get(row, col, k-1);
					p->b->set(model->bias_w->get(biasid-1), row, col, k-1);
					p->biasI->set(model->bias_i->get(biasid-1), row, col, k-1);
				}
			}

			for(int f = 1; f <= model->typesNbr; f++)
			{
				int filterId = p->filterid->get(k-1, f-1);
				myArray<FPTYPE> *x_w = model->filters_w[filterId-1];
				p->sizey->set(x_w->getRows(), f-1, 0, k-1);
				p->sizex->set(x_w->getCols(), f-1, 0, k-1);
			}

			if( k >= 2 )  // defid is not defined for the root
			{
				for(int f = 1; f <= model->typesNbr; f++)
				{
					int defId = p->defid->get(k-1, f-1);
					for(int i = 1; i <= p->w->getRows(); i++) 
						p->w->set(model->defs_w->get(defId-1, i-1), i-1, f-1, k-1);
					p->defI->set(model->defs_i->get(defId-1), f-1, 0, k-1);
					FPTYPE ax = model->defs_anchor->get(defId-1, 1-1);
					FPTYPE ay = model->defs_anchor->get(defId-1, 2-1);
					FPTYPE ds = model->defs_anchor->get(defId-1, 3-1);
					FPTYPE parent_scale = p->scale->get(par-1);
					p->scale->set(ds + parent_scale, k-1);
					// amount of (virtual) padding to hallucinate
					FPTYPE step = pow(2.0, ds);
					FPTYPE virtpady = (step-1) * pyra->pady;
					FPTYPE virtpadx = (step-1) * pyra->padx;
					// starting points (simulates additional padding at finer scales)
					p->starty->set(ay - virtpady, f-1, 0, k-1);
					p->startx->set(ax - virtpadx, f-1, 0, k-1);
					p->step->set(step, k-1);
				}
			}
		}
	}

	*filters = model->filters_w; // 156x[5x5x32]
}

void delete_components(Component *components, int componentsNbr)
{
	for(int c = 1; c <= componentsNbr; c++)
	{
		Component *p = &(components[c-1]);
		delete p->b;
		delete p->biasI;
		delete p->sizex;
		delete p->sizey;
		delete p->w;
		delete p->defI;
		delete p->scale;
		delete p->startx;
		delete p->starty;
		delete p->step;
	}
}

