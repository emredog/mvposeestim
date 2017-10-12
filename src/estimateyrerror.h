//
//  estimateyrerror.hpp
//  C-version
//
//  Created by emredog on 07/08/16.
//  Copyright © 2016 emredog. All rights reserved.
//

#ifndef estimateyrerror_h
#define estimateyrerror_h

#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>


#ifdef HUMANEVA

#define NB_NODES 90  // using NN: netHE09_100out.m
#define SW_TL_RATIO 0.7648

#elif defined(UMPM)

#define NB_NODES 95
#define SW_TL_RATIO 0.8372 // using NN: netUmpm09_100out.m

#endif

#define INPUT_DIM 53
#define NB_PARTS 26
#define ARBIT_Z 2000.0
#define PI 3.1415926535897

typedef struct Calib
{
    //    FC      - Focal length (2x1)
    FPTYPE fcX;
    FPTYPE fcY;
    //    CC      - Principle point (2x1)
    FPTYPE cX;
    FPTYPE cY;
    
    //    ALPHA_C - Skew coefficient (1x1)
    //    KC      - Distortion coefficient (5x1)
    //    RC_EXT  - Extrinsic rotation matrix (3x3)
    //    OMC_EXT - Extrinsic rotation angles, simply a decomposition
    //              of RC_EXT (3x1)
    //    TC_EXT  - Extrinsic translation vector (3x1)
} Calib;

class YRerrorEstimator
{
public:
    /// Returns estimated single view errors for 26 parts, given 53d features.
#ifdef HUMANEVA
    static std::vector<FPTYPE> estimatePartBasedError_mimo(const std::vector<FPTYPE> *boxes, std::string view); // view= "C1" / "C2" / "C3"
#elif defined(UMPM)
    static std::vector<FPTYPE> estimatePartBasedError_mimo(const std::vector<FPTYPE> *boxes, std::string view, std::string action); // view= "f" / "l" / "r" / "s" (action: because different calib files are provided for different sequences)
#endif
    
private:
    YRerrorEstimator();
    ~YRerrorEstimator();
    
    
    // HELPER FUNCTIONS    
    static FPTYPE calculateAlpha(const std::vector<FPTYPE> *boxes, const Calib *cal, bool scaleAlpha = false);//if true, scales alpha between [0, PI/2]
    static void bodyCenterAndTorsoLen2D(const std::vector<FPTYPE> *boxes, /*OUTPUT*/ cv::Point2d *bodyCenter, cv::Vec2d *dirSpineVector, FPTYPE *torsoLen);
    static FPTYPE torsoLen3D(std::vector<cv::Point3d> pose3d);
    static void partAnglesAndNormalizedDists2D(const std::vector<FPTYPE> *boxes, /*OUTPUT*/ std::vector<FPTYPE> *betas, std::vector<FPTYPE> *dists);
    static cv::Point3d backprojectPoint(cv::Point2d pt, FPTYPE z, Calib cal);
    static std::vector<cv::Point3d> backprojectYrPose(const std::vector<FPTYPE> *boxes, FPTYPE z, Calib cal);
    static std::vector<cv::Point2d> lineCircleIntersections(FPTYPE x1, FPTYPE y1, FPTYPE x2, FPTYPE y2, cv::Point2d center, FPTYPE radius);
#ifdef UMPM
    static inline const Calib* fetchCalibInfo(std::string view, std::string action);
#endif
    
    // NN Module functions
    static cv::Mat mapminmax_apply(cv::Mat x, cv::Mat settings_gain, cv::Mat settings_xoffset, FPTYPE settings_ymin);
    static cv::Mat tansig_apply(cv::Mat n);
    static FPTYPE mapminmax_reverse(FPTYPE y, FPTYPE settings_gain, FPTYPE settings_xoffset, FPTYPE settings_ymin);
    static std::vector<FPTYPE> mapminmax_reverse_vect(cv::Mat y, cv::Mat settings_gain, cv::Mat settings_xoffset, FPTYPE settings_ymin);
    
    
    
    // Calibration data
#ifdef HUMANEVA
    static const Calib  CalC1, CalC2, CalC3; // just intrinsic params
    // these intrinsic params are same subject-wise, so don't need to load different cal files for different subjects. (whew.)
#elif defined(UMPM)
    static const Calib CalibChairF, CalibChairL, CalibChairR, CalibChairS, // just intrinsic params
    CalibGrabF, CalibGrabL, CalibGrabR, CalibGrabS,
    CalibOrthosynF, CalibOrthosynL, CalibOrthosynR, CalibOrthosynS,
    CalibTableF, CalibTableL, CalibTableR, CalibTableS,
    CalibTriangleF, CalibTriangleL, CalibTriangleR, CalibTriangleS;
#endif
    
    // weights and other parameters of the neural net HOLISTIC/MIMO:
    //=========================================================
    // Input 1                              // dimensions are for MIMO net
    static const cv::Mat x1_step1_xoffset;  // (INPUT_DIM x 1) matrix to offset the input
    static const cv::Mat x1_step1_gain;     // (INPUT_DIM x1) matrix to scale the input
    static const FPTYPE x1_step1_ymin;      // scalar to scale the input
    
    // Layer 1
    static const cv::Mat b1;                // (NB_NODES x 1) matrix for bias on first layer

    static const cv::Mat IW1_1;             // (NB_NODES x INPUT_DIM) weight matrix on first layer
    
    // Layer 2
    static const cv::Mat b2;                // (NB_NODES x 1) matrix for bias on second layer
    
    static const cv::Mat LW2_1;             // (NB_NODES x NB_NODES) weight matrix on second layer
    
    // Layer 3
    static const cv::Mat b3;                // (NB_PARTS x 1) matrix for bias on output layer
    
    static const cv::Mat LW3_2;             // (NB_PARTS x NB_NODES) weight matrix on the output layer
    
    // Output 1
    static const FPTYPE y1_step1_ymin;      // scalar to re-scale the output
    static const cv::Mat y1_step1_gain;      // (NB_PARTS x 1) matrix to rescale the output
    static const cv::Mat y1_step1_xoffset;   // (NB_PARTS x 1) matrix to offset-back the output
    
};


#endif /* estimateyrerror_h */
