#ifndef EPIPOLARHEATMAP_H
#define EPIPOLARHEATMAP_H

#include <opencv2/opencv.hpp>
#include "top.h"

//fwd declaration
struct LineCoefs;
typedef std::pair<cv::Point2d, cv::Point2d> PointPair;

class EpipolarHeatmap
{
public:
    /**
     * @brief calculateEpiHeatmap Function that calculates epipolar heatmap to add to the part score
     * @param coefA A in Ax + By + C = 0
     * @param coefB B in Ax + By + C = 0
     * @param coefC C in Ax + By + C = 0
     * @param origRows Original row size of the image, where the epipolar lines are calculated
     * @param origCols Original column size of the image, where the epipolar lines are calculated
     * @param targetRows Desired row size of the output heatmap (should be same size with the partscore)
     * @param targetCols Desired column size of the output heatmap (should be same size with the partscore)
     * @return
     */
    static cv::Mat calculateEpiHeatmap(LineCoefs coefs, int origRows, int origCols, int targetRows, int targetCols);
    static cv::Mat calculateEpiHeatmap(PointPair epiBorderPts, int origRows, int origCols, int targetRows, int targetCols);
    static const FPTYPE EpiHeatMapWeight = 1.0;
    //note: coefficients correspond to these ones: http://www.mathworks.com/help/vision/ref/epipolarline.html#zmw57dd0e135420
protected:
    EpipolarHeatmap();
};

#endif // EPIPOLARHEATMAP_H
