#include "feature_pyramid.h"

#include "logging.h"

#include <string.h>
#include <cmath>
#include <assert.h>


//--------------------------------------------------------------------

// source: mex_unix/reduce.cc

// reduce(im) resizes im to half its size, using a 5-tap binomial filter for anti-aliasing
// (see Burt & Adelson's Laplacian Pyramid paper)

// reduce each column
// result is transposed, so we can apply it twice for a complete reduction
void reduce1dtran(FPTYPE *src, int sheight, FPTYPE *dst, int dheight, 
		  int width, int chan) {
  // resize each column of each color channel
  bzero(dst, chan*width*dheight*sizeof(FPTYPE));
  int y;
  FPTYPE *s, *d;

  for (int c = 0; c < chan; c++) {
    for (int x = 0; x < width; x++) {
      s  = src + c*width*sheight + x*sheight;
      d  = dst + c*dheight*width + x;

      // First row
      *d = s[0]*.6875 + s[1]*.2500 + s[2]*.0625;      

      for (y = 1; y < dheight-2; y++) {	
	s += 2;
	d += width;
	*d = s[-2]*0.0625 + s[-1]*.25 + s[0]*.375 + s[1]*.25 + s[2]*.0625;
      }

      // Last two rows
      s += 2;
      d += width;
      if (dheight*2 <= sheight) {
	*d = s[-2]*0.0625 + s[-1]*.25 + s[0]*.375 + s[1]*.25 + s[2]*.0625;
      } else {
	*d = s[1]*.3125 + s[0]*.3750 + s[-1]*.2500 + s[-2]*.0625;
      }
      s += 2;
      d += width;
      *d = s[0]*.6875 + s[-1]*.2500 + s[-2]*.0625;
    }
  }
}

// takes a FPTYPE color image and a scaling factor
// returns resized image
myArray<FPTYPE>* reduce(myArray<FPTYPE>* im)
{
	FPTYPE *src = im->getPtr();
	int sdims[3];
	sdims[0] = im->getRows();
	sdims[1] = im->getCols();
	sdims[2] = im->getPlanes();

	int ddims[3];
	ddims[0] = (int)round(sdims[0]*.5);
	ddims[1] = (int)round(sdims[1]*.5);
	ddims[2] = sdims[2];
	myArray<FPTYPE> *mydst = new myArray<FPTYPE>(ddims[0], ddims[1], ddims[2]);
	FPTYPE *dst = mydst->getPtr();

	FPTYPE *tmp = (FPTYPE *)calloc(ddims[0]*sdims[1]*sdims[2], sizeof(FPTYPE));
	reduce1dtran(src, sdims[0], tmp, ddims[0], sdims[1], sdims[2]);
	reduce1dtran(tmp, sdims[1], dst, ddims[1], ddims[0], sdims[2]);
	free(tmp);

	return mydst;
}

//--------------------------------------------------------------------

// source: mex_unix/features.cc
	
// small value, used to avoid division by zero
#define eps 0.0001

// unit vectors used to compute gradient orientation
FPTYPE uu[9] = {1.0000, 
		0.9397, 
		0.7660, 
		0.500, 
		0.1736, 
		-0.1736, 
		-0.5000, 
		-0.7660, 
		-0.9397};
FPTYPE vv[9] = {0.0000, 
		0.3420, 
		0.6428, 
		0.8660, 
		0.9848, 
		0.9848, 
		0.8660, 
		0.6428, 
		0.3420};

static inline FPTYPE min(FPTYPE x, FPTYPE y) { return (x <= y ? x : y); }
static inline FPTYPE max(FPTYPE x, FPTYPE y) { return (x <= y ? y : x); }

static inline int min(int x, int y) { return (x <= y ? x : y); }
static inline int max(int x, int y) { return (x <= y ? y : x); }

// takes a FPTYPE color image and a bin size 
// returns HOG features
myArray<FPTYPE>* features(myArray<FPTYPE> *scaled, int sbin)
{
	FPTYPE *im = scaled->getPtr();
	int dims[3];
	dims[0] = scaled->getRows();
	dims[1] = scaled->getCols();
	dims[2] = scaled->getPlanes();

	// memory for caching orientation histograms & their norms
	int blocks[2];
	blocks[0] = (int)round((FPTYPE)dims[0]/(FPTYPE)sbin);
	blocks[1] = (int)round((FPTYPE)dims[1]/(FPTYPE)sbin);
	FPTYPE *hist = (FPTYPE *)calloc(blocks[0]*blocks[1]*18, sizeof(FPTYPE));
	FPTYPE *norm = (FPTYPE *)calloc(blocks[0]*blocks[1], sizeof(FPTYPE));

	// memory for HOG features
	int out[3];
	out[0] = max(blocks[0]-2, 0);
	out[1] = max(blocks[1]-2, 0);
	out[2] = 27+4+1;
	myArray<FPTYPE> *myfeat = new myArray<FPTYPE>(out[0], out[1], out[2]);
	FPTYPE *feat = myfeat->getPtr();

	int visible[2];
	visible[0] = blocks[0]*sbin;
	visible[1] = blocks[1]*sbin;

	for (int x = 1; x < visible[1]-1; x++) {
		for (int y = 1; y < visible[0]-1; y++) {
			// first color channel
			FPTYPE *s = im + min(x, dims[1]-2)*dims[0] + min(y, dims[0]-2);
			FPTYPE dy = *(s+1) - *(s-1);
			FPTYPE dx = *(s+dims[0]) - *(s-dims[0]);
			FPTYPE v = dx*dx + dy*dy;

			// second color channel
			s += dims[0]*dims[1];
			FPTYPE dy2 = *(s+1) - *(s-1);
			FPTYPE dx2 = *(s+dims[0]) - *(s-dims[0]);
			FPTYPE v2 = dx2*dx2 + dy2*dy2;

			// third color channel
			s += dims[0]*dims[1];
			FPTYPE dy3 = *(s+1) - *(s-1);
			FPTYPE dx3 = *(s+dims[0]) - *(s-dims[0]);
			FPTYPE v3 = dx3*dx3 + dy3*dy3;

			// pick channel with strongest gradient
			if (v2 > v) {
				v = v2;
				dx = dx2;
				dy = dy2;
			} 
			if (v3 > v) {
				v = v3;
				dx = dx3;
				dy = dy3;
			}

			// snap to one of 18 orientations
			FPTYPE best_dot = 0;
			int best_o = 0;
			for (int o = 0; o < 9; o++) {
				FPTYPE dot = uu[o]*dx + vv[o]*dy;
				if (dot > best_dot) {
					best_dot = dot;
					best_o = o;
				} else if (-dot > best_dot) {
					best_dot = -dot;
					best_o = o+9;
				}
			}

			// add to 4 histograms around pixel using linear interpolation
			FPTYPE xp = ((FPTYPE)x+0.5)/(FPTYPE)sbin - 0.5;
			FPTYPE yp = ((FPTYPE)y+0.5)/(FPTYPE)sbin - 0.5;
			int ixp = (int)floor(xp);
			int iyp = (int)floor(yp);
			FPTYPE vx0 = xp-ixp;
			FPTYPE vy0 = yp-iyp;
			FPTYPE vx1 = 1.0-vx0;
			FPTYPE vy1 = 1.0-vy0;
			v = sqrt(v);

			if (ixp >= 0 && iyp >= 0) {
				*(hist + ixp*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += 
					vx1*vy1*v;
			}

			if (ixp+1 < blocks[1] && iyp >= 0) {
				*(hist + (ixp+1)*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += 
					vx0*vy1*v;
			}

			if (ixp >= 0 && iyp+1 < blocks[0]) {
				*(hist + ixp*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += 
					vx1*vy0*v;
			}

			if (ixp+1 < blocks[1] && iyp+1 < blocks[0]) {
				*(hist + (ixp+1)*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += 
					vx0*vy0*v;
			}
		}
	}

	// compute energy in each block by summing over orientations
	for (int o = 0; o < 9; o++) {
		FPTYPE *src1 = hist + o*blocks[0]*blocks[1];
		FPTYPE *src2 = hist + (o+9)*blocks[0]*blocks[1];
		FPTYPE *dst = norm;
		FPTYPE *end = norm + blocks[1]*blocks[0];
		while (dst < end) {
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	// compute features
	for (int x = 0; x < out[1]; x++) {
		for (int y = 0; y < out[0]; y++) {
			FPTYPE *dst = feat + x*out[0] + y;      
			FPTYPE *src, *p, n1, n2, n3, n4;

			p = norm + (x+1)*blocks[0] + y+1;
			n1 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + (x+1)*blocks[0] + y;
			n2 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y+1;
			n3 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y;      
			n4 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);

			FPTYPE t1 = 0;
			FPTYPE t2 = 0;
			FPTYPE t3 = 0;
			FPTYPE t4 = 0;

			// contrast-sensitive features
			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 18; o++) {
				FPTYPE h1 = min(*src * n1, 0.2);
				FPTYPE h2 = min(*src * n2, 0.2);
				FPTYPE h3 = min(*src * n3, 0.2);
				FPTYPE h4 = min(*src * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
				dst += out[0]*out[1];
				src += blocks[0]*blocks[1];
			}

			// contrast-insensitive features
			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 9; o++) {
				FPTYPE sum = *src + *(src + 9*blocks[0]*blocks[1]);
				FPTYPE h1 = min(sum * n1, 0.2);
				FPTYPE h2 = min(sum * n2, 0.2);
				FPTYPE h3 = min(sum * n3, 0.2);
				FPTYPE h4 = min(sum * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				dst += out[0]*out[1];
				src += blocks[0]*blocks[1];
			}

			// texture features
			*dst = 0.2357 * t1;
			dst += out[0]*out[1];
			*dst = 0.2357 * t2;
			dst += out[0]*out[1];
			*dst = 0.2357 * t3;
			dst += out[0]*out[1];
			*dst = 0.2357 * t4;

			// truncation feature
			dst += out[0]*out[1];
			*dst = 0;
		}
	}

	free(hist);
	free(norm);
	return myfeat;
}

//--------------------------------------------------------------------

// source: mex_unix/resize.cc

/*
 * Fast image subsampling.
 * This is used to construct the feature pyramid.
 */

// struct used for caching interpolation values
struct alphainfo {
  int si, di;
  FPTYPE alpha;
};

// copy src into dst using pre-computed interpolation values
void alphacopy(FPTYPE *src, FPTYPE *dst, struct alphainfo *ofs, int n) {
  struct alphainfo *end = ofs + n;
  while (ofs != end) {
    dst[ofs->di] += ofs->alpha * src[ofs->si];
    ofs++;
  }
}

// resize along each column
// result is transposed, so we can apply it twice for a complete resize
void resize1dtran(FPTYPE *src, int sheight, FPTYPE *dst, int dheight, 
		  int width, int chan) {
  FPTYPE scale = (FPTYPE)dheight/(FPTYPE)sheight;
  FPTYPE invscale = (FPTYPE)sheight/(FPTYPE)dheight;
  
  // we cache the interpolation values since they can be 
  // shared among different columns
  int len = (int)ceil(dheight*invscale) + 2*dheight;
  alphainfo ofs[len];
  int k = 0;
  for (int dy = 0; dy < dheight; dy++) {
    FPTYPE fsy1 = dy * invscale;
    FPTYPE fsy2 = fsy1 + invscale;
    int sy1 = (int)ceil(fsy1);
    int sy2 = (int)floor(fsy2);       

    if (sy1 - fsy1 > 1e-3) {
      assert(k < len);
      //assert(sy-1 >= 0);
      ofs[k].di = dy*width;
      ofs[k].si = sy1-1;
      ofs[k++].alpha = (sy1 - fsy1) * scale;
    }

    for (int sy = sy1; sy < sy2; sy++) {
      assert(k < len);
      assert(sy < sheight);
      ofs[k].di = dy*width;
      ofs[k].si = sy;
      ofs[k++].alpha = scale;
    }

    if (fsy2 - sy2 > 1e-3) {
      assert(k < len);
      assert(sy2 < sheight);
      ofs[k].di = dy*width;
      ofs[k].si = sy2;
      ofs[k++].alpha = (fsy2 - sy2) * scale;
    }
  }

  // resize each column of each color channel
  memset(dst, 0, chan*width*dheight*sizeof(FPTYPE));
  for (int c = 0; c < chan; c++) {
    for (int x = 0; x < width; x++) {
      FPTYPE *s = src + c*width*sheight + x*sheight;
      FPTYPE *d = dst + c*width*dheight + x;
      alphacopy(s, d, ofs, k);
    }
  }
}

// takes an unsigned 8 bits color image and a scaling factor
// returns FPTYPE resized image
myArray<FPTYPE> *resize(myArray<FPTYPE> *im, FPTYPE scale)
{
	FPTYPE *src = im->getPtr();
	if( scale > 1.0 )
		throw std::runtime_error("resize(): invalid scaling factor");

	int sdims[3];
	sdims[0] = im->getRows();
	sdims[1] = im->getCols();
	sdims[2] = im->getPlanes();

	int ddims[3];
	ddims[0] = (int)round(sdims[0]*scale);
	ddims[1] = (int)round(sdims[1]*scale);
	ddims[2] = sdims[2];

	myArray<FPTYPE> *mydst = new myArray<FPTYPE>(ddims[0], ddims[1], ddims[2]);
	FPTYPE *dst = mydst->getPtr();

	FPTYPE *tmp = (FPTYPE *)calloc(ddims[0]*sdims[1]*sdims[2], sizeof(FPTYPE));
	resize1dtran(src, sdims[0], tmp, ddims[0], sdims[1], sdims[2]);
	resize1dtran(tmp, sdims[1], dst, ddims[1], ddims[0], sdims[2]);
	free(tmp);

	return mydst;
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// Pad features
// all planes are padded with 0, except the last one, which is 
// padded with 1
myArray<FPTYPE>* pad_feat(myArray<FPTYPE> *feat, int padx, int pady)
{
	int rows = feat->getRows();
	int cols = feat->getCols();
	int planes = feat->getPlanes();

	// add 1 to padding because feature generation deletes a 1-cell
	// wide border around the feature map
	int prows = rows+2*(pady+1);
	int pcols = cols+2*(padx+1);
	myArray<FPTYPE> *padded = new myArray<FPTYPE>(prows, pcols, planes);

	// fill the last plane with 1
	FPTYPE *ptr = padded->getPtr(0, 0, planes-1);
	for(int c = 0; c < pcols; c++)
	{
		for(int r = 0; r < prows; r++)
		{
			*ptr = 1;
			ptr++;
		}
	}

	// copy original data to padded array
	for(int p = 0; p < planes; p++)
	{
		for(int c = 0; c < cols; c++)
		{
			for(int r = 0; r < rows; r++)
			{
				padded->set(feat->get(r, c, p), r+pady+1, c+padx+1, p);
			}
		}
	}

	return padded;
}

// compute feature pyramid.
Feature_pyramid* featpyramid_C(const myArray<unsigned char> *im, const Model *model)
{
	int sbin = model->sbin;
	int interval = model->interval;
	int padx = model->maxsize->get(1-1, 2-1);
	padx = std::max(padx-1-1, 0);
	int pady = model->maxsize->get(1-1, 1-1);
	pady = std::max(pady-1-1, 0);
	FPTYPE sc = pow(2.0, (1.0/interval));
	int imsizex = im->getCols();
	int imsizey = im->getRows();
	int max_scale = 1 + floor(log(std::min(imsizex, imsizey)/(5.0*sbin))/log(sc));

#if 0
	// debug
	snprintf(buffer, sizeof(buffer), "featpyramid sbin=%d interval=%d padx=%d pady=%d sc=%f max_scale=%d", sbin, interval, padx, pady, sc, max_scale);
	writeLog(buffer);
#endif

	Feature_pyramid *pyra = new Feature_pyramid;
	pyra->feat = (myArray<FPTYPE>**) calloc(max_scale, sizeof(myArray<FPTYPE>*));
	pyra->scale = new myArray<FPTYPE>(max_scale, 1);

	// the resize function wants floating point values
	// convert image to FPTYPE and 3 channels
	myArray<FPTYPE> imd(im->getRows(), im->getCols(), 3);
	int eltsNbr = im->getNumberOfElements();
	unsigned char *im_data = im->getPtr();
	FPTYPE *imd_data = imd.getPtr();
	if( im->getPlanes() == 1 )
	{
		for( int i = 0; i < eltsNbr; i++)
		{
			// duplicate channel 1 on channels 2 and 3
			imd_data[i] = im_data[i];
			imd_data[i + eltsNbr] = im_data[i];
			imd_data[i + 2*eltsNbr] = im_data[i];
		}
	}
	else if( im->getPlanes() == 3 )
	{
		// copy im data to imd
		for( int i = 0; i < eltsNbr; i++)
			imd_data[i] = im_data[i];
	}
	else
		throw std::runtime_error("Image must have 1 or 3 channel(s)!");

	// compute features
	for(int i = 1; i <= interval; i++)
	{
		myArray<FPTYPE> *scaled = resize(&imd, 1.0/pow(sc, i-1));
		pyra->feat[i-1] = features(scaled, sbin);
		pyra->scale->set(1.0/pow(sc, i-1), i-1);

		// remaining intervals
		for(int j = i+interval; j <= max_scale; j += interval)
		{
			myArray<FPTYPE> *reduced = reduce(scaled);
			delete scaled;
			scaled = reduced;
			pyra->feat[j-1] = features(scaled, sbin);
			pyra->scale->set(0.5*(pyra->scale->get(j-1-interval)), j-1);
		}

		delete scaled;
	}

	// pad features
	for( int i = 1; i <= max_scale; i++)
	{
		myArray<FPTYPE> *padded_feat = pad_feat(pyra->feat[i-1], padx, pady);
		delete pyra->feat[i-1];
		pyra->feat[i-1] = padded_feat;
	}
	
	FPTYPE *pyra_scale = pyra->scale->getPtr();
	for( int j = 0; j < max_scale; j++)
		pyra_scale[j] = sbin / pyra_scale[j];
	pyra->max_scale = max_scale;
	pyra->interval = interval;
	pyra->imy = imsizey;
	pyra->imx = imsizex;
	pyra->pady = pady;
	pyra->padx = padx;

	return pyra;
}

// delete feature pyramid
void delete_featpyramid(Feature_pyramid **pyra)
{
	if( ! *pyra )
		return;

	// free memory
	for( int i = 0; i < (*pyra)->max_scale; i++)
		delete (*pyra)->feat[i];
	free((*pyra)->feat);
	delete (*pyra)->scale;
	delete *pyra;
	*pyra = NULL;
}
