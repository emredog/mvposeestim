#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <queue> // for the priority queue
#include <opencv2/opencv.hpp>

// search space for pyramid searches. sometimes (e.g. when learning hyperparams) it's faster to
// just search the levels where we know probably the pose will be detected
// in the HumanEva experiments, I saw that it's generally around 13-17
#define MINLEVEL -1 // if negative, levels calculated during training will be used
#define MAXLEVEL -1 // else, these levels will be used

#if defined(WFV) || defined(AVS) // WEIGHTS FOR VIEWPOINTS

#if defined(HUMANEVA)
#define ROWS 484;
#define COLS 644;
std::string predictionFile = "/home/emredog/git/yrposeestim/yr_multiview_matlab/pose-release-FULL-ver1.3/code-basic/WeightsForViewpoints/wfv_data/predictions/finetune_he_06_predictions.csv";
#elif defined(UMPM)
#define ROWS 486
#define COLS 644
std::string predictionFile = "/home/emredog/git/yrposeestim/yr_multiview_matlab/pose-release-FULL-ver1.3/code-basic/WeightsForViewpoints/wfv_data/predictions/finetune_umpm_16_predictions.csv";
#endif

// single view median errors from training set
std::map<int, FPTYPE> initMedianErrorsMap()
{
    std::map<int, FPTYPE> medErrs;
#ifdef HUMANEVA
    medErrs[1-1] =  9.78313718239405;
    medErrs[2-1] =  8.30235370932268;
    medErrs[3-1] =  8.00845854020378;
    medErrs[4-1] =  22.4044306459876;
    medErrs[5-1] =  13.3696938317966;
    medErrs[6-1] =  13.7506188249566;
    medErrs[7-1] =  15.0899381106216;
    medErrs[8-1] =  9.57309965550523;
    medErrs[9-1] =  10.8662711407439;
    medErrs[10-1] = 10.2091848103777;
    medErrs[11-1] = 21.4575346104467;
    medErrs[12-1] = 13.0803636499000;
    medErrs[13-1] = 13.4702688223093;
    medErrs[14-1] = 17.9694271642385;
    medErrs[15-1] = 10.0361765752665;
    medErrs[16-1] = 24.4668959965729;
    medErrs[17-1] = 12.8327617065567;
    medErrs[18-1] = 12.5336295873443;
    medErrs[19-1] = 14.3972288998480;
    medErrs[20-1] = 8.88690768151459;
    medErrs[21-1] = 9.98602191946283;
    medErrs[22-1] = 9.39406887295127;
    medErrs[23-1] = 22.6348461433751;
    medErrs[24-1] = 11.2361022964662;
    medErrs[25-1] = 11.7311624457572;
    medErrs[26-1] = 13.6086380901691;
#elif defined(UMPM) // median singleview YRErrors 
// UMPM10 on train set               
    medErrs[1-1] =  4.71260;
    medErrs[2-1] =  4.37160;
    medErrs[3-1] =  6.77740;
    medErrs[4-1] =  7.73220;
    medErrs[5-1] =  9.64100;
    medErrs[6-1] =  12.6993;
    medErrs[7-1] =  16.1455;
    medErrs[8-1] =  6.15180;
    medErrs[9-1] =  6.08160;
    medErrs[10-1] = 6.12110;
    medErrs[11-1] = 5.93120;
    medErrs[12-1] = 5.74650;
    medErrs[13-1] = 5.23180;
    medErrs[14-1] = 4.43860;
    medErrs[15-1] = 6.20600;
    medErrs[16-1] = 6.42290;
    medErrs[17-1] = 7.56770;
    medErrs[18-1] = 9.87990;
    medErrs[19-1] = 11.4372;
    medErrs[20-1] = 5.73780;
    medErrs[21-1] = 5.74410;
    medErrs[22-1] = 5.83680;
    medErrs[23-1] = 5.67080;
    medErrs[24-1] = 4.56150;
    medErrs[25-1] = 4.97870;
    medErrs[26-1] = 4.52870;
#endif //ifdef HUMANEVA / elif defined(UMPM)
    return medErrs;
}

std::map<int, FPTYPE> singleViewMedianErrors = initMedianErrorsMap();

// inaccuracy (mean error) on single view error estimation (to use as an additional offset)
std::map<int, FPTYPE> initExpectedErrEstInaccuracy()
{
    std::map<int, FPTYPE> expErrEstInac;
#ifdef HUMANEVA
#ifdef AVS // Adaptive Viewpoint Selection
    // === Calculated on actual set of he09, for the CNN: finetune_he06_weights.hdf5
    expErrEstInac[1-1] =   8.29422192892976;
    expErrEstInac[2-1] =   3.33345534053354;
    expErrEstInac[3-1] =   6.03345248508699;
    expErrEstInac[4-1] =   7.08087985239318;
    expErrEstInac[5-1] =   11.0476899743232;
    expErrEstInac[6-1] =   12.0681618131760;
    expErrEstInac[7-1] =   15.9062923634253;
    expErrEstInac[8-1] =   5.43673415136194;
    expErrEstInac[9-1] =   4.74703164831585;
    expErrEstInac[10-1] =  4.56361365577300;
    expErrEstInac[11-1] =  6.26389956519879;
    expErrEstInac[12-1] =  8.63456100529535;
    expErrEstInac[13-1] =  9.79314162953738;
    expErrEstInac[14-1] =  12.6873084236859;
    expErrEstInac[15-1] =  6.57602874803541;
    expErrEstInac[16-1] =  7.06510309737576;
    expErrEstInac[17-1] =  10.9952222636080;
    expErrEstInac[18-1] =  12.0305352028764;
    expErrEstInac[19-1] =  16.6914319081470;
    expErrEstInac[20-1] =  5.65404937647564;
    expErrEstInac[21-1] =  4.67711102911713;
    expErrEstInac[22-1] =  4.39050950266745;
    expErrEstInac[23-1] =  6.05237023316719;
    expErrEstInac[24-1] =  8.10523155072496;
    expErrEstInac[25-1] =  9.46193947272834;
    expErrEstInac[26-1] =  12.0613704031297;
#endif // ifdef AVS

#elif defined(UMPM)
#ifdef AVS
// === Calculated on umpm10 validation data, for the CNN: finetune_umpm_16_weights.hdf5
    expErrEstInac[1-1] =   1.84596254034401;
    expErrEstInac[2-1] =   2.09170141852983;
    expErrEstInac[3-1] =   4.93839658711549;
    expErrEstInac[4-1] =   6.22962573456900;
    expErrEstInac[5-1] =   8.30896609295971;
    expErrEstInac[6-1] =   10.8594626586157;
    expErrEstInac[7-1] =   14.0464428002356;
    expErrEstInac[8-1] =   4.29426390014042;
    expErrEstInac[9-1] =   3.69971458896637;
    expErrEstInac[10-1] =  3.73417061807980;
    expErrEstInac[11-1] =  4.85614101913731;
    expErrEstInac[12-1] =  6.79838502430504;
    expErrEstInac[13-1] =  7.37187909839112;
    expErrEstInac[14-1] =  8.61933905229370;
    expErrEstInac[15-1] =  5.49988624428888;
    expErrEstInac[16-1] =  23.2577953812038;
    expErrEstInac[17-1] =  9.39126718117708;
    expErrEstInac[18-1] =  11.9448943458527;
    expErrEstInac[19-1] =  15.3775202195926;
    expErrEstInac[20-1] =  4.98596833524438;
    expErrEstInac[21-1] =  4.54131012272930;
    expErrEstInac[22-1] =  3.93130814546678;
    expErrEstInac[23-1] =  4.97494291833853;
    expErrEstInac[24-1] =  6.80539339130505;
    expErrEstInac[25-1] =  7.73396854118898;
    expErrEstInac[26-1] =  9.35173399623231;
#endif // AVS
#endif
    return expErrEstInac;
}

std::map<int, FPTYPE> expectedInaccuracyForSingleViewErrEst = initExpectedErrEstInaccuracy();

#endif // if defined(WFV) || defined(AVS)

#if WITH_MATLAB
#include "mex.h"
#endif

#if WITH_OPENCV
#include <opencv2/opencv.hpp>
#endif

#include "model.h"
#include "myarray.hpp"
#include "feature_pyramid.h"
#include "component.h"
#include "filter_response.h"
#include "passmsg.h"
#include "nms.h"
#include "logging.h"
#include "top.h"
#include "epipolarheatmap.h"
#include "epipolargeometry.h"
#include "displaytools.h"
#include "parttypecompat.h"
#ifdef AVS
#include "estimateyrerrorcnn.h"
#endif

#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>


static void split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}
//--------------------------------------------------------------------
static inline void boxCenter(FPTYPE x1, FPTYPE y1, FPTYPE x2, FPTYPE y2,
                             /*OUTPUTS:*/ double *centerX, double *centerY)
{
    *centerX = (x2-x1)/2.0 + x1;
    *centerY = (y2-y1)/2.0 + y1;
}

//--------------------------------------------------------------------

void saveBoxCenters(const std::vector<FPTYPE> *boxes, int partsNbr, const char* fileName)
{
    int detectionsNbr = boxes->size() / (4*partsNbr+2);
    char buffer[250];
    double corners[4];
    double centerX, centerY;
    std::ofstream txtFile;
    txtFile.open(fileName);

    for(int d = 1; d <= detectionsNbr; d++)
    {
        std::sprintf(buffer, "detection:%d,component:%d,score:%.5g\n", d, (int)(*boxes)[(d)*(4*partsNbr+2) - 2], (*boxes)[(d)*(4*partsNbr+2) -1]);
        txtFile << buffer;

        // parts coordinates
        for(int k = 1; k <= partsNbr; k++)
        {
            for(int i = 0; i < 4; i++)
                corners[i] = (*boxes)[(d-1)*(4*partsNbr+2) + 4*(k-1) + i];

            boxCenter(corners[0], corners[1], corners[2], corners[3], &centerX, &centerY);

            std::sprintf(buffer, "part%02d,%.6g,%.6g\n", k, centerX, centerY);
            txtFile << buffer;
        }
    }

    txtFile.close();
}
//--------------------------------------------------------------------

std::string outputFolder = "";
std::string imgFileA;
std::string viewA;
int frame = -1;



#if defined(MV) || defined(MV_PTC)
std::string imgFileB;
std::string viewB;

FPTYPE heatMultiplier;
FPTYPE partTypeMultiplier;




static inline double calculateEpsilon(const std::vector<FPTYPE>& oldBoxes, const std::vector<FPTYPE>& newBoxes)
{
    int nbOfParts = (oldBoxes.size() - 2) / 4;

    //check for scale change (x2-x1 in old box should be equal to x2-x1 in new box
    if (oldBoxes.at(2) - oldBoxes.at(0) != newBoxes.at(2) - newBoxes.at(0))
        std::cout << "WARNING different scale from previous detection.\n"
                  << "\tOld width of box: " << oldBoxes.at(2) - oldBoxes.at(0) << std::endl
                  << "\tNew width of box: " << newBoxes.at(2) - newBoxes.at(0) << std::endl;

    double totalDistance = 0;

    //for each part
    for (int p=0; p<nbOfParts; p++)
    {
        double oldCenterX, oldCenterY, newCenterX, newCenterY;
        boxCenter(oldBoxes.at(4*p + 0), oldBoxes.at(4*p + 1), oldBoxes.at(4*p + 2), oldBoxes.at(4*p + 3),
                  &oldCenterX, &oldCenterY);
        boxCenter(newBoxes.at(4*p + 0), newBoxes.at(4*p + 1), newBoxes.at(4*p + 2), newBoxes.at(4*p + 3),
                  &newCenterX, &newCenterY);

        double distance = std::sqrt((oldCenterX-newCenterX)*(oldCenterX-newCenterX) + (oldCenterY-newCenterY)*(oldCenterY-newCenterY));
        totalDistance += distance;
    }

    double meanDistance = totalDistance / static_cast<double>(nbOfParts);
    return meanDistance;
}

#endif //defined(MV) || defined(MV_PTC)

//--------------------------------------------------------------------
//--------------------------------------------------------------------



//--------------------------------------------------------------------
// function that is equivalent of "mkdir -p /a/b/c/d/e" (the mkdir(2) that we use does not create the intermediate folders)
//more info on: http://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
static int _mkdir(const char *dir, mode_t mode)
{
        char tmp[512];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/')
                {
                        *p = 0;
                        mkdir(tmp, mode);
                        *p = '/';
                }
         return mkdir(tmp, mode);
}


// Original backtracking with distance transform
void backtrack_C(FPTYPE thresh, myArray<FPTYPE> *rscore, myArray<int> **Ix, myArray<int> **Iy, myArray<int> **Ik, const Component *parts, const Feature_pyramid *pyra, int *parts_level, int componentId, /*output*/ std::vector<FPTYPE>* &boxes, int *detectionNbr)
{
    int numparts = parts->filterid->getRows();
    int rows = rscore->getRows();
    int cols = rscore->getCols();
#if LOGGING_ON
    std::vector<int> xptrv_debug;
    std::vector<int> yptrv_debug;
    std::vector<int> mptrv_debug;
#endif

    int currentDetectionNbr = 0; // detections number for current call

    for( int c = 1; c <= cols; c++)
    {
        for( int r = 1; r <= rows; r++)
        {
            // search sucessfull detections
            FPTYPE score = rscore->get(r-1, c-1);
            if( score >= thresh )
            {
                // new detection
                // get box position for each body part
                int xptr[numparts];
                int yptr[numparts];
                int mptr[numparts];

                for( int k = 1; k <= numparts; k++)
                {
                    if( k == 1 )
                    {
                        xptr[k-1] = c;
                        yptr[k-1] = r;
                        mptr[k-1] = Ik[1-1]->get(r-1, c-1);
                    }
                    else
                    {
                        int par = parts->parent->get(k-1);
                        int xpar = xptr[par-1];
                        int ypar = yptr[par-1];
                        int mpar = mptr[par-1];


                        xptr[k-1] = Ix[k-1]->get(ypar-1, xpar-1, mpar-1);
                        yptr[k-1] = Iy[k-1]->get(ypar-1, xpar-1, mpar-1);
                        mptr[k-1] = Ik[k-1]->get(ypar-1, xpar-1, mpar-1);
                    }
#if LOGGING_ON
                    xptrv_debug.push_back(xptr[k-1]);
                    yptrv_debug.push_back(yptr[k-1]);
                    mptrv_debug.push_back(mptr[k-1]);
#endif
                    int p_level = parts_level[k-1];
                    FPTYPE *pyra_scale = pyra->scale->getPtr();
                    FPTYPE scale = pyra_scale[p_level-1];
                    FPTYPE padx = pyra->padx;
                    FPTYPE pady = pyra->pady;
                    FPTYPE *p_sizx = parts->sizex->getPtr(0, 0, k-1);
                    FPTYPE *p_sizy = parts->sizey->getPtr(0, 0, k-1);

                    FPTYPE x1 = (xptr[k-1] - 1 - padx)*scale + 1;
                    FPTYPE y1 = (yptr[k-1] - 1 - pady)*scale + 1;
                    FPTYPE x2 = x1 + p_sizx[mptr[k-1]-1]*scale - 1;
                    FPTYPE y2 = y1 + p_sizy[mptr[k-1]-1]*scale - 1;

                    boxes->push_back(x1);
                    boxes->push_back(y1);
                    boxes->push_back(x2);
                    boxes->push_back(y2);
                }

                boxes->push_back(componentId);
                boxes->push_back(score);

                currentDetectionNbr++;
                (*detectionNbr)++;
            }
        }
    }

#if LOGGING_ON
    // debug
    if( currentDetectionNbr > 0 )
    {
        // display xptr, yptr and mptr
        myArray<int> _xptr(currentDetectionNbr, numparts);
        myArray<int> _yptr(currentDetectionNbr, numparts);
        myArray<int> _mptr(currentDetectionNbr, numparts);
        for(int d = 1; d <= currentDetectionNbr; d++)
        {
            for(int k = 1; k <= numparts; k++)
            {
                _xptr.set(xptrv_debug[(d-1)*numparts + k-1], d-1, k-1);
                _yptr.set(yptrv_debug[(d-1)*numparts + k-1], d-1, k-1);
                _mptr.set(mptrv_debug[(d-1)*numparts + k-1], d-1, k-1);
            }
        }
        writeLog("backtrack xptr=");
        writeLog(&_xptr);
        writeLog("backtrack yptr=");
        writeLog(&_yptr);
        writeLog("backtrack mptr=");
        writeLog(&_mptr);

        // display final detection boxes
        myArray<FPTYPE> _boxes(currentDetectionNbr, 4*numparts);
        for(int d = 1, dg = (*detectionNbr)-currentDetectionNbr+1; d <= currentDetectionNbr; d++, dg++)
        {
            for(int k = 1; k <= numparts; k++)
                for(int i = 0; i < 4; i++)
                    _boxes.set((*boxes)[(dg-1)*(4*numparts+2) + 4*(k-1) + i], d-1, 4*(k-1)+i); // we keep only the 4*numparts first values of each line
        }
        writeLog("backtrack box=");
        writeLog(&_boxes);
    }
#endif
}

//--------------------------------------------------------------------

// NO THRESHOLD backtracking WITH distance transform (just picks the best scoring root)
void backtrack_C(myArray<FPTYPE> *rscore, myArray<int> **Ix, myArray<int> **Iy, myArray<int> **Ik, const Component *parts, const Feature_pyramid *pyra, int *parts_level, int componentId, /*output*/ std::vector<FPTYPE>* &boxes)
{
    int numparts = parts->filterid->getRows();
    int rows = rscore->getRows();
    int cols = rscore->getCols();

    FPTYPE maxScore = -DBL_MAX;
    int rowOfMax = -1, colOfMax = -1;

    for( int c = 1; c <= cols; c++)
    {
        for( int r = 1; r <= rows; r++)
        {
            // search sucessfull detections
            FPTYPE score = rscore->get(r-1, c-1);
            if (score > maxScore)
            {
                maxScore = score;
                rowOfMax = r;
                colOfMax = c;
            }
        }
    }

    int xptr[numparts];
    int yptr[numparts];
    int mptr[numparts];

    for( int k = 1; k <= numparts; k++)
    {
        if( k == 1 )
        {
            xptr[k-1] = colOfMax;
            yptr[k-1] = rowOfMax;
            mptr[k-1] = Ik[1-1]->get(rowOfMax-1, colOfMax-1);
        }
        else
        {
            int par = parts->parent->get(k-1);
            int xpar = xptr[par-1];
            int ypar = yptr[par-1];
            int mpar = mptr[par-1];


            xptr[k-1] = Ix[k-1]->get(ypar-1, xpar-1, mpar-1);
            yptr[k-1] = Iy[k-1]->get(ypar-1, xpar-1, mpar-1);
            mptr[k-1] = Ik[k-1]->get(ypar-1, xpar-1, mpar-1);
        }

        int p_level = parts_level[k-1];
        FPTYPE *pyra_scale = pyra->scale->getPtr();
        FPTYPE scale = pyra_scale[p_level-1];
        FPTYPE padx = pyra->padx;
        FPTYPE pady = pyra->pady;
        FPTYPE *p_sizx = parts->sizex->getPtr(0, 0, k-1);
        FPTYPE *p_sizy = parts->sizey->getPtr(0, 0, k-1);

        FPTYPE x1 = (xptr[k-1] - 1 - padx)*scale + 1;
        FPTYPE y1 = (yptr[k-1] - 1 - pady)*scale + 1;
        FPTYPE x2 = x1 + p_sizx[mptr[k-1]-1]*scale - 1;
        FPTYPE y2 = y1 + p_sizy[mptr[k-1]-1]*scale - 1;

        boxes->push_back(x1);
        boxes->push_back(y1);
        boxes->push_back(x2);
        boxes->push_back(y2);
    }

    boxes->push_back(componentId);
    boxes->push_back(maxScore);


}

//--------------------------------------------------------------------

// Original detect_fast: calculate pose estimation with distance transform
std::vector<FPTYPE>* detect_fast_C(const myArray<unsigned char> *img, const Model *model,
                                   /*OUTPUT*/ myArray<FPTYPE> ***copyOfResp = NULL,
                                   /*INPUT*/ Feature_pyramid *pyra = NULL)
{
    bool isPyraManagedOutside = true;
    // Compute the feature pyramid and prepare filter
    if (pyra == NULL)
    {
        pyra = featpyramid_C(img, model);
        isPyraManagedOutside = false;
    }

    // Cache various statistics derived from model
    Component components[model->componentsNbr];
    myArray<FPTYPE> **filters;
    modelcomponents_C(model, pyra, /*outputs*/ components, &filters);

    myArray<FPTYPE> **pyra_feat = pyra->feat;
    int levels = pyra->max_scale;
    int componentsNb = model->componentsNbr;
    int numparts = 0; // number of parts ; supposed to be constant over all components

    std::vector<FPTYPE> *boxes = new std::vector<FPTYPE>; // detections boxes
    int detectionNbr = 0; // number of detections
    FPTYPE interval = model->interval;

#ifdef USE_CUDA
    initCuda();
#endif

    // Iterate over scales and components
    int minLevel = (MINLEVEL > 0) ? MINLEVEL : 1;
    int maxLevel = (MAXLEVEL > 0) ? MAXLEVEL : levels;
    for( int rlevel = minLevel; rlevel <= maxLevel; rlevel++)
    {
        myArray<FPTYPE> **resp = NULL;


        for( int c = 1; c <= componentsNb; c++)
        {
            Component *parts = &(components[c-1]);
            numparts = model->partsNbr;
            myArray<FPTYPE> **parts_score[numparts];
            // parts_score[k-1][fi-1] is a myArray* that
            // points to a 2D array
            int parts_level[numparts];

            // Local scores
            for( int k = 1; k <= numparts; k++)
            {
                int fNb = parts->filterid->getCols();
                myArray<int> *f = parts->filterid;

                FPTYPE scale = parts->scale->get(k-1);
                int level = rlevel - scale*interval;
                if( ! resp )
                {
                    // computed one time per rlevel
                    const myArray<FPTYPE> *pyra_feat_level = pyra_feat[level-1];
                    int filtersNb =  model->filtersSz;
#ifdef USE_CUDA
                    // GPU VERSION
                    resp = fconv_cuda(pyra_feat_level, filters, 1, filtersNb);
#else
                    // CPU VERSION
                    resp = fconvMT_C(pyra_feat_level, filters, 1, filtersNb);

                    // take a deep copy of the resp for this level
                    if (copyOfResp != NULL)
                    {
                        copyOfResp[rlevel-1] = new myArray<FPTYPE>*[filtersNb];
                        for (int indFilter = 0; indFilter < filtersNb; indFilter++)
                            copyOfResp[rlevel-1][indFilter] = new myArray<FPTYPE>(resp[indFilter]);
                    } // will be deleted outside
#endif
                }

                // allocate memory to store one score per filterid
                parts_score[k-1] = (myArray<FPTYPE>**) calloc(fNb, sizeof(myArray<FPTYPE>*));
                for( int fi = 1; fi <= fNb; fi++)
                    parts_score[k-1][fi-1] = resp[f->get(k-1, fi-1) - 1];

                parts_level[k-1] = level;
            }

            // Walk from leaves to root of tree, passing message to parent
            myArray<int> *Ix[numparts], *Iy[numparts], *Ik[numparts];
            for( int k = numparts; k >= 2; k--)
            {
                int par = parts->parent->get(k-1); // parent id

                myArray<FPTYPE> *msg;
                passmsg_C( k, par, parts, parts_score, /*outputs*/ &msg, Ix, Iy, Ik);

                // update parent score
                // parts(par).score = parts(par).score + msg;
                int fNb = parts->filterid->getCols();
                for( int fi = 1; fi <= fNb; fi++)
                {
                    myArray<FPTYPE> *parent_score = parts_score[par-1][fi-1]; // 2D array

                    int nbElts = parent_score->getNumberOfElements();

                    FPTYPE *parent_score_data = parent_score->getPtr(); // 2D array
                    FPTYPE *msg_data = msg->getPtr(0, 0, fi-1); // 3D array
                    for( int i = 0; i < nbElts; i++)
                        parent_score_data[i] += msg_data[i];
                }

                delete msg;
            }

            // Add bias to root score
            // parts(1).score = parts(1).score + parts(1).b;
            int fNb = parts->filterid->getCols();
            FPTYPE root_b = parts->b->get(1-1, 1-1, 1-1); // parts(1).b is a single value
            int Nx = parts_score[1-1][1-1]->getCols();
            int Ny = parts_score[1-1][1-1]->getRows();
            myArray<FPTYPE> rscore(Ny, Nx); // max score array
            Ix[1-1] = NULL; // to smake deallocation easier
            Iy[1-1] = NULL; // to smake deallocation easier
            Ik[1-1] = new myArray<int>(Ny, Nx);
            for( int fi = 1; fi <= fNb; fi++)
            {
                myArray<FPTYPE> *root_score = parts_score[1-1][fi-1]; // 2D array
                int rows = root_score->getRows();
                int cols = root_score->getCols();
                if( Ny != rows  ||  Nx != cols )
                    printf("ERROR: Array size mismatch !!!!\n");

                for( int r = 1 ; r <= rows; r++)
                {
                    for( int c = 1; c <= cols; c++)
                    {
                        FPTYPE *_root_score = root_score->getPtr(r-1, c-1);
                        *_root_score += root_b;

                        FPTYPE *_rscore = rscore.getPtr(r-1, c-1);

                        // init or update the max score array
                        if( fi == 1 || (*_root_score > *_rscore) )
                        {
                            // replace max score
                            *_rscore = *_root_score;
                            Ik[1-1]->set(fi, r-1, c-1);
                        }
                    }
                }
            }

            // Walk back down tree following pointers
            FPTYPE thresh = std::min(model->thresh, (FPTYPE) -1.0);
            backtrack_C(thresh, &rscore, Ix, Iy, Ik, parts, pyra, parts_level, c, /*output*/ boxes, &detectionNbr);

            // free memory
            for( int k = 1; k <= numparts; k++)
            {
                free(parts_score[k-1]);
                delete Ix[k-1];
                delete Iy[k-1];
                delete Ik[k-1];
            }
        }

        // free resp here
        int len = model->filtersSz - 1 + 1;
        for(int i = 0; i < len; i++)
            delete resp[i];
        free(resp);


    }

    delete_components(components, model->componentsNbr);
    if (!isPyraManagedOutside)
        delete_featpyramid(&pyra);

#ifdef USE_CUDA
    releaseCuda();
#endif

    return boxes;
}


//--------------------------------------------------------------------

void display_boxes(const std::vector<FPTYPE> *boxes, int partsNbr)
{
    int detectionsNbr = boxes->size() / (4*partsNbr+2);
    for(int d = 1; d <= detectionsNbr; d++)
    {
        printf("detection #%d:\n", d);

        // parts coordinates
        for(int k = 1; k <= partsNbr; k++)
        {
            printf(" - part #%d:", k);
            for(int i = 0; i < 4; i++)
                printf(" %.5g", (*boxes)[(d-1)*(4*partsNbr+2) + 4*(k-1) + i]);
            printf("\n");
        }

        // component Id
        printf(" - component id: %.5g\n", (*boxes)[(d-1)*(4*partsNbr+2) + 4*partsNbr]);

        // score
        printf(" - score: %.5g\n", (*boxes)[(d-1)*(4*partsNbr+2) + 4*partsNbr+1]);
    }
}
//--------------------------------------------------------------------

#ifdef MV

//--------------------------------------------------------------------
// calculate pose estimation with distance transform
std::vector<FPTYPE>* detect_fast_withEpiConstraint(const myArray<unsigned char> *img, const Model *model,
                                                   const std::vector<FPTYPE> *supportBoxes,
                                                   const EpipolarGeometry *epiGeometry,
                                                   myArray<FPTYPE>*** filterResponse,
                                                   Feature_pyramid *pyra = NULL)
{
    bool isPyraManagedOutside = true;
    // Compute the feature pyramid and prepare filter
    if (pyra == NULL)
    {
        pyra = featpyramid_C(img, model);
        isPyraManagedOutside = false;
    }

    // Cache various statistics derived from model
    Component components[model->componentsNbr];
    myArray<FPTYPE> **filters;
    modelcomponents_C(model, pyra, /*outputs*/ components, &filters);

    int levels = pyra->max_scale;
    int componentsNb = model->componentsNbr;
    int numparts = 0; // number of parts ; supposed to be constant over all components

    std::vector<FPTYPE> *boxes = new std::vector<FPTYPE>(); // detections boxes
    boxes->reserve(1500000);
    int detectionNbr = 0; // number of detections
    FPTYPE interval = model->interval;

    // Iterate over scales and components
    int minLevel = (MINLEVEL > 0) ? MINLEVEL : 1;
    int maxLevel = (MAXLEVEL > 0) ? MAXLEVEL : levels;
    for( int rlevel = minLevel; rlevel <= maxLevel; rlevel++)
    {
        // Copy data in filterResponse into resp
        // because parts_score point to resp, and modify it during message passing,
        // and this is resp that we'll use in other iterations
        int len = model->filtersSz - 1 + 1;

        myArray<FPTYPE> **resp = (myArray<FPTYPE>**) calloc(len, sizeof(myArray<FPTYPE>*)); //allocate memory for pointers

        for (int i = 0; i < len; i++)
        {
            resp[i] = new myArray<FPTYPE>(filterResponse[rlevel-1][i]); //create a deep copy of the filter response for rlevel and i
        }

        for( int c = 1; c <= componentsNb; c++)
        {
            Component *parts = &(components[c-1]);
            numparts = model->partsNbr;
            int fNb = parts->filterid->getCols();

            myArray<FPTYPE> **parts_score[numparts];
            // parts_score[k-1][fi-1] is a myArray* that
            // points to a 2D array
            int parts_level[numparts];

            // Local scores
            for( int k = 1; k <= numparts; k++)
            {

                myArray<int> *f = parts->filterid;
                FPTYPE scale = parts->scale->get(k-1);
                int level = rlevel - scale*interval;
                //no need to recalculate response, we already have it.

                // allocate memory to store one score per filterid
                parts_score[k-1] = (myArray<FPTYPE>**) calloc(fNb, sizeof(myArray<FPTYPE>*));
                for( int fi = 1; fi <= fNb; fi++)
                    parts_score[k-1][fi-1] = resp[f->get(k-1, fi-1) - 1];

                parts_level[k-1] = level;
            }


            // ////////////////////////////////////////////////////////////////////////////////////
            //      EPIPOLAR CONSTRAINTS START HERE
            // ////////////////////////////////////////////////////////////////////////////////////
            // FIXME: merge this with the for loop above OR fetch responses from previous runs
            for( int k = 1; k <= numparts; k++)
            {
                // get position from supportBoxes (the detection in the other view)
                double x1 = (*supportBoxes)[4*(k-1) + 0];
                double y1 = (*supportBoxes)[4*(k-1) + 1];
                double x2 = (*supportBoxes)[4*(k-1) + 2];
                double y2 = (*supportBoxes)[4*(k-1) + 3];

                // calculate the box center
                double centerX = 0, centerY = 0;
                boxCenter(x1, y1, x2, y2, &centerX, &centerY);

                //check box center is in the image
                if ( centerX < 0.0 || centerX >= (double)img->getCols() ||
                     centerY < 0.0 || centerY >= (double)img->getRows() )
                    continue; //not in the image

                // fetch corresponding intersection points with epipolar line & image borders, according the box center
                PointPair epiBorderPoints = epiGeometry->getEpiBorderPoints((int)round(centerX), (int)round(centerY));                

                cv::Mat epiHeatMapMat = EpipolarHeatmap::calculateEpiHeatmap(epiBorderPoints, //intersection of the epipolar line with the original image size
                                                                             epiGeometry->getOrigRowSize(), epiGeometry->getOrigColSize(), // original size where the epipolar lines are calculated
                                                                             parts_score[k-1][0]->getRows(), parts_score[k-1][0]->getCols()); //desired size of the heatmap


                // multiply epiHeatMap with multiplier (the amount of effect)
                epiHeatMapMat = epiHeatMapMat * heatMultiplier;

                // add it to parts_score for that part
                for( int fi = 1; fi <= fNb; fi++)
                {
                    //no need to check for sizes (epiheatmap is initialized with the correct values)
                    cv::MatConstIterator_<float> it, end = epiHeatMapMat.end<float>();
                    for (it = epiHeatMapMat.begin<float>(); it != end; ++it)
                    {
                        cv::Point2i pos = it.pos();
                        FPTYPE newVal = *it + parts_score[k-1][fi-1]->get(pos.y, pos.x);
                        parts_score[k-1][fi-1]->set(newVal, pos.y, pos.x);
                    }
                }
            }


            // ////////////////////////////////////////////////////////////////////////////////////
            //      MESSAGE PASSING STARTS HERE
            // ////////////////////////////////////////////////////////////////////////////////////

            // Walk from leaves to root of tree, passing message to parent
            myArray<int> *Ix[numparts], *Iy[numparts], *Ik[numparts];
            for( int k = numparts; k >= 2; k--)
            {
                int par = parts->parent->get(k-1); // parent id

                myArray<FPTYPE> *msg;
                passmsg_C( k, par, parts, parts_score, /*outputs*/ &msg, Ix, Iy, Ik);

                // update parent score
                // parts(par).score = parts(par).score + msg;
                for( int fi = 1; fi <= fNb; fi++)
                {
                    myArray<FPTYPE> *parent_score = parts_score[par-1][fi-1]; // 2D array

                    int nbElts = parent_score->getNumberOfElements();

                    FPTYPE *parent_score_data = parent_score->getPtr(); // 2D array
                    FPTYPE *msg_data = msg->getPtr(0, 0, fi-1); // 3D array
                    for( int i = 0; i < nbElts; i++)
                        parent_score_data[i] += msg_data[i];
                }

                delete msg;
            }

            // Add bias to root score
            // parts(1).score = parts(1).score + parts(1).b;
            FPTYPE root_b = parts->b->get(1-1, 1-1, 1-1); // parts(1).b is a single value
            int Nx = parts_score[1-1][1-1]->getCols();
            int Ny = parts_score[1-1][1-1]->getRows();
            myArray<FPTYPE> rscore(Ny, Nx); // max score array
            Ix[1-1] = NULL; // to smake deallocation easier
            Iy[1-1] = NULL; // to smake deallocation easier
            Ik[1-1] = new myArray<int>(Ny, Nx);
            for( int fi = 1; fi <= fNb; fi++)
            {
                myArray<FPTYPE> *root_score = parts_score[1-1][fi-1]; // 2D array
                int rows = root_score->getRows();
                int cols = root_score->getCols();
                if( Ny != rows  ||  Nx != cols )
                    printf("ERROR: Array size mismatch !!!!\n");

                for( int r = 1 ; r <= rows; r++)
                {
                    for( int c = 1; c <= cols; c++)
                    {
                        FPTYPE *_root_score = root_score->getPtr(r-1, c-1);
                        *_root_score += root_b;

                        FPTYPE *_rscore = rscore.getPtr(r-1, c-1);

                        // init or update the max score array
                        if( fi == 1 || (*_root_score > *_rscore) )
                        {
                            // replace max score
                            *_rscore = *_root_score;
                            Ik[1-1]->set(fi, r-1, c-1);
                        }
                    }
                }
            }

            // Walk back down tree following pointers
            FPTYPE thresh = std::min(model->thresh, (FPTYPE) -1.0);
            thresh += heatMultiplier;
            backtrack_C(thresh, &rscore, Ix, Iy, Ik, parts, pyra, parts_level, c, /*output*/ boxes, &detectionNbr);

//            backtrack_C(&rscore, Ix, Iy, Ik, parts, pyra, parts_level, c, /*output*/ boxes); //--> no threshold version
            // this version yielded EXTREMELY HIGH error when tested on a few example

            // free memory
            for( int k = 1; k <= numparts; k++)
            {
                //free(parts_score[k-1]);
                delete Ix[k-1];
                delete Iy[k-1];
                delete Ik[k-1];
            }
        }

        // free resp here
        for(int i = 0; i < len; i++)
            delete resp[i];
        free(resp);

        //        std::cout << "\tCalculation completed for level: " << rlevel << std::endl;
    }

    delete_components(components, model->componentsNbr);
    if (!isPyraManagedOutside)
        delete_featpyramid(&pyra);

    return boxes;
}

//--------------------------------------------------------------------

// Function that manage the iterations for the pose detection with epipolar constraints
std::pair<const std::vector<FPTYPE>*, const std::vector<FPTYPE>* > detect_fast_mv(const myArray<unsigned char> *imgA,
                                                                                  const myArray<unsigned char> *imgB, const Model *model,
                                                                                  const EpipolarGeometry* epiGeometryA2B, const EpipolarGeometry* epiGeometryB2A,
                                                                                  int maxIterations = 4, FPTYPE requiredEpsilon = 0.1)
{
    std::cout << "Running detect_fast_mv with maximum iterations: " << maxIterations << " and required epsilon: " << requiredEpsilon << std::endl;

    // char buffer[400]; //for filenames and stuff

    std::pair<const std::vector<FPTYPE>*, const std::vector<FPTYPE>* > boxPair(NULL, NULL);

    int nbOfParts = model->partsNbr;

    //INITIAL CONDITIONS: best of independent estimations on solo views

    Feature_pyramid *pyraA = featpyramid_C(imgA, model);
    Feature_pyramid *pyraB = featpyramid_C(imgB, model);
    int levels = pyraA->max_scale;


    // filter response backups
    myArray<FPTYPE> **filterResponseA[levels];
    myArray<FPTYPE> **filterResponseB[levels];

    // Run solo detect_fast for first image
    std::vector<FPTYPE> *boxesA = detect_fast_C(imgA, model, filterResponseA, pyraA);
    if (boxesA->empty())
        std::cerr << "WARNING\tNo detection for image A: detect_fast_C(imgA, model)\n";
    std::cout << "Detect fast for image A is completed with " << boxesA->size() << " instance of detections." << std::endl;
#ifdef USE_NMS
    std::vector<FPTYPE> *boxesA_nms = nms(boxesA, .1, nbOfParts);
#else
    std::vector<FPTYPE> *boxesA_nms = not_nms_pick_best(boxesA, nbOfParts); //we don't need NMS when there are only one subject
#endif //USE_NMS

#ifdef SAVE_IMAGES
    std::sprintf(buffer, "%s/%s_A_Iteration_0.png", outputFolder.c_str(), imgFileA.c_str());
    DisplayTools::saveDetection(*imgA, *boxesA_nms, nbOfParts, buffer);
    //std::sprintf(buffer, "%s/%s_A_Iteration_0", outputFolder.c_str(), imgFileA.c_str());
    //DisplayTools::saveDetectionPartBased(*imgA, *boxesA_nms, nbOfParts, buffer);

    std::sprintf(buffer, "%s/%s_B_EpiLinesFromBorderPoints_Iteration_0.png", outputFolder.c_str(), imgFileB.c_str());
    std::vector<PointPair> allEpiBorderPoints;
    for (int p=0; p<26; p++)
    {
        double centerX, centerY;
        boxCenter(boxesA_nms->at(4*p + 0), boxesA_nms->at(4*p + 1), boxesA_nms->at(4*p + 2), boxesA_nms->at(4*p + 3),
                  &centerX, &centerY);
        allEpiBorderPoints.push_back(epiGeometryA2B->getEpiBorderPoints((int)round(centerX), (int)round(centerY)));
    }
    DisplayTools::saveEpipolarLines(*imgB, allEpiBorderPoints, buffer);
    //DisplayTools::saveEpipolarLinesPartBased(*imgB, allEpiBorderPoints, buffer);
    allEpiBorderPoints.clear();
#endif   

    // Run solo detect_fast for second image
    std::vector<FPTYPE> *boxesB = detect_fast_C(imgB, model, filterResponseB, pyraB);
    if (boxesB->empty())
        std::cerr << "WARNING\tNo detection for image B: detect_fast_C(imgB, model)\n";
    std::cout << "Detect fast for image B is completed with " << boxesB->size() << " instance of detections." << std::endl;
#ifdef USE_NMS
    std::vector<FPTYPE> *boxesB_nms = nms(boxesB, .1, nbOfParts);
#else
    std::vector<FPTYPE> *boxesB_nms = not_nms_pick_best(boxesB, nbOfParts); // we don't need NMS when there are only one subject
#endif


#ifdef SAVE_IMAGES
    std::sprintf(buffer, "%s/%s_B_Iteration_0.png", outputFolder.c_str(), imgFileB.c_str());
    DisplayTools::saveDetection(*imgB, *boxesB_nms, nbOfParts, buffer);
    //std::sprintf(buffer, "%s/%s_B_Iteration_0", outputFolder.c_str(), imgFileB.c_str());
    //DisplayTools::saveDetectionPartBased(*imgB, *boxesB_nms, nbOfParts, buffer);

    std::sprintf(buffer, "%s/%s_A_EpiLinesFromBorderPoints_Iteration_0.png", outputFolder.c_str(), imgFileA.c_str());
    for (int p=0; p<26; p++)
    {
        double centerX, centerY;
        boxCenter(boxesB_nms->at(4*p + 0), boxesB_nms->at(4*p + 1), boxesB_nms->at(4*p + 2), boxesB_nms->at(4*p + 3),
                  &centerX, &centerY);
        allEpiBorderPoints.push_back(epiGeometryB2A->getEpiBorderPoints((int)round(centerX), (int)round(centerY)));
    }
    DisplayTools::saveEpipolarLines(*imgA, allEpiBorderPoints, buffer);
    //DisplayTools::saveEpipolarLinesPartBased(*imgA, allEpiBorderPoints, buffer);
    allEpiBorderPoints.clear();
#endif

#ifdef SAVE_TEXT
    //save boxes
    std::sprintf(buffer, "%s/%s_Solo_A_Pose.txt", outputFolder.c_str(), imgFileA.c_str());
    saveBoxCenters(boxesA_nms, model->partsNbr, buffer);
    std::sprintf(buffer, "%s/%s_Solo_B_Pose.txt", outputFolder.c_str(), imgFileB.c_str());
    saveBoxCenters(boxesB_nms, model->partsNbr, buffer);
#endif

    //clean up memory
    delete boxesA; boxesA = NULL;
    delete boxesB; boxesB = NULL;

    // check for detection results
    FPTYPE scoreA = -DBL_MAX, scoreB = -DBL_MAX;
    if ( boxesA_nms->empty() || boxesA_nms->size() % (4*nbOfParts+2) != 0 )
    {
        std::cerr << "Either NMS yielded empty detections for A or number of obtained boxes are inconsistent with the number of parts." << std::endl;
        delete boxesA_nms;
        boxesA_nms = NULL;
    }
    else
    {
        scoreA = (*boxesA_nms)[4*nbOfParts+1]; // this is the root score for detection A
        std::cout << ">> Score for image A: " << scoreA << std::endl;
    }

    if (boxesB_nms->empty() || boxesB_nms->size() % (4*nbOfParts+2) != 0 )
    {
        std::cerr << "Either NMS yielded empty detections for B or number of obtained boxes are inconsistent with the number of parts." << std::endl;
        delete boxesB_nms;
        boxesB_nms = NULL;
    }
    else
    {
        scoreB = (*boxesB_nms)[4*nbOfParts+1]; // this is the root score for detection B
        std::cout << ">> Score for image B: " << scoreB << std::endl;
    }

    // select the "better looking" result (based on their score) as initialization
    const myArray<unsigned char> *targetImg = NULL, *supportImg = NULL;
    const std::vector<FPTYPE> *targetBoxes = NULL, *supportBoxes = NULL;
    const EpipolarGeometry *supportToTargetEpiLines = NULL,  //epipolar lines on targetImg, based on boxes at supportImg
                           *targetToSupportEpiLines = NULL; //epipolar lines on supportImg, based on boxes at targetImg
    myArray<FPTYPE> ***targetFilterResponse = NULL, ***supportFilterResponse = NULL;
    Feature_pyramid *targetPyra = NULL, *supportPyra = NULL;


    if (scoreA >= scoreB)
    {
        //just assigning the pointers
        targetImg = imgB;  //keep these pointers to identify later
        targetBoxes = boxesB_nms; boxesB_nms = NULL;
        supportToTargetEpiLines = epiGeometryA2B; epiGeometryA2B = NULL;
        targetFilterResponse = filterResponseB;
        targetPyra = pyraB;

        supportImg = imgA;  //keep these pointers to identify later
        supportBoxes = boxesA_nms; boxesA_nms = NULL;
        targetToSupportEpiLines = epiGeometryB2A; epiGeometryB2A = NULL;
        supportFilterResponse = filterResponseA;
        supportPyra = pyraA;
    }
    else //if( scoreA < scoreB)
    {
        //just assigning the pointers
        targetImg = imgA;  //keep these pointers to identify later
        targetBoxes = boxesA_nms; boxesA_nms = NULL;
        supportToTargetEpiLines = epiGeometryB2A; epiGeometryB2A = NULL;
        targetFilterResponse = filterResponseA;
        targetPyra = pyraA;

        supportImg = imgB;  //keep these pointers to identify later
        supportBoxes = boxesB_nms; boxesB_nms = NULL;
        targetToSupportEpiLines = epiGeometryA2B; epiGeometryA2B = NULL;
        supportFilterResponse = filterResponseB;
        supportPyra = pyraB;
    }    


    // ================================================================================================
    //  START JOINT DETECTION HERE
    // ================================================================================================
    int iteration = 1;
    double actualEpsilon = DBL_MAX;
    double previousEpsilon = DBL_MAX;
    int nbOfBoxes = 4*nbOfParts+2; //last two is the component number and detection score
    int nbOfDetections;
    std::vector<FPTYPE> epsilons;
    do
    {
        // run it with supporting information
        std::vector<FPTYPE> *newBoxes = detect_fast_withEpiConstraint(targetImg, model,
                                                                      supportBoxes, supportToTargetEpiLines,
                                                                      targetFilterResponse, targetPyra);
        //targetEpiLines: epilines that are in targetImg, given a point in supportImg

        std::cout << "Joint detection is completed with " << newBoxes->size() << " instance of detections." << std::endl;
#ifdef USE_NMS
        std::vector<FPTYPE> *newBoxes_nms = nms(newBoxes, .1, nbOfParts);
#else
        std::vector<FPTYPE> *newBoxes_nms = not_nms_pick_best(newBoxes, nbOfParts); //we don't need NMS when there are only one
#endif

        delete newBoxes;    //clean up memory
        newBoxes = NULL;

        if (newBoxes_nms->size() % nbOfBoxes != 0)
            throw std::runtime_error("Detection completed, but obtained wrong number of boxes.\n\n");

        nbOfDetections = newBoxes_nms->size() / nbOfBoxes;

        if (nbOfDetections == 0) // no detection
        {
            delete newBoxes_nms; //this is actually empty, delete it to prevent memleak
            newBoxes_nms = NULL;
            //note: it's likely that we'll end up here every other iteration

            std::cout << "WARNING No detection with the epipolar constraint.\n";
        }
        else if (nbOfDetections == 1) //single detection
        {
            //  check for convergence here
            actualEpsilon = calculateEpsilon(*targetBoxes, *newBoxes_nms);

            epsilons.push_back(actualEpsilon);

            delete targetBoxes; //these ones are obsolete now
            targetBoxes = NULL;

            targetBoxes = newBoxes_nms; // save a pointer to new detection boxes
            newBoxes_nms = NULL; // these boxes are managed by targetBoxes from now on.
        }
#ifdef USE_NMS //multiple detection is only possible if do NMS.
        else // multiple detections
        {
            // TODO handle (BETTER!) the case with multiple detection
            std::cout << "WARNING Multiple detection obtained. Discarding the one(s) with lower score...\n";

            //create a copy for the best instance of the new boxes
            std::vector<FPTYPE> *copyOfNewBoxes = new std::vector<FPTYPE>(4*nbOfParts+2);

            // find the max scored detection
            FPTYPE maxScore = -DBL_MAX;
            for(int detectionIndex = 0; detectionIndex<nbOfDetections; detectionIndex++)
            {
                int offset = detectionIndex * nbOfBoxes;
                FPTYPE score = newBoxes_nms->at(detectionIndex * (4*nbOfParts+2));
                if (score > maxScore)
                {
                    maxScore = score;
                    //and copy it
                    //copyOfNewBoxes->clear(); // FIXME: does it work as expected?
                    std::vector<FPTYPE>::iterator itCopy, itNew = newBoxes_nms->begin() + offset;
                    for (itCopy = copyOfNewBoxes->begin(); itCopy != copyOfNewBoxes->end(); ++itCopy, ++itNew)
                        *itCopy = *itNew;
                }
            }
            // we don't need newBoxes_nms anymore, we already copied it
            delete newBoxes_nms; newBoxes_nms = NULL;

            // check for convergence here
            actualEpsilon = calculateEpsilon(*targetBoxes, *copyOfNewBoxes);

            epsilons.push_back(actualEpsilon);

            delete targetBoxes; //these ones are obsolete now
            targetBoxes = NULL;

            // what we need is the copied one
            targetBoxes = copyOfNewBoxes;
            copyOfNewBoxes = NULL; //memory allocated by copyOfNewBoxes is controlled by targetBoxes from now on.
        }
#endif //USE_NMS


#ifdef SAVE_IMAGES
        if (targetImg == imgA) //we processed imgA
            std::sprintf(buffer, "%s/%s_A_Iteration_%d.png", outputFolder.c_str(), imgFileA.c_str(), iteration);
        else //we processed imgB
            std::sprintf(buffer, "%s/%s_B_Iteration_%d.png", outputFolder.c_str(), imgFileB.c_str(), iteration);
        DisplayTools::saveDetection(*targetImg, *targetBoxes, nbOfParts, buffer);
        //DisplayTools::saveDetectionPartBased(*targetImg, *targetBoxes, nbOfParts, buffer);
        //        DisplayTools::displayDetection(*targetImg, *targetBoxes, nbOfParts, 1000);

        if (targetImg == imgA) //we processed imgA
            std::sprintf(buffer, "%s/%s_A_Pose_Iteration_%d.txt", outputFolder.c_str(), imgFileA.c_str(), iteration);
        else
            std::sprintf(buffer, "%s/%s_B_Pose_Iteration_%d.txt", outputFolder.c_str(), imgFileB.c_str(), iteration);
        saveBoxCenters(targetBoxes, nbOfParts, buffer);
#endif


        //swap the support and the target:
        std::swap(targetBoxes, supportBoxes);
        std::swap(targetImg, supportImg);
        std::swap(supportToTargetEpiLines, targetToSupportEpiLines);
        std::swap(targetFilterResponse, supportFilterResponse);
        std::swap(targetPyra, supportPyra);

        if (epsilons.size() > 2)
            std::cout << "Iteration " << iteration << " completed with new score: " << (*supportBoxes)[4*nbOfParts+1] << std::endl
                      << "\tActual eps: " << actualEpsilon
                      << "\tPrevious eps: " << epsilons.at(epsilons.size() - 3)
                      << "\tRequired eps: " << requiredEpsilon << std::endl;
        else
            std::cout << "Iteration " << iteration << " completed with new score: " << (*supportBoxes)[4*nbOfParts+1] << std::endl
                      << "\tActual eps: " << actualEpsilon
                      << "\tRequired eps: " << requiredEpsilon << std::endl;        

        if (epsilons.size() > 1)
            previousEpsilon = epsilons.at(epsilons.size() - 2);

        iteration++;
    }
    while(iteration-1 < maxIterations &&
          !(actualEpsilon <= requiredEpsilon && previousEpsilon <= requiredEpsilon) ); //additional condition to check "the other" epsilon as well before stopping

#ifdef SAVE_TEXT
    //save iteration and epsilon info
    {
        std::sprintf(buffer, "%s/%s_MV_stats.txt", outputFolder.c_str(), imgFileA.c_str());
        std::ofstream txtFile;
        txtFile.open(buffer);
        txtFile << iteration-1 << "\n";
        for (std::vector<FPTYPE>::iterator it = epsilons.begin() ; it != epsilons.end(); ++it) 
            txtFile << *it<< "\n";
        txtFile.close();
    }
#endif

    //check which one is which:
    if (supportImg == imgA)
    {
        //write boxes for A in the "first" slot, and B in the "second" one
        boxPair.first  = supportBoxes;
        boxPair.second = targetBoxes;

    }
    else if (supportImg == imgB)
    {
        //write boxes for A in the "first" slot, and B in the "second" one
        boxPair.first  = targetBoxes;
        boxPair.second = supportBoxes;
    }
    else
    {
        //something's wrong here.
        std::cerr << "Cannot match the boxes with either view A or B.\n";
        //there are still NULL's in the boxPair
    }

    // clean up the filter response objects
    int minLevel = (MINLEVEL > 0) ? MINLEVEL : 1;
    int maxLevel = (MAXLEVEL > 0) ? MAXLEVEL : levels;
    for( int rlevel = minLevel; rlevel <= maxLevel; rlevel++)
    {
        int len = model->filtersSz - 1 + 1;
        for(int i = 0; i < len; i++)
        {
            delete filterResponseA[rlevel-1][i];
            delete filterResponseB[rlevel-1][i];
        }
        delete filterResponseA[rlevel-1];
        delete filterResponseB[rlevel-1];
    }
    
    delete_featpyramid(&pyraA);
    delete_featpyramid(&pyraB);

    // FIXME: we got a segmentation fault here.
    //    delete filterResponseA;
    //    delete filterResponseB;

    return boxPair;
}
#endif //MV




#ifdef MV_PTC

// Original backtracking THAT KEEPS TRACK OF PART TYPES
void backtrack_C_pt(FPTYPE thresh, myArray<FPTYPE> *rscore, myArray<int> **Ix, myArray<int> **Iy, myArray<int> **Ik, const Component *parts, const Feature_pyramid *pyra, int *parts_level, int componentId, /*output*/ std::vector<FPTYPE>* &boxes, std::vector<int>* &partTypes, int *detectionNbr)
{
    int numparts = parts->filterid->getRows();
    int rows = rscore->getRows();
    int cols = rscore->getCols();
#if LOGGING_ON
    std::vector<int> xptrv_debug;
    std::vector<int> yptrv_debug;
    std::vector<int> mptrv_debug;
#endif
    
    //TODO: null-check boxes and partTypes
    // (do it for all backtrack versions)
    
    int currentDetectionNbr = 0; // detections number for current call
    
    for( int c = 1; c <= cols; c++)
    {
        for( int r = 1; r <= rows; r++)
        {
            // search sucessfull detections
            FPTYPE score = rscore->get(r-1, c-1);
            if( score >= thresh )
            {
                // new detection
                // get box position for each body part
                int xptr[numparts];
                int yptr[numparts];
                int mptr[numparts];
                
                for( int k = 1; k <= numparts; k++)
                {
                    if( k == 1 )
                    {
                        xptr[k-1] = c;
                        yptr[k-1] = r;
                        mptr[k-1] = Ik[1-1]->get(r-1, c-1);
                    }
                    else
                    {
                        int par = parts->parent->get(k-1);
                        int xpar = xptr[par-1];
                        int ypar = yptr[par-1];
                        int mpar = mptr[par-1];
                        
                        
                        xptr[k-1] = Ix[k-1]->get(ypar-1, xpar-1, mpar-1);
                        yptr[k-1] = Iy[k-1]->get(ypar-1, xpar-1, mpar-1);
                        mptr[k-1] = Ik[k-1]->get(ypar-1, xpar-1, mpar-1);
                    }
#if LOGGING_ON
                    xptrv_debug.push_back(xptr[k-1]);
                    yptrv_debug.push_back(yptr[k-1]);
                    mptrv_debug.push_back(mptr[k-1]);
#endif
                    int p_level = parts_level[k-1];
                    FPTYPE *pyra_scale = pyra->scale->getPtr();
                    FPTYPE scale = pyra_scale[p_level-1];
                    FPTYPE padx = pyra->padx;
                    FPTYPE pady = pyra->pady;
                    FPTYPE *p_sizx = parts->sizex->getPtr(0, 0, k-1);
                    FPTYPE *p_sizy = parts->sizey->getPtr(0, 0, k-1);
                    
                    FPTYPE x1 = (xptr[k-1] - 1 - padx)*scale + 1;
                    FPTYPE y1 = (yptr[k-1] - 1 - pady)*scale + 1;
                    FPTYPE x2 = x1 + p_sizx[mptr[k-1]-1]*scale - 1;
                    FPTYPE y2 = y1 + p_sizy[mptr[k-1]-1]*scale - 1;
                    
                    boxes->push_back(x1);
                    boxes->push_back(y1);
                    boxes->push_back(x2);
                    boxes->push_back(y2);
                    partTypes->push_back(mptr[k-1]);
                }
                
                boxes->push_back(componentId);
                boxes->push_back(score);
                
                currentDetectionNbr++;
                (*detectionNbr)++;
            }
        }
    }
    
#if LOGGING_ON
    // debug
    if( currentDetectionNbr > 0 )
    {
        // display xptr, yptr and mptr
        myArray<int> _xptr(currentDetectionNbr, numparts);
        myArray<int> _yptr(currentDetectionNbr, numparts);
        myArray<int> _mptr(currentDetectionNbr, numparts);
        for(int d = 1; d <= currentDetectionNbr; d++)
        {
            for(int k = 1; k <= numparts; k++)
            {
                _xptr.set(xptrv_debug[(d-1)*numparts + k-1], d-1, k-1);
                _yptr.set(yptrv_debug[(d-1)*numparts + k-1], d-1, k-1);
                _mptr.set(mptrv_debug[(d-1)*numparts + k-1], d-1, k-1);
            }
        }
        writeLog("backtrack xptr=");
        writeLog(&_xptr);
        writeLog("backtrack yptr=");
        writeLog(&_yptr);
        writeLog("backtrack mptr=");
        writeLog(&_mptr);
        
        // display final detection boxes
        myArray<FPTYPE> _boxes(currentDetectionNbr, 4*numparts);
        for(int d = 1, dg = (*detectionNbr)-currentDetectionNbr+1; d <= currentDetectionNbr; d++, dg++)
        {
            for(int k = 1; k <= numparts; k++)
                for(int i = 0; i < 4; i++)
                    _boxes.set((*boxes)[(dg-1)*(4*numparts+2) + 4*(k-1) + i], d-1, 4*(k-1)+i); // we keep only the 4*numparts first values of each line
        }
        writeLog("backtrack box=");
        writeLog(&_boxes);
    }
#endif
}

// Original detect_fast: calculate pose estimation AND KEEP TRACK OF PART TYPES
std::vector<FPTYPE>* detect_fast_C_pt(const myArray<unsigned char> *img, const Model *model,
                                      /*OUTPUT*/std::vector<int>* &partTypes,
                                             myArray<FPTYPE> ***copyOfResp = NULL,
                                   /*INPUT*/ Feature_pyramid *pyra = NULL)
{
#if LOGGING_ON
    //debug
    writeLog("detect_fast im=");
    writeLog(img);
    
    writeLog("detect_fast model.bias.w=");
    writeLog(model->bias_w);
    writeLog("detect_fast model.bias.i=");
    writeLog(model->bias_i);
    
    writeLog("detect_fast model.filters.w=");
    for(int j = 0; j < model->filtersSz; j++)
        writeLog(model->filters_w[j]);
    writeLog("detect_fast model.filters.i=");
    writeLog(model->filters_i);
    
    writeLog("detect_fast model.defs.w=");
    writeLog(model->defs_w);
    writeLog("detect_fast model.defs.i=");
    writeLog(model->defs_i);
    writeLog("detect_fast model.defs.anchor=");
    writeLog(model->defs_anchor);
    
    for(int c = 0; c < model->componentsNbr; c++)
    {
        snprintf(buffer, sizeof(buffer), "detect_fast model.components{%d}.biasid=", c+1);
        writeLog(buffer);
        writeLog(model->components_biasid[c]);
        
        snprintf(buffer, sizeof(buffer), "detect_fast model.components{%d}.filterid=", c+1);
        writeLog(buffer);
        writeLog(model->components_filterid[c]);
        
        snprintf(buffer, sizeof(buffer), "detect_fast model.components{%d}.defid=", c+1);
        writeLog(buffer);
        writeLog(model->components_defid[c]);
        
        snprintf(buffer, sizeof(buffer), "detect_fast model.components{%d}.parent=", c+1);
        writeLog(buffer);
        writeLog(model->components_parent);
    }
    
    writeLog("detect_fast model.pa=");
    writeLog(model->pa);
    writeLog("detect_fast model.maxsize=");
    writeLog(model->maxsize);
    writeLog("detect_fast model.interval=");
    writeLog(model->interval);
    writeLog("detect_fast model.sbin=");
    writeLog(model->sbin);
    writeLog("detect_fast model.len=");
    writeLog(model->len);
    writeLog("detect_fast model.thresh=");
    writeLog(model->thresh);
    writeLog("detect_fast model.obj=");
    writeLog(model->obj);
#endif
    
    bool isPyraManagedOutside = true;
    // Compute the feature pyramid and prepare filter
    if (pyra == NULL)
    {
        pyra = featpyramid_C(img, model);
        isPyraManagedOutside = false;
    }
    
#if LOGGING_ON
    // debug
    writeLog("detect_fast pyra.feat=");
    for( int i = 0; i < pyra->max_scale; i++)
        writeLog(pyra->feat[i]);
    writeLog("detect_fast pyra.scale=");
    writeLog(pyra->scale);
    snprintf(buffer, sizeof(buffer), "detect_fast pyra.interval=%d .imy=%d .imx=%d .pady=%d .padx=%d", pyra->interval, pyra->imy, pyra->imx, pyra->pady, pyra->padx);
    writeLog(buffer);
#endif
    
    // Cache various statistics derived from model
    Component components[model->componentsNbr];
    myArray<FPTYPE> **filters;
    modelcomponents_C(model, pyra, /*outputs*/ components, &filters);
    
#if LOGGING_ON
    //debug
    for(int c = 1; c <= model->componentsNbr; c++)
    {
        snprintf(buffer, sizeof(buffer), "detect_fast components{%d}=", c);
        writeLog(buffer);
        
        Component *p = &(components[c-1]);
        
        writeLog("detect_fast components.biasid=");
        writeLog(p->biasid);
        writeLog("detect_fast components.filterid=");
        writeLog(p->filterid);
        writeLog("detect_fast components.defid=");
        writeLog(p->defid);
        writeLog("detect_fast components.parent=");
        writeLog(p->parent);
        
        writeLog("detect_fast components.b=");
        writeLog(p->b);
        writeLog("detect_fast components.biasI=");
        writeLog(p->biasI);
        writeLog("detect_fast components.sizex=");
        writeLog(p->sizex);
        writeLog("detect_fast components.sizey=");
        writeLog(p->sizey);
        writeLog("detect_fast components.w=");
        writeLog(p->w);
        writeLog("detect_fast components.defI=");
        writeLog(p->defI);
        writeLog("detect_fast components.scale=");
        writeLog(p->scale);
        writeLog("detect_fast components.startx=");
        writeLog(p->startx);
        writeLog("detect_fast components.starty=");
        writeLog(p->starty);
        writeLog("detect_fast components.step=");
        writeLog(p->step);
    }
#endif
    
    myArray<FPTYPE> **pyra_feat = pyra->feat;
    int levels = pyra->max_scale;
    int componentsNb = model->componentsNbr;
    int numparts = 0; // number of parts ; supposed to be constant over all components
    
    std::vector<FPTYPE> *boxes = new std::vector<FPTYPE>; // detections boxes
    if (!partTypes)
        partTypes = new std::vector<int>();
    partTypes->clear();
    
    int detectionNbr = 0; // number of detections
    FPTYPE interval = model->interval;
    
#ifdef USE_CUDA
    initCuda();
#endif
    
    // Iterate over scales and components
    int minLevel = (MINLEVEL > 0) ? MINLEVEL : 1;
    int maxLevel = (MAXLEVEL > 0) ? MAXLEVEL : levels;
    for( int rlevel = minLevel; rlevel <= maxLevel; rlevel++)
    {
        myArray<FPTYPE> **resp = NULL;
        
        
        for( int c = 1; c <= componentsNb; c++)
        {
            Component *parts = &(components[c-1]);
            numparts = model->partsNbr;
            myArray<FPTYPE> **parts_score[numparts];
            // parts_score[k-1][fi-1] is a myArray* that
            // points to a 2D array
            int parts_level[numparts];
            
            // Local scores
            for( int k = 1; k <= numparts; k++)
            {
                int fNb = parts->filterid->getCols();
                myArray<int> *f = parts->filterid;
                
                FPTYPE scale = parts->scale->get(k-1);
                int level = rlevel - scale*interval;
                if( ! resp )
                {
                    // computed one time per rlevel
                    const myArray<FPTYPE> *pyra_feat_level = pyra_feat[level-1];
                    int filtersNb =  model->filtersSz;
#ifdef USE_CUDA
                    resp = fconv_cuda(pyra_feat_level, filters, 1, filtersNb);
#else
                    resp = fconvMT_C(pyra_feat_level, filters, 1, filtersNb);
                    
                    // take a deep copy of the resp for this level
                    if (copyOfResp != NULL)
                    {
                        copyOfResp[rlevel-1] = new myArray<FPTYPE>*[filtersNb];
                        for (int indFilter = 0; indFilter < filtersNb; indFilter++)
                            copyOfResp[rlevel-1][indFilter] = new myArray<FPTYPE>(resp[indFilter]);
                    } // will be deleted outside
#endif
                }
                
                // allocate memory to store one score per filterid
                parts_score[k-1] = (myArray<FPTYPE>**) calloc(fNb, sizeof(myArray<FPTYPE>*));
                for( int fi = 1; fi <= fNb; fi++)
                    parts_score[k-1][fi-1] = resp[f->get(k-1, fi-1) - 1];
                
#if LOGGING_ON
                // debug
                for( int fi = 1; fi <= fNb; fi++)
                {
                    snprintf(buffer, sizeof(buffer), "rlevel=%d level=%d c=%d k=%d fi=%d", rlevel, level, c, k, fi);
                    writeLog(buffer);
                    writeLog("parts(k).score(:,:,fi)=");
                    writeLog(parts_score[k-1][fi-1]);
                }
#endif
                
                parts_level[k-1] = level;
            }
            
            // Walk from leaves to root of tree, passing message to parent
            myArray<int> *Ix[numparts], *Iy[numparts], *Ik[numparts];
            for( int k = numparts; k >= 2; k--)
            {
                int par = parts->parent->get(k-1); // parent id
                
#if LOGGING_ON
                snprintf(buffer, sizeof(buffer), "detect_fast for_k_#2 k=%d par=%d", k, par);
                writeLog(buffer);
#endif
                
                myArray<FPTYPE> *msg;
                passmsg_C( k, par, parts, parts_score, /*outputs*/ &msg, Ix, Iy, Ik);
                
                // update parent score
                // parts(par).score = parts(par).score + msg;
                int fNb = parts->filterid->getCols();
                for( int fi = 1; fi <= fNb; fi++)
                {
                    myArray<FPTYPE> *parent_score = parts_score[par-1][fi-1]; // 2D array
                    
                    int nbElts = parent_score->getNumberOfElements();
                    
                    FPTYPE *parent_score_data = parent_score->getPtr(); // 2D array
                    FPTYPE *msg_data = msg->getPtr(0, 0, fi-1); // 3D array
                    for( int i = 0; i < nbElts; i++)
                        parent_score_data[i] += msg_data[i];
                }
                
                delete msg;
            }
            
            // Add bias to root score
            // parts(1).score = parts(1).score + parts(1).b;
            int fNb = parts->filterid->getCols();
            FPTYPE root_b = parts->b->get(1-1, 1-1, 1-1); // parts(1).b is a single value
            int Nx = parts_score[1-1][1-1]->getCols();
            int Ny = parts_score[1-1][1-1]->getRows();
            myArray<FPTYPE> rscore(Ny, Nx); // max score array
            Ix[1-1] = NULL; // to smake deallocation easier
            Iy[1-1] = NULL; // to smake deallocation easier
            Ik[1-1] = new myArray<int>(Ny, Nx);
            for( int fi = 1; fi <= fNb; fi++)
            {
                myArray<FPTYPE> *root_score = parts_score[1-1][fi-1]; // 2D array
                int rows = root_score->getRows();
                int cols = root_score->getCols();
                if( Ny != rows  ||  Nx != cols )
                    printf("ERROR: Array size mismatch !!!!\n");
                
                for( int r = 1 ; r <= rows; r++)
                {
                    for( int c = 1; c <= cols; c++)
                    {
                        FPTYPE *_root_score = root_score->getPtr(r-1, c-1);
                        *_root_score += root_b;
                        
                        FPTYPE *_rscore = rscore.getPtr(r-1, c-1);
                        
                        // init or update the max score array
                        if( fi == 1 || (*_root_score > *_rscore) )
                        {
                            // replace max score
                            *_rscore = *_root_score;
                            Ik[1-1]->set(fi, r-1, c-1);
                        }
                    }
                }
            }
            
#if LOGGING_ON
            // debug
            for( int fi = 1; fi <= fNb; fi++)
            {
                snprintf(buffer, sizeof(buffer), "rlevel=%d c=%d parts(1).score(:,:,%d)=", rlevel, c, fi);
                writeLog(buffer);
                writeLog(parts_score[1-1][fi-1]);
            }
            snprintf(buffer, sizeof(buffer), "rlevel=%d c=%d rscore=", rlevel, c);
            writeLog(buffer);
            writeLog(&rscore);
            snprintf(buffer, sizeof(buffer), "rlevel=%d c=%d Ik=", rlevel, c);
            writeLog(buffer);
            writeLog(Ik[1-1]);
#endif
            
            // Walk back down tree following pointers
            FPTYPE thresh = std::min(model->thresh, (FPTYPE) -1.0);
            backtrack_C_pt(thresh, &rscore, Ix, Iy, Ik, parts, pyra, parts_level, c, /*output*/ boxes, partTypes, &detectionNbr);
            
            // free memory
            for( int k = 1; k <= numparts; k++)
            {
                free(parts_score[k-1]);
                delete Ix[k-1];
                delete Iy[k-1];
                delete Ik[k-1];
            }
        }
        
        // free resp here
        int len = model->filtersSz - 1 + 1;
        for(int i = 0; i < len; i++)
            delete resp[i];
        free(resp);
        
        
    }
    
    delete_components(components, model->componentsNbr);
    if (!isPyraManagedOutside)
        delete_featpyramid(&pyra);
    
#ifdef USE_CUDA
    releaseCuda();
#endif
    
#if LOGGING_ON
    // display detection boxes
    writeLog("detect_fast boxes=");
    writeLog(boxes, detectionNbr, 4*numparts+2);
#endif
    
    return boxes;
}
//--------------------------------------------------------------------
// Calculate pose estimation with epipolar constraints + part type compatibility
std::vector<FPTYPE>* detect_fast_withEpiConstraint_and_PTC(const myArray<unsigned char> *img, const Model *model,
#if defined(WFV) || defined(AVS)
                                                           const std::vector<FPTYPE> *supportWeights,
#endif
                                                           const std::vector<FPTYPE> *supportBoxes,
                                                           const EpipolarGeometry *epiGeometry,
                                                           const PartTypeCompat *partTypeCompat,
                                                           myArray<FPTYPE>*** filterResponse,
                                                           /*INPUT*/ std::vector<int>* &supportPartTypes,
                                                           /*OUTPUT*/ std::vector<int>* &targetPartTypes,
                                                           /*INPUT*/Feature_pyramid *pyra = NULL)
{
    bool isPyraManagedOutside = true;
    // Compute the feature pyramid and prepare filter
    if (pyra == NULL)
    {
        pyra = featpyramid_C(img, model);
        isPyraManagedOutside = false;
    }
    
    // Cache various statistics derived from model
    Component components[model->componentsNbr];
    myArray<FPTYPE> **filters;
    modelcomponents_C(model, pyra, /*outputs*/ components, &filters);
    
    int levels = pyra->max_scale;
    int componentsNb = model->componentsNbr;
    int numparts = 0; // number of parts ; supposed to be constant over all components
    
    if (targetPartTypes) //supposed to be null, will be allocated here
        delete targetPartTypes;
    
    targetPartTypes = new std::vector<int>();
    targetPartTypes->reserve(1000000);
    std::vector<FPTYPE> *boxes = new std::vector<FPTYPE>(); // detections boxes
    boxes->reserve(1500000);
    int detectionNbr = 0; // number of detections
    FPTYPE interval = model->interval;
    
    // fetch part type compatibility scores: (it's same for all components and levels)
    const myArray<FPTYPE>* ptCompatScores = partTypeCompat->getAllScores();
    
    // Iterate over scales and components
    int minLevel = (MINLEVEL > 0) ? MINLEVEL : 1;
    int maxLevel = (MAXLEVEL > 0) ? MAXLEVEL : levels;
    for( int rlevel = minLevel; rlevel <= maxLevel; rlevel++)
    {
        // Copy data in filterResponse into resp
        // because parts_score point to resp, and modify it during message passing,
        // and this is resp that we'll use in other iterations
        int len = model->filtersSz - 1 + 1;
        
        myArray<FPTYPE> **resp = (myArray<FPTYPE>**) calloc(len, sizeof(myArray<FPTYPE>*)); //allocate memory for pointers
        
        for (int i = 0; i < len; i++)
        {
            resp[i] = new myArray<FPTYPE>(filterResponse[rlevel-1][i]); //create a deep copy of the filter response for rlevel and i
        }
        
        for( int c = 1; c <= componentsNb; c++)
        {
            Component *parts = &(components[c-1]);
            numparts = model->partsNbr;
            int fNb = parts->filterid->getCols();
            
            myArray<FPTYPE> **parts_score[numparts];
            // parts_score[k-1][fi-1] is a myArray* that
            // points to a 2D array
            int parts_level[numparts];
            
            // Local scores
            for( int k = 1; k <= numparts; k++)
            {
                
                myArray<int> *f = parts->filterid;
                FPTYPE scale = parts->scale->get(k-1);
                int level = rlevel - scale*interval;
                //no need to recalculate response, we already have it.
                
                // allocate memory to store one score per filterid
                parts_score[k-1] = (myArray<FPTYPE>**) calloc(fNb, sizeof(myArray<FPTYPE>*));
                for( int fi = 1; fi <= fNb; fi++)
                    parts_score[k-1][fi-1] = resp[f->get(k-1, fi-1) - 1];
                
                parts_level[k-1] = level;
            }
            
            
            // ////////////////////////////////////////////////////////////////////////////////////
            //      EPIPOLAR CONSTRAINTS START HERE
            // ////////////////////////////////////////////////////////////////////////////////////
            // FIXME: merge this with the for loop above
            for( int k = 1; k <= numparts; k++)
            {
#if defined(WFV) || defined(AVS)
                if ((*supportWeights)[k-1] == 0) // nothing to do for this part
                    continue;
#endif
                
                int partTypeOnSupportView = supportPartTypes->at(k-1);
                
                // get position from supportBoxes (the detection in the other view)
                double x1 = (*supportBoxes)[4*(k-1) + 0];
                double y1 = (*supportBoxes)[4*(k-1) + 1];
                double x2 = (*supportBoxes)[4*(k-1) + 2];
                double y2 = (*supportBoxes)[4*(k-1) + 3];
                
                // calculate the box center
                double centerX = 0, centerY = 0;
                boxCenter(x1, y1, x2, y2, &centerX, &centerY);
                
                //check box center is in the image
                if ( centerX < 0.0 || centerX >= (double)img->getCols() ||
                    centerY < 0.0 || centerY >= (double)img->getRows() )
                    continue; //not in the image
                
                // fetch corresponding epipolar line according the box center
                PointPair epiBorderPoints = epiGeometry->getEpiBorderPoints((int)round(centerX), (int)round(centerY));
                //calculate the heatmap based on the epipolar lines
                cv::Mat epiHeatMapMat = EpipolarHeatmap::calculateEpiHeatmap(epiBorderPoints, //intersection of the epipolar line with the original image size
                                                                             epiGeometry->getOrigRowSize(), epiGeometry->getOrigColSize(), // original size where the epipolar lines are calculated
                                                                             parts_score[k-1][0]->getRows(), parts_score[k-1][0]->getCols()); //desired size of the heatmap
                
                // multiply the heatmap with the multiplier
#if defined(WFV) || defined(AVS)
                epiHeatMapMat = epiHeatMapMat * heatMultiplier * (*supportWeights)[k-1];
#else
                epiHeatMapMat = epiHeatMapMat * heatMultiplier;
#endif
                
                // add it to parts_score for that part
                for( int fi = 1; fi <= fNb; fi++)
                {
                    // fetch the compatibility score based on part type on the other view, filter type at hand (f1-1) and part at hand (k-1)
#if defined(WFV) || defined(AVS)
                    FPTYPE additionalPartTypeScore = partTypeMultiplier * (*supportWeights)[k-1] * ptCompatScores->get(partTypeOnSupportView-1, fi-1, k-1);
#else
                    FPTYPE additionalPartTypeScore = partTypeMultiplier * ptCompatScores->get(partTypeOnSupportView-1, fi-1, k-1);
#endif
                    //no need to check for sizes (epiheatmap is initialized with the correct values)
                    cv::MatConstIterator_<float> it, end = epiHeatMapMat.end<float>();
                    for (it = epiHeatMapMat.begin<float>(); it != end; ++it)
                    {
                        cv::Point2i pos = it.pos();
                        // new value is original score + score from epipolar heatmap + part type compatibility score
                        FPTYPE newVal = parts_score[k-1][fi-1]->get(pos.y, pos.x) + *it + additionalPartTypeScore;
                        parts_score[k-1][fi-1]->set(newVal, pos.y, pos.x);
                    }
                }
            }
            
            
            // ////////////////////////////////////////////////////////////////////////////////////
            //      MESSAGE PASSING STARTS HERE
            // ////////////////////////////////////////////////////////////////////////////////////
            
            // Walk from leaves to root of tree, passing message to parent
            myArray<int> *Ix[numparts], *Iy[numparts], *Ik[numparts];
            for( int k = numparts; k >= 2; k--)
            {
                int par = parts->parent->get(k-1); // parent id
                
                myArray<FPTYPE> *msg;
                passmsg_C( k, par, parts, parts_score, /*outputs*/ &msg, Ix, Iy, Ik);
                
                // update parent score
                // parts(par).score = parts(par).score + msg;
                for( int fi = 1; fi <= fNb; fi++)
                {
                    myArray<FPTYPE> *parent_score = parts_score[par-1][fi-1]; // 2D array
                    
                    int nbElts = parent_score->getNumberOfElements();
                    
                    FPTYPE *parent_score_data = parent_score->getPtr(); // 2D array
                    FPTYPE *msg_data = msg->getPtr(0, 0, fi-1); // 3D array
                    for( int i = 0; i < nbElts; i++)
                        parent_score_data[i] += msg_data[i];
                }
                
                delete msg;
            }
            
            // Add bias to root score
            // parts(1).score = parts(1).score + parts(1).b;
            FPTYPE root_b = parts->b->get(1-1, 1-1, 1-1); // parts(1).b is a single value
            int Nx = parts_score[1-1][1-1]->getCols();
            int Ny = parts_score[1-1][1-1]->getRows();
            myArray<FPTYPE> rscore(Ny, Nx); // max score array
            Ix[1-1] = NULL; // to smake deallocation easier
            Iy[1-1] = NULL; // to smake deallocation easier
            Ik[1-1] = new myArray<int>(Ny, Nx);
            for( int fi = 1; fi <= fNb; fi++)
            {
                myArray<FPTYPE> *root_score = parts_score[1-1][fi-1]; // 2D array
                int rows = root_score->getRows();
                int cols = root_score->getCols();
                if( Ny != rows  ||  Nx != cols )
                    printf("ERROR: Array size mismatch !!!!\n");
                
                for( int r = 1 ; r <= rows; r++)
                {
                    for( int c = 1; c <= cols; c++)
                    {
                        FPTYPE *_root_score = root_score->getPtr(r-1, c-1);
                        *_root_score += root_b;
                        
                        FPTYPE *_rscore = rscore.getPtr(r-1, c-1);
                        
                        // init or update the max score array
                        if( fi == 1 || (*_root_score > *_rscore) )
                        {
                            // replace max score
                            *_rscore = *_root_score;
                            Ik[1-1]->set(fi, r-1, c-1);
                        }
                    }
                }
            }
            
            // Walk back down tree following pointers
            FPTYPE thresh = std::min(model->thresh, (FPTYPE) -1.0);
#if defined(WFV) || defined(AVS)
            thresh += heatMultiplier; // FIXME: what should we do here?
#else
            thresh += heatMultiplier;
#endif
            backtrack_C_pt(thresh, &rscore, Ix, Iy, Ik, parts, pyra, parts_level, c, /*output*/ boxes, targetPartTypes, &detectionNbr);
            
            //            backtrack_C(&rscore, Ix, Iy, Ik, parts, pyra, parts_level, c, /*output*/ boxes); //--> no threshold version
            // this version yielded EXTREMELY HIGH error when tested on a few example
            
            // free memory
            for( int k = 1; k <= numparts; k++)
            {
                //free(parts_score[k-1]);
                delete Ix[k-1];
                delete Iy[k-1];
                delete Ik[k-1];
            }
        }
        
        // free resp here
        for(int i = 0; i < len; i++)
            delete resp[i];
        free(resp);
        
        //        std::cout << "\tCalculation completed for level: " << rlevel << std::endl;
    }
    
    delete_components(components, model->componentsNbr);
    if (!isPyraManagedOutside)
        delete_featpyramid(&pyra);
    
    return boxes;
}

//--------------------------------------------------------------------

// Function that manage the iterations for the pose detection with epipolar constraints AND part type compatibility
std::pair<const std::vector<FPTYPE>*, const std::vector<FPTYPE>* > detect_fast_mvptc(const myArray<unsigned char> *imgA,
                                                                                  const myArray<unsigned char> *imgB, const Model *model,
                                                                                  const EpipolarGeometry* epiGeometryA2B, const EpipolarGeometry* epiGeometryB2A,
                                                                                  const PartTypeCompat* ptCompatA2B, const PartTypeCompat* ptCompatB2A,
                                                                                  int maxIterations = 4, FPTYPE requiredEpsilon = 0.1)
{
    std::cout << "Running detect_fast_mvptc with maximum iterations: " << maxIterations << " and required epsilon: " << requiredEpsilon << std::endl;
    
    char buffer[400]; //for filenames and stuff
    
    std::pair<const std::vector<FPTYPE>*, const std::vector<FPTYPE>* > boxPair(NULL, NULL);
    
    int nbOfParts = model->partsNbr;
    
    //INITIAL CONDITIONS: best of independent estimations on solo views
    
    
    // FIXME: this calculation is only to get "levels". (it's quite fast, though)
    Feature_pyramid *pyraA = featpyramid_C(imgA, model);
    Feature_pyramid *pyraB = featpyramid_C(imgB, model);
    int levels = pyraA->max_scale;
    
    // filter response backups
    myArray<FPTYPE> **filterResponseA[levels];
    myArray<FPTYPE> **filterResponseB[levels];
    
    // part types
    std::vector<int>* partTypesA = NULL; //allocated in detect_fast_C_pt
    std::vector<int>* partTypesB = NULL; //allocated in detect_fast_C_pt
    std::vector<int>* partTypesA_nms = NULL; //allocated in nms() or not_nms_pick_best()
    std::vector<int>* partTypesB_nms = NULL; //allocated in nms() or not_nms_pick_best()
    
    
    // Run solo detect_fast for first image
    std::vector<FPTYPE> *boxesA = detect_fast_C_pt(imgA, model, partTypesA, filterResponseA, pyraA);
    if (boxesA->empty())
        std::cerr << "WARNING\tNo detection for image A: detect_fast_C(imgA, model)\n";
    std::cout << "Detect fast for image A is completed with " << boxesA->size() << " instance of detections." << std::endl;       
             
#ifdef USE_NMS
    std::vector<FPTYPE> *boxesA_nms = nms(boxesA, .1, nbOfParts, (*partTypesA), partTypesA_nms);
#else
    std::vector<FPTYPE> *boxesA_nms = not_nms_pick_best(boxesA, nbOfParts, (*partTypesA), partTypesA_nms); // we don't need NMS when there are only one subject
#endif   //USE_NMS


#ifdef WFV // ESTIMATE ERRORS FOR SINGLEVIEW YR
    
#ifdef HUMANEVA
    std::vector<FPTYPE> estimatedYRErrors_A = YRerrorEstimator::estimatePartBasedError_mimo(boxesA_nms, viewA);
#elif defined(UMPM)
    std::size_t found = outputFolder.find_last_of("/");
    std::string action = outputFolder.substr(found+1); // "p1_chair_2_f-wrt-s"
    action.erase(action.end()-8, action.end()); // "p1_chair_2"
    std::vector<FPTYPE> estimatedYRErrors_A = YRerrorEstimator::estimatePartBasedError_mimo(boxesA_nms, viewA, action);
#endif
    
#elif defined(AVS)
    EstimateYrErrorCNN estCNN(predictionFile.c_str());
#ifdef HUMANEVA
    std::size_t found = outputFolder.find_last_of("/");
    std::string subject = outputFolder.substr(found+1); // "S1_Box_1_C1-wrt-C2"
    subject.erase(subject.begin()+2, subject.end()); // "S1"
    std::string action = outputFolder.substr(found+1); // "S1_Box_1_C1-wrt-C2"
    action.erase(action.begin(), action.begin()+3); // "Box_1_C1-wrt-C2"
    action.erase(action.end()-12, action.end()); // "Box"
    std::vector<FPTYPE> estimatedYRErrors_A = estCNN.getErrors(subject, action, viewA, frame);
#elif defined(UMPM)
    std::size_t found = outputFolder.find_last_of("/");
    std::string action = outputFolder.substr(found+1); // "p1_chair_2_f-wrt-s"
    action.erase(action.end()-8, action.end()); // "p1_chair_2"
    std::vector<FPTYPE> estimatedYRErrors_A = estCNN.getErrors(action, viewA, frame);
#endif
    if (estimatedYRErrors_A.empty())
    {
        std::cerr << "ERROR\tError estimation returned zero results for imgA (which means no GT for this image)\n";
        return boxPair;
    }
#endif

    
#ifdef SAVE_IMAGES
    std::sprintf(buffer, "%s/%s_A_Iteration_0.png", outputFolder.c_str(), imgFileA.c_str());
    DisplayTools::saveDetection(*imgA, *boxesA_nms, nbOfParts, buffer);
//    //std::sprintf(buffer, "%s/%s_A_Iteration_0", outputFolder.c_str(), imgFileA.c_str());
//    //DisplayTools::saveDetectionPartBased(*imgA, *boxesA_nms, nbOfParts, buffer);
    
//    std::sprintf(buffer, "%s/%s_B_EpiLinesFromBorderPoints_Iteration_0.png", outputFolder.c_str(), imgFileB.c_str());
//    std::vector<PointPair> allEpiBorderPoints;
//    for (int p=0; p<26; p++)
//    {
//        double centerX, centerY;
//        boxCenter(boxesA_nms->at(4*p + 0), boxesA_nms->at(4*p + 1), boxesA_nms->at(4*p + 2), boxesA_nms->at(4*p + 3),
//                  &centerX, &centerY);
//        allEpiBorderPoints.push_back(epiGeometryA2B->getEpiBorderPoints((int)round(centerX), (int)round(centerY)));
//    }
//    DisplayTools::saveEpipolarLines(*imgB, allEpiBorderPoints, buffer);
//    //DisplayTools::saveEpipolarLinesPartBased(*imgB, allEpiBorderPoints, buffer);
//    allEpiBorderPoints.clear();
#endif
    
    // Run solo detect_fast for second image
    std::vector<FPTYPE> *boxesB = detect_fast_C_pt(imgB, model, partTypesB, filterResponseB, pyraB);
    if (boxesB->empty())
        std::cerr << "WARNING\tNo detection for image B: detect_fast_C(imgB, model)\n";
    std::cout << "Detect fast for image B is completed with " << boxesB->size() << " instance of detections." << std::endl;
    
    
#ifdef USE_NMS
    std::vector<FPTYPE> *boxesB_nms = nms(boxesB, .1, nbOfParts, (*partTypesB), partTypesB_nms);
#else
    std::vector<FPTYPE> *boxesB_nms = not_nms_pick_best(boxesB, nbOfParts, (*partTypesB), partTypesB_nms); //we don't need to use NMS when there are only one subject
#endif

#ifdef WFV // ESTIMATE ERRORS FOR SINGLEVIEW YR

#ifdef HUMANEVA
    std::vector<FPTYPE> estimatedYRErrors_B = YRerrorEstimator::estimatePartBasedError_mimo(boxesB_nms, viewB);
#elif defined(UMPM)
    std::size_t found = outputFolder.find_last_of("/");
    std::string action = outputFolder.substr(found+1); // "p1_chair_2_f-wrt-s"
    action.erase(action.end()-8, action.end()); // "p1_chair_2"
    std::vector<FPTYPE> estimatedYRErrors_B = YRerrorEstimator::estimatePartBasedError_mimo(boxesB_nms, viewB, action);
#endif

#elif defined(AVS)
#ifdef HUMANEVA
    // we already fetched action and subject for estimatedYRErrors_A
    std::vector<FPTYPE> estimatedYRErrors_B = estCNN.getErrors(subject, action, viewB, frame);
#elif defined(UMPM)
    // we already fetched action for estimatedYRErrors_A
    std::vector<FPTYPE> estimatedYRErrors_B = estCNN.getErrors(action, viewB, frame);
#endif
    if (estimatedYRErrors_B.empty())
    {
        std::cerr << "ERROR\tError estimation returned zero results for imgB (which means no GT for this image)\n";
        return boxPair;
    }
#endif




#if defined(WFV) || defined(AVS)
    /////// VIEWPOINT WEIGHT
    
    std::vector<FPTYPE> viewA_weights, viewB_weights;
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Compare part based errors and fill these weights
    for (int k=0; k<nbOfParts; k++)
    {
        std::map<int,FPTYPE>::iterator it = singleViewMedianErrors.find(k); //look for the k'th part in limb list
        FPTYPE singleViewMedianError = it->second;
#ifdef WFV
        it = expectedInaccuracyForSingleViewErrEst.find(k); //look for the k'th part in limb list
        FPTYPE expInacForSviewErrEst = it->second;
#elif defined(AVS)
        it = expectedInaccuracyForSingleViewErrEst.find(k); //look for the k'th part in limb list
        FPTYPE expInacForSviewErrEst = it->second;
        // FPTYPE expInacForSviewErrEst = 0.0;
#endif
        (estimatedYRErrors_A.at(k) > singleViewMedianError+expInacForSviewErrEst) ? viewA_weights.push_back(0.0) : viewA_weights.push_back(1.0);
        (estimatedYRErrors_B.at(k) > singleViewMedianError+expInacForSviewErrEst) ? viewB_weights.push_back(0.0) : viewB_weights.push_back(1.0);

        // additional mechanism to get more improvements:
       if (viewA_weights.at(k) == 0.0 && viewB_weights.at(0.0)) // if both weights are "killed"
       {
           FPTYPE margin = singleViewMedianError;
           // if A is better by a significant margin
           if ((estimatedYRErrors_B.at(k) - estimatedYRErrors_A.at(k)) > margin)
           {
               viewA_weights.at(k) = 1.0; // treat A as a "good part"
           }
           // else if B is better by a significant margin
           else if ((estimatedYRErrors_A.at(k) - estimatedYRErrors_B.at(k)) > margin)
           {
               viewB_weights.at(k) = 1.0; // treat B as a "good part"
           }
       }
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
#endif // defined(WFV) || defined(AVS)
    
#ifdef SAVE_IMAGES
    std::sprintf(buffer, "%s/%s_B_Iteration_0.png", outputFolder.c_str(), imgFileB.c_str());
    DisplayTools::saveDetection(*imgB, *boxesB_nms, nbOfParts, buffer);
//    //std::sprintf(buffer, "%s/%s_B_Iteration_0", outputFolder.c_str(), imgFileB.c_str());
//    //DisplayTools::saveDetectionPartBased(*imgB, *boxesB_nms, nbOfParts, buffer);
    
//    std::sprintf(buffer, "%s/%s_A_EpiLinesFromBorderPoints_Iteration_0.png", outputFolder.c_str(), imgFileA.c_str());
//    for (int p=0; p<26; p++)
//    {
//        double centerX, centerY;
//        boxCenter(boxesB_nms->at(4*p + 0), boxesB_nms->at(4*p + 1), boxesB_nms->at(4*p + 2), boxesB_nms->at(4*p + 3),
//                  &centerX, &centerY);
//        allEpiBorderPoints.push_back(epiGeometryB2A->getEpiBorderPoints((int)round(centerX), (int)round(centerY)));
//    }
//    DisplayTools::saveEpipolarLines(*imgA, allEpiBorderPoints, buffer);
//    //DisplayTools::saveEpipolarLinesPartBased(*imgA, allEpiBorderPoints, buffer);
//    allEpiBorderPoints.clear();
#endif
    
#ifdef SAVE_TEXT
    //save boxes
    std::sprintf(buffer, "%s/%s_Solo_A_Pose.txt", outputFolder.c_str(), imgFileA.c_str());
    saveBoxCenters(boxesA_nms, model->partsNbr, buffer);
    std::sprintf(buffer, "%s/%s_Solo_B_Pose.txt", outputFolder.c_str(), imgFileB.c_str());
    saveBoxCenters(boxesB_nms, model->partsNbr, buffer);
#endif
    
    //clean up memory
    delete boxesA; boxesA = NULL;
    delete boxesB; boxesB = NULL;
    delete partTypesA; partTypesA = NULL;
    delete partTypesB; partTypesB = NULL;
    
    // check for detection results
    FPTYPE scoreA = -DBL_MAX, scoreB = -DBL_MAX;
    if ( boxesA_nms->empty() || boxesA_nms->size() % (4*nbOfParts+2) != 0 || partTypesA_nms->empty() || partTypesA_nms->size() % nbOfParts != 0 )
    {
        std::cerr << "Either NMS yielded empty detections for A\n"
                  << "\tor number of obtained boxes are inconsistent with the number of parts.\n"
                  << "\tor there is something wrong with the number of tracked part types" << std::endl;
        delete boxesA_nms; boxesA_nms = NULL;
        delete partTypesA_nms; partTypesA_nms = NULL;
    }
    else
    {
        scoreA = (*boxesA_nms)[4*nbOfParts+1]; // this is the root score for detection A
        std::cout << ">> Score for image A: " << scoreA << std::endl;
    }
    
    if (boxesB_nms->empty() || boxesB_nms->size() % (4*nbOfParts+2) != 0  || partTypesB_nms->empty() || partTypesB_nms->size() % nbOfParts != 0 )
    {
        std::cerr << "Either NMS yielded empty detections for B\n"
        << "\tor number of obtained boxes are inconsistent with the number of parts.\n"
        << "\tor there is something wrong with the number of tracked part types" << std::endl;
        delete boxesB_nms; boxesB_nms = NULL;
        delete partTypesB_nms; partTypesB_nms = NULL;
    }
    else
    {
        scoreB = (*boxesB_nms)[4*nbOfParts+1]; // this is the root score for detection B
        std::cout << ">> Score for image B: " << scoreB << std::endl;
    }
    
    // select the "better looking" result (based on their score) as initialization
    const myArray<unsigned char> *targetImg = NULL, *supportImg = NULL;
    const std::vector<FPTYPE> *targetBoxes = NULL, *supportBoxes = NULL;
    std::vector<int> *targetPartTypes = NULL, *supportPartTypes = NULL;
    const EpipolarGeometry *supportToTargetEpiLines = NULL, //epipolar lines on targetImg, based on boxes at supportImg
                            *targetToSupportEpiLines = NULL;//epipolar lines on supportImg, based on boxes at targetImg
    myArray<FPTYPE> ***targetFilterResponse = NULL, ***supportFilterResponse = NULL;
    const PartTypeCompat *supportToTargetPTScores = NULL,
                         *targetToSupportPTScores = NULL;
    Feature_pyramid *targetPyra = NULL, *supportPyra = NULL;
#if defined(WFV) || defined(AVS)
    std::vector<FPTYPE> *targetWeights = NULL, *supportWeights = NULL;
#endif


#if defined(WFV) || defined(AVS)
    // sum of errors
    FPTYPE sumOfErrorsA = 0.0, sumOfErrorsB = 0.0;
    for(std::vector<FPTYPE>::size_type i=0; i<estimatedYRErrors_B.size(); i++)
    {
        sumOfErrorsA += estimatedYRErrors_A[i];
        sumOfErrorsB += estimatedYRErrors_B[i];
    }
    if (sumOfErrorsB >= sumOfErrorsA) // we estimate higher part based error on view B ==> we trust A better, start with it
#else // not WFV nor AVS
    if (scoreA >= scoreB)
#endif
    {
        //just assigning the pointers
        targetImg = imgB;  //keep these pointers to identify later
        targetBoxes = boxesB_nms; boxesB_nms = NULL;
        targetPartTypes = partTypesB_nms; partTypesB_nms = NULL;
        supportToTargetEpiLines = epiGeometryA2B; epiGeometryA2B = NULL;
        supportToTargetPTScores = ptCompatA2B; ptCompatA2B = NULL;
        targetFilterResponse = filterResponseB;
        targetPyra = pyraB;
#if defined(WFV) || defined(AVS)
        targetWeights = &viewB_weights;
#endif
        
        supportImg = imgA;  //keep these pointers to identify later
        supportBoxes = boxesA_nms; boxesA_nms = NULL;
        supportPartTypes = partTypesA_nms; partTypesA_nms = NULL;        
        targetToSupportEpiLines = epiGeometryB2A; epiGeometryB2A = NULL;
        targetToSupportPTScores = ptCompatB2A; ptCompatB2A = NULL;
        supportFilterResponse = filterResponseA;
        supportPyra = pyraA;
#if defined(WFV) || defined(AVS)
        supportWeights = &viewA_weights;
#endif
    }
#if defined(WFV) || defined(AVS)
    else //if( sumOfErrorsB < sumOfErrorsA )
#else
    else //if( scoreA < scoreB)
#endif
    {
        //just assigning the pointers
        targetImg = imgA;  //keep these pointers to identify later
        targetBoxes = boxesA_nms; boxesA_nms = NULL;
        targetPartTypes = partTypesA_nms; partTypesA_nms = NULL;
        supportToTargetEpiLines = epiGeometryB2A; epiGeometryB2A = NULL;
        supportToTargetPTScores = ptCompatB2A; ptCompatB2A = NULL;
        targetFilterResponse = filterResponseA;
        targetPyra = pyraA;
#if defined(WFV) || defined(AVS)
        targetWeights = &viewA_weights;
#endif
        supportImg = imgB;  //keep these pointers to identify later
        supportBoxes = boxesB_nms; boxesB_nms = NULL;
        supportPartTypes = partTypesB_nms; partTypesB_nms = NULL;
        targetToSupportEpiLines = epiGeometryA2B; epiGeometryA2B = NULL;
        targetToSupportPTScores = ptCompatA2B; ptCompatA2B = NULL;
        supportFilterResponse = filterResponseB;
        supportPyra = pyraB;
#if defined(WFV) || defined(AVS)
        supportWeights = &viewB_weights;
#endif
    }
    
    // variables for joint detection iterations
    int iteration = 1;
    double actualEpsilon = DBL_MAX;
    double previousEpsilon = DBL_MAX;
    int nbOfBoxes = 4*nbOfParts+2; //last two is the component number and detection score
    int nbOfDetections;
    
    
    // ================================================================================================
    //  START JOINT DETECTION HERE
    // ================================================================================================
    
    std::vector<FPTYPE> epsilons;
    do
    {
#ifdef SAVE_IMAGES
//        if (targetImg == imgA) //we processed imgA
//            std::sprintf(buffer, "%s/%s_B_EpiLinesFromBorderPoints_Iteration_%d.png", outputFolder.c_str(), imgFileB.c_str(), iteration);
//        else
//            std::sprintf(buffer, "%s/%s_A_EpiLinesFromBorderPoints_Iteration_%d.png", outputFolder.c_str(), imgFileA.c_str(), iteration);

//        {
//            std::vector<PointPair> allEpiBorderPoints;
//            for (int p=0; p<26; p++)
//            {
//                double centerX, centerY;
//                boxCenter(supportBoxes->at(4*p + 0), supportBoxes->at(4*p + 1), supportBoxes->at(4*p + 2), supportBoxes->at(4*p + 3),
//                          &centerX, &centerY);
//                allEpiBorderPoints.push_back(supportToTargetEpiLines->getEpiBorderPoints((int)round(centerX), (int)round(centerY)));

//            }
//            DisplayTools::saveEpipolarLines(*targetImg, allEpiBorderPoints, buffer);
//            //DisplayTools::saveEpipolarLinesPartBased(*imgB, allEpiBorderPoints, buffer);
//            allEpiBorderPoints.clear();
//        }
#endif

        std::vector<int>* newPartTypes = NULL; // a container to be used before nms (or not_nms_pick_best) - allocated in detect_fast_withEpiConstraint_and_PTC
        std::vector<int>* newPartTypes_nms = NULL; //a container to be used after nms (or not_nms_pick_best) - allocated in nms
        // run it with supporting information
        std::vector<FPTYPE> *newBoxes = detect_fast_withEpiConstraint_and_PTC(targetImg, model,
#if defined(WFV) || defined(AVS)
                                                                              supportWeights,                                                                              
#endif
                                                                              supportBoxes, supportToTargetEpiLines,
                                                                              supportToTargetPTScores, targetFilterResponse, supportPartTypes,
                                                                              newPartTypes, targetPyra);
        //targetEpiLines: epilines that are in targetImg, given a point in supportImg
        
        std::cout << "Joint detection is completed with " << newBoxes->size() << " instance of detections." << std::endl;
#ifdef USE_NMS
        std::vector<FPTYPE> *newBoxes_nms = nms(newBoxes, .1, nbOfParts, (*newPartTypes), newPartTypes_nms);
#else
        std::vector<FPTYPE> *newBoxes_nms = not_nms_pick_best(newBoxes, nbOfParts, (*newPartTypes), newPartTypes_nms); //we don't need to use NMS when there are only one subject
#endif
        
        
        delete newBoxes;    //clean up memory
        newBoxes = NULL;
        delete newPartTypes; //clean up memory
        newPartTypes = NULL;
        
        if (newBoxes_nms->size() % nbOfBoxes != 0 || newPartTypes_nms->size() % nbOfParts != 0)
            throw std::runtime_error("Detection completed, but obtained wrong number of boxes or part types.\n\n");
        
        nbOfDetections = (int)newBoxes_nms->size() / nbOfBoxes;
        
        if (nbOfDetections == 0) // no detection
        {
            delete newBoxes_nms; //this is actually empty, delete it to prevent memleak
            newBoxes_nms = NULL;
            delete newPartTypes_nms;
            newPartTypes_nms = NULL;
            //note: it's likely that we'll end up here every other iteration
            
            std::cout << "WARNING No detection with the epipolar constraint.\n";
        }
        else if (nbOfDetections == 1) //single detection
        {
            //  check for convergence here
            actualEpsilon = calculateEpsilon(*targetBoxes, *newBoxes_nms);
            
            epsilons.push_back(actualEpsilon);

            delete targetBoxes; //these ones are obsolete now
            targetBoxes = NULL;
            targetBoxes = newBoxes_nms; // save a pointer to new detection boxes
            newBoxes_nms = NULL; // these boxes are managed by targetBoxes from now on.
            
            delete targetPartTypes; //same as above...
            targetPartTypes = NULL;
            targetPartTypes = newPartTypes_nms;
            newPartTypes_nms = NULL;
        }
#ifdef USE_NMS //multiple detection is only possible if do NMS.
        else // multiple detections
        {
            // TODO handle (BETTER!) the case with multiple detection
            std::cout << "WARNING Multiple detection obtained. Discarding the one(s) with lower score...\n";
            
            //create a copy for the best instance of the new boxes
            std::vector<FPTYPE> *copyOfNewBoxes = new std::vector<FPTYPE>(4*nbOfParts+2);
            std::vector<int> *copyOfNewPartTypes = new std::vector<int>(nbOfParts);
            
            // find the max scored detection
            FPTYPE maxScore = -DBL_MAX;
            for(int detectionIndex = 0; detectionIndex<nbOfDetections; detectionIndex++)
            {
                int offset = detectionIndex * nbOfBoxes; //to copy boxes
                int offset2 = detectionIndex * nbOfParts; //to copy part types
                FPTYPE score = newBoxes_nms->at(detectionIndex * (4*nbOfParts+2));
                if (score > maxScore)
                {
                    maxScore = score;
                    //and copy it
                    std::vector<FPTYPE>::iterator itCopy, itNew = newBoxes_nms->begin() + offset;
                    for (itCopy = copyOfNewBoxes->begin(); itCopy != copyOfNewBoxes->end(); ++itCopy, ++itNew)
                        *itCopy = *itNew;
                    std::vector<int>::iterator itCopy2, itNew2 = newPartTypes_nms->begin() + offset2;
                    for (itCopy2 = copyOfNewPartTypes->begin(); itCopy2 != copyOfNewPartTypes->end(); ++itCopy2, ++itNew2)
                        *itCopy2 = *itNew;
                }
            }
            // we don't need newBoxes_nms anymore, we already copied it
            delete newBoxes_nms; newBoxes_nms = NULL;
            delete newPartTypes_nms; newPartTypes_nms = NULL;
            
            // check for convergence here
            actualEpsilon = calculateEpsilon(*targetBoxes, *copyOfNewBoxes);
            
            epsilons.push_back(actualEpsilon);
            
            delete targetBoxes; //these ones are obsolete now
            targetBoxes = NULL;
            // what we need is the copied one
            targetBoxes = copyOfNewBoxes;
            copyOfNewBoxes = NULL; //memory allocated by copyOfNewBoxes is controlled by targetBoxes from now on.
            
            delete targetPartTypes; //same as above..
            targetPartTypes = NULL;
            targetPartTypes = copyOfNewPartTypes;
            copyOfNewPartTypes = NULL;
            
        }
#endif // USE_NMS
        
        
#ifdef SAVE_IMAGES
        if (targetImg == imgA) //we processed imgA
            std::sprintf(buffer, "%s/%s_A_Iteration_%d.png", outputFolder.c_str(), imgFileA.c_str(), iteration);
        else //we processed imgB
            std::sprintf(buffer, "%s/%s_B_Iteration_%d.png", outputFolder.c_str(), imgFileB.c_str(), iteration);
        DisplayTools::saveDetection(*targetImg, *targetBoxes, nbOfParts, buffer);
        //DisplayTools::saveDetectionPartBased(*targetImg, *targetBoxes, nbOfParts, buffer);
        //        DisplayTools::displayDetection(*targetImg, *targetBoxes, nbOfParts, 1000);
        
        if (targetImg == imgA) //we processed imgA
            std::sprintf(buffer, "%s/%s_A_Pose_Iteration_%d.txt", outputFolder.c_str(), imgFileA.c_str(), iteration);
        else
            std::sprintf(buffer, "%s/%s_B_Pose_Iteration_%d.txt", outputFolder.c_str(), imgFileB.c_str(), iteration);
        saveBoxCenters(targetBoxes, nbOfParts, buffer);
#endif
        
        
        //swap the support and the target:
        std::swap(targetBoxes, supportBoxes); // targetBoxes are now obsolete? --> NO. it might be used again in (nbOfDetections == 0), and is deleted otherwise.
        std::swap(targetPartTypes, supportPartTypes);
        std::swap(targetImg, supportImg);
        std::swap(supportToTargetEpiLines, targetToSupportEpiLines);
        std::swap(supportToTargetPTScores, targetToSupportPTScores);
        std::swap(targetFilterResponse, supportFilterResponse);
        std::swap(targetPyra, supportPyra);
#if defined(WFV) || defined(AVS)
        std::swap(targetWeights, supportWeights);        
#endif
        
        //debug
        if (epsilons.size() > 2)
            std::cout << "Iteration " << iteration << " completed with new score: " << (*supportBoxes)[4*nbOfParts+1] << std::endl
            << "\tActual eps: " << actualEpsilon
            << "\tPrevious eps: " << epsilons.at(epsilons.size() - 3)
            << "\tRequired eps: " << requiredEpsilon << std::endl;
        else
            std::cout << "Iteration " << iteration << " completed with new score: " << (*supportBoxes)[4*nbOfParts+1] << std::endl
            << "\tActual eps: " << actualEpsilon
            << "\tRequired eps: " << requiredEpsilon << std::endl;
        //debug
        
        if (epsilons.size() > 1)
            previousEpsilon = epsilons.at(epsilons.size() - 2);
        
        iteration++;
    }
    while(iteration-1 < maxIterations &&
          !(actualEpsilon <= requiredEpsilon && previousEpsilon <= requiredEpsilon) ); //additional condition to check "the other" epsilon as well before stopping
    
#ifdef SAVE_TEXT
    //save iteration and epsilon info
    {
        std::sprintf(buffer, "%s/%s_MV_stats.txt", outputFolder.c_str(), imgFileA.c_str());
        std::ofstream txtFile;
        txtFile.open(buffer);
        txtFile << iteration-1 << "\n";
        for (std::vector<FPTYPE>::iterator it = epsilons.begin() ; it != epsilons.end(); ++it) txtFile << *it << "\n";
#if defined(WFV) || defined(AVS)
        txtFile << "\n------- Weight Info -------\n";
        txtFile << "Est. YR errors on A: " << std::endl;
        for (std::vector<FPTYPE>::iterator it = estimatedYRErrors_A.begin() ; it != estimatedYRErrors_A.end(); ++it){ txtFile << *it << "\n";}
        txtFile << "\nEst. YR errors on B: " << std::endl;
        for (std::vector<FPTYPE>::iterator it = estimatedYRErrors_B.begin() ; it != estimatedYRErrors_B.end(); ++it){ txtFile << *it << "\n";}
        txtFile << "\nAssigned Weights for A: " << std::endl;
        for (std::vector<FPTYPE>::iterator it = viewA_weights.begin() ; it != viewA_weights.end(); ++it){ txtFile << *it << "\t";}
        txtFile << "\nAssigned Weights for B: " << std::endl;
        for (std::vector<FPTYPE>::iterator it = viewB_weights.begin() ; it != viewB_weights.end(); ++it){ txtFile << *it << "\t";}
        std::cout << std::endl;
#endif

        txtFile.close();

    }
#endif
    
    //check which one is which:
    if (supportImg == imgA)
    {
        //write boxes for A in the "first" slot, and B in the "second" one
        boxPair.first  = supportBoxes;
        boxPair.second = targetBoxes;
        
    }
    else if (supportImg == imgB)
    {
        //write boxes for A in the "first" slot, and B in the "second" one
        boxPair.first  = targetBoxes;
        boxPair.second = supportBoxes;
    }
    else
    {
        //something's wrong here.
        std::cerr << "Cannot match the boxes with either view A or B.\n";
        //there are still NULL's in the boxPair
    }
    
    // clean up the filter response objects
    int minLevel = (MINLEVEL > 0) ? MINLEVEL : 1;
    int maxLevel = (MAXLEVEL > 0) ? MAXLEVEL : levels;
    for( int rlevel = minLevel; rlevel <= maxLevel; rlevel++)
    {
        int len = model->filtersSz - 1 + 1;
        for(int i = 0; i < len; i++)
        {
            delete filterResponseA[rlevel-1][i];
            delete filterResponseB[rlevel-1][i];
        }
        delete filterResponseA[rlevel-1];
        delete filterResponseB[rlevel-1];
    }
    
    //clean up part types
//    delete partTypesA_nms;
//    delete partTypesB_nms;
    
    //clean up feat pyramids
    delete_featpyramid(&pyraA);
    delete_featpyramid(&pyraB);
    
    // FIXME: we got a segmentation fault here.
    //    delete filterResponseA;
    //    delete filterResponseB;
    
    // TODO: do we need to clean up any more memory?
    //--> filterResponses, boxes, epiGeometry objects are cleared
    //--> boxpair is deleted outside
    
    return boxPair;
}
#endif //MV_PTC

void printVersionInfo()
{
    std::cout << "\n--------------------------------------------------------------\n";
    std::cout << "Human Pose Detection based on Yang & Ramanan's implementation." << std::endl;
    std::cout << "Compiled with following preprocessor commands: " << std::endl;
    
    std::cout << "\tMV :\t\t";
#ifdef MV
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Multiview pose estimation)\n";
    
    std::cout << "\tMV_PTC :\t";
#ifdef MV_PTC
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Multiview pose estimation with part type compatibility )\n";

    std::cout << "\tWFV :\t\t";
#ifdef WFV
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Multiview pose estimation with Adaptive Viewpoint Selection [can be used with MV or MV_PTC] )\n";

    std::cout << "\tAVS :\t";
#ifdef AVS
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Multiview pose estimation with Adaptive Viewpoint Selection on ConvNets [can be used with MV or MV_PTC] )\n";
    

    
    std::cout << "\tUSE_NMS :\t";
#ifdef USE_NMS
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Use of non-maxima suppression. Not required when 1 subject is assumed )\n";

    std::cout << "\tSAVE_TEXT :\t";
#ifdef SAVE_TEXT
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Saves detections as text output)\n";
    
    std::cout << "\tSAVE_IMAGES :\t";
#ifdef SAVE_IMAGES
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Saves detections as image output)\n";
    
    std::cout << "\tHUMANEVA :\t";
#ifdef HUMANEVA
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Code configured for HumanEva dataset)\n";
    
    std::cout << "\tUMPM :\t\t";
#ifdef UMPM
    std::cout << "yes\t";
#else
    std::cout << "no\t";
#endif
    std::cout << "(Code configured for Utrecht - UMPM dataset)\n";
    
    std::cout << "--------------------------------------------------------------\n\n";
}



//--------------------------------------------------------------------
// FIXME There might be some nested top(0) calls, I think we don't suppose to have that.
int main(const int argc, const char **argv)
{
    printVersionInfo();    
    
    const char *imageFileName = NULL;
    const char *modelFileName = NULL;

    
    
//----------------------------------------------------------------------------
//  Read arguments for MV version
//----------------------------------------------------------------------------
#ifdef MV // with distance transform + multiview
    int maxIterations = 12;
    double requiredEpsilon = 0.1;
    outputFolder = "./";
    const char *secondImageFileName = NULL;
    const char *epipolarGeometryFileNameA2B = NULL;
    const char *epipolarGeometryFileNameB2A = NULL;

    if (argc != 10) //let's force the user to input all arguments
    {
        printf("Usage: %s image_fileA image_fileB epipolarGeometryFileBtoA epipolarGeomteryFileBtoA maxIterations requiredEpsilon OutputFolderForSaving modelFileName HeatMapMultiplier\n\n", argv[0]);
        printf("Example: %s ../resources/im0110A.jpg ../resources/im0110B.jpg ../resources/BtoA.csv ../resources/AtoB.csv 12 0.1 ../output ../resources/model.txt 0.07 \n", argv[0]);
        return 1;
    }

    // fetch arguments
    imageFileName = argv[1];
    secondImageFileName = argv[2];
    epipolarGeometryFileNameA2B = argv[3]; // yes. these are inversed on purpose
    epipolarGeometryFileNameB2A = argv[4];
    maxIterations = atoi(argv[5]);
    requiredEpsilon = atof(argv[6]);
    outputFolder = std::string(argv[7]);
    modelFileName = argv[8];
    heatMultiplier = atof(argv[9]);

    //keep track of filenames for debug purposes
    {
        std::string fullPathImgA(imageFileName);
        std::string fullPathImgB(secondImageFileName);

        //split with "/"
        std::vector<std::string> elems;
        split(fullPathImgA, '/', elems);
        imgFileA = elems.back();
#ifdef HUMANEVA
        std::string seq = elems.at(elems.size()-2);
        std::size_t posStart = seq.find('(');
        std::size_t posEnd = seq.find(')');
        viewA = seq.substr(posStart+1, (posEnd-1-posStart));
#elif defined(UMPM)
        std::string seq = elems.at(elems.size()-2);
        viewA = seq.substr(seq.length()-1, seq.length());
#endif
        elems.clear();
        split(fullPathImgB, '/', elems);
        imgFileB = elems.back();
#ifdef HUMANEVA
        seq = elems.at(elems.size()-2);
        posStart = seq.find('(');
        posEnd = seq.find(')');
        viewB = seq.substr(posStart+1, (posEnd-1-posStart));
#elif defined(UMPM)
        seq = elems.at(elems.size()-2);
        viewB = seq.substr(seq.length()-1, seq.length());
#endif

    }
    //create folder for this execution
    {
        char directoryName[200];
        std::sprintf(directoryName, "%s", outputFolder.c_str());

        std::string directoryStr(directoryName);

        struct stat st = {0};

        if (stat(directoryStr.c_str(), &st) == -1) // if dir does not exist
        {
            int status = _mkdir(directoryStr.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //with read/write/search permissions for owner and group, and with read/search permissions for others.
            if (status < 0)
            {
                std::cerr << "ERROR Could not create directory: " << directoryName << std::endl;
                return -1;
            }
        }
    }
    
    
//----------------------------------------------------------------------------
//  Read arguments for MV_PTC version
//----------------------------------------------------------------------------
#elif defined(MV_PTC)
    int maxIterations = 4;
    double requiredEpsilon = 0.1;
    outputFolder = "./";
    const char *secondImageFileName = NULL;
    const char *epipolarGeometryFileNameA2B = NULL;
    const char *epipolarGeometryFileNameB2A = NULL;
    const char *partTypeScoreFileNameA2B = NULL;
    const char *partTypeScoreFileNameB2A = NULL;
    
    if (argc != 13) //let's force the user to input all arguments
    {
        printf("Usage: %s image_fileA image_fileB epipolarGeometryFileBtoA epipolarGeomteryFileBtoA partTypeScoreFileBtoA partTypeScoreAtoB maxIterations requiredEpsilon OutputFolderForSaving modelFileName HeatMapMultiplier PartTypeMultiplier\n\n", argv[0]);
        printf("Example: %s ../resources/im0110A.jpg ../resources/im0110B.jpg ../resources/epiBtoA.csv ../resources/epiAtoB.csv ../resources/partTypeScrBtoA.csv ../resources/partTypeScrAtoB.csv 12 0.1 ../output ../resources/model.txt 0.025 0.01\n", argv[0]);
        return 1;
    }
    
    // fetch arguments
    imageFileName = argv[1];
    secondImageFileName = argv[2];
    epipolarGeometryFileNameA2B = argv[3]; // yes these are inversed on purpose
    epipolarGeometryFileNameB2A = argv[4];
    partTypeScoreFileNameA2B = argv[6]; //and these are not inversed, again, on purpose
    partTypeScoreFileNameB2A = argv[5];
    maxIterations = atoi(argv[7]);
    requiredEpsilon = atof(argv[8]);
    outputFolder = std::string(argv[9]);
    modelFileName = argv[10];
    heatMultiplier = atof(argv[11]);
    partTypeMultiplier = atof(argv[12]);
    
    //keep track of filenames for debug purposes
    {
        std::string fullPathImgA(imageFileName);
        std::string fullPathImgB(secondImageFileName);
        
        //split with "/"
        std::vector<std::string> elems;
        split(fullPathImgA, '/', elems);
        imgFileA = elems.back();
#ifdef HUMANEVA
        std::string seq = elems.at(elems.size()-2);
        std::size_t posStart = seq.find('(');
        std::size_t posEnd = seq.find(')');
        viewA = seq.substr(posStart+1, (posEnd-1-posStart));
        frame = std::atoi(imgFileA.substr(2, 6).c_str()); // /media/emredog/research-data/HumanEva-I/set_VALIDATE/S1_Box_1_(C1)/im0010.bmp
#elif defined(UMPM)
        std::string seq = elems.at(elems.size()-2);  // /media/emredog/research-data/UMPM/Video_test/p1_chair_2_f/im2508.png
        viewA = seq.substr(seq.length()-1, seq.length());
        frame = std::atoi(imgFileA.substr(2, 6).c_str());
#endif


        elems.clear();
        split(fullPathImgB, '/', elems);
        imgFileB = elems.back();
#ifdef HUMANEVA
        seq = elems.at(elems.size()-2);
        posStart = seq.find('(');
        posEnd = seq.find(')');
        viewB = seq.substr(posStart+1, (posEnd-1-posStart));
        //frame should be same as viewA
#elif defined(UMPM)
        seq = elems.at(elems.size()-2);
        viewB = seq.substr(seq.length()-1, seq.length());
        //frame should be same as viewA
#endif
    }
    //create folder for this execution
    {
        char directoryName[200];
        std::sprintf(directoryName, "%s", outputFolder.c_str());
        
        std::string directoryStr(directoryName);
        
        struct stat st = {0};
        
        if (stat(directoryStr.c_str(), &st) == -1) // if dir does not exist
        {
            int status = _mkdir(directoryStr.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //with read/write/search permissions for owner and group, and with read/search permissions for others.
            if (status < 0)
            {
                std::cerr << "ERROR Could not create directory: " << directoryName << std::endl;
                return -1;
            }
        }
    }

//----------------------------------------------------------------------------
//  Read arguments for the Single-view version
//----------------------------------------------------------------------------
#else // with distance transform && single view (NOT MV NOR MV_PTC)

    if( argc == 4 )
    {
        imageFileName = argv[1];
        modelFileName = argv[2];
        outputFolder = argv[3];
    }
    else
    {
        printf("Usage: %s image_file [model_file]\n", argv[0]);
        printf("Example: %s ../resources/im0110.jpg ../resources/model.txt outputFolder\n", argv[0]);
        return 1;
    }
#endif	

    //keep track of filenames for debug purposes
    {
        std::string fullPathImgA(imageFileName);

        //split with "/"
        std::vector<std::string> elems;
        split(fullPathImgA, '/', elems);
        imgFileA = elems.back();
    }
    //create folder for this execution
    {
        char directoryName[200];
        std::sprintf(directoryName, "%s", outputFolder.c_str());

        std::string directoryStr(directoryName);

        struct stat st = {0};

        if (stat(directoryStr.c_str(), &st) == -1) // if dir does not exist
        {
            int status = _mkdir(directoryStr.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //with read/write/search permissions for owner and group, and with read/search permissions for others.
            if (status < 0)
            {
                std::cerr << "ERROR Could not create directory: " << directoryName << std::endl;
                return -1;
            }
        }
    }

#if LOGGING_ON
    initLog("detect_fast_C.log");
#endif

#ifdef USE_CUDA
    cudaWakeUp();
#endif

    // load image
    cv::Mat cvImg = cv::imread(imageFileName, -1); //image is assumed to have the original size (the right size for the epipolar geometry)
    myArray<unsigned char> img(&cvImg);

    // load model
    Model *model = load_model(modelFileName);
    
    
//----------------------------------------------------------------------------
//  Execute detect_fast for MV version
//----------------------------------------------------------------------------
#ifdef MV

    cv::Mat cvSecondImg = cv::imread(secondImageFileName, -1);
    myArray<unsigned char> imgB(&cvSecondImg);

    // read epipolar geometry file
    bool isEpiBorderPoints = true; //are we using epi border points (intersection points of the epi lines with image borders) or coefficients for epi lines.
    EpipolarGeometry epiGeometryA2B(epipolarGeometryFileNameA2B, isEpiBorderPoints);
    EpipolarGeometry epiGeometryB2A(epipolarGeometryFileNameB2A, isEpiBorderPoints);

    top(0);
    std::pair<const std::vector<FPTYPE>*, const std::vector<FPTYPE>* > boxPair = detect_fast_mv(&img, &imgB, model, &epiGeometryA2B, &epiGeometryB2A, maxIterations, requiredEpsilon);
    double duration = top(0);

    if (boxPair.first == NULL || boxPair.second == NULL)
    {
        std::cout << "Missing detection for detect_fast_mv(&img, &imgB, model, &epiLinesAtoB, &epiLinesBtoA)\n";
        return 1;
    }

    // display boxes
    //    printf("Image A: Detections after NMS\n");
    //    display_boxes(boxPair.first, model->partsNbr);
    //    printf("Image B: Detections after NMS\n");
    //    display_boxes(boxPair.second, model->partsNbr);

#ifdef SAVE_TEXT
    //save boxes
    char buffer[350];
    std::sprintf(buffer, "%s/%s_A_Pose.txt", outputFolder.c_str(), imgFileA.c_str());
    saveBoxCenters(boxPair.first, model->partsNbr, buffer);
    std::sprintf(buffer, "%s/%s_B_Pose.txt", outputFolder.c_str(), imgFileB.c_str());
    saveBoxCenters(boxPair.second, model->partsNbr, buffer);
#else
    DisplayTools::displayDetection(img, *(boxPair.first), model->partsNbr, 0);
    DisplayTools::displayDetection(cvSecondImg, *(boxPair.second), model->partsNbr, 0);
#endif //SAVE_TEXT

    //DisplayTools::displayBothDetections(cvImg, *(boxPair.first), cvSecondImg, *(boxPair.second), model->partsNbr);

    // non maximal suppression is already performed inside detect_fast_mv

    // display run time
    printf("\nJoint detection took %.2f seconds\n", duration/1000.0);
    
    
//----------------------------------------------------------------------------
//  Execute detect_fast for MV_PTC version
//----------------------------------------------------------------------------
#elif defined(MV_PTC)
    cv::Mat cvSecondImg = cv::imread(secondImageFileName, -1);
    myArray<unsigned char> imgB(&cvSecondImg);
    
    // read epipolar geometry file
    bool isEpiBorderPoints = true; //are we using epi border points (intersection points of the epi lines with image borders) or coefficients for epi lines.
    EpipolarGeometry epiGeometryA2B(epipolarGeometryFileNameA2B, isEpiBorderPoints);
    EpipolarGeometry epiGeometryB2A(epipolarGeometryFileNameB2A, isEpiBorderPoints);
    
    PartTypeCompat partTypeScoresA2B(partTypeScoreFileNameA2B, (model->filtersSz/model->partsNbr), model->partsNbr);
    PartTypeCompat partTypeScoresB2A(partTypeScoreFileNameB2A, (model->filtersSz/model->partsNbr), model->partsNbr);
    
    top(0);
    std::pair<const std::vector<FPTYPE>*, const std::vector<FPTYPE>* > boxPair = detect_fast_mvptc(&img, &imgB, model, &epiGeometryA2B, &epiGeometryB2A, &partTypeScoresA2B, &partTypeScoresB2A, maxIterations, requiredEpsilon);
    double duration = top(0);
    
    if (boxPair.first == NULL || boxPair.second == NULL)
    {
        std::cout << "Missing detection for detect_fast_mv(&img, &imgB, model, &epiLinesAtoB, &epiLinesBtoA)\n";
        return 1;
    }
    
#ifdef SAVE_TEXT
    //save boxes
    char buffer[350];
    std::sprintf(buffer, "%s/%s_A_Pose.txt", outputFolder.c_str(), imgFileA.c_str());
    saveBoxCenters(boxPair.first, model->partsNbr, buffer);
    std::sprintf(buffer, "%s/%s_B_Pose.txt", outputFolder.c_str(), imgFileB.c_str());
    saveBoxCenters(boxPair.second, model->partsNbr, buffer);
#endif //SAVE_TEXT
    
    // display run time
    printf("\nJoint detection with part type scores took %.2f seconds\n", duration/1000.0);
    
    
    
#else
//----------------------------------------------------------------------------
//  Execute Original detect_fast_C
//----------------------------------------------------------------------------
 // NOT MV NOR MV_PTC
    // run detection with distance transform
    top(0);
    std::vector<FPTYPE> *boxes = detect_fast_C(&img, model);
    double duration = top(0);

    // display boxes
    //printf("Detections before NMS\n");
    //display_boxes(boxes, model->partsNbr);

    // non maximal suppression
#ifdef USE_NMS
    std::vector<FPTYPE> *boxes_nms = nms(boxes, .1, model->partsNbr);
#else
    std::vector<FPTYPE> *boxes_nms = not_nms_pick_best(boxes, model->partsNbr);
#endif
    //printf("\nDetections after NMS\n");
    //display_boxes(boxes_nms, model->partsNbr);
    //debug
    //    DisplayTools::displayDetection(cvImg, *boxes_nms, model->partsNbr, 0);
    //debug

#ifdef SAVE_TEXT
    //save boxes
    char buffer[350];
    std::sprintf(buffer, "%s/%s_Pose.txt", outputFolder.c_str(), imgFileA.c_str());
    saveBoxCenters(boxes_nms, model->partsNbr, buffer);

#else
    DisplayTools::displayDetection(cvImg, *boxes_nms, model->partsNbr, 0);
#endif //SAVE_TEXT

    // display run time
    printf("detection took %.2f seconds\n", duration/1000.0);
#endif

    // release memory
#if defined(MV) || defined(MV_PTC)
    delete boxPair.first;
    delete boxPair.second;
#else
    delete boxes_nms;
    delete boxes;
#endif

    delete_model(&model);



    return 0;
}
