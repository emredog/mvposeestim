#ifndef DISPLAYTOOLS_H
#define DISPLAYTOOLS_H

#include "myarray.hpp"
#include "epipolargeometry.h"

class DisplayTools
{
public:
    static void displayMyArrayAsHeatMap(const myArray<FPTYPE>* myArr, const char* windowName = NULL, bool display = true, bool save = false);
    static void displayHeatMap(const cv::Mat& heatMap, const char* windowName = NULL, bool display = true, bool save = false);
    static void display_boxes_csvCompatible(std::vector<FPTYPE> *boxes, int partsNbr);
    static void displayDetection(const cv::Mat& img, const std::vector<FPTYPE>& boxes, int partsNbr, int waitKey = 0 );
    static void displayBothDetections(const cv::Mat& imgA, const std::vector<FPTYPE>& boxesA,
                                      const cv::Mat& imgB, const std::vector<FPTYPE>& boxesB, int partsNbr);
    static void displayDetection(const myArray<uchar> &myArr, const std::vector<FPTYPE>& boxes, int partsNbr, int waitKey = 0 );

    static void saveDetection(const myArray<uchar> &myArr, const std::vector<FPTYPE> &boxes, int partsNbr, const char *fileName);
    static void saveDetectionPartBased(const myArray<uchar> &myArr, const std::vector<FPTYPE> &boxes, int partsNbr, const char *fileName);

    static void saveEpipolarLines(const myArray<uchar> &myArr, const std::vector<LineCoefs> &lines,  const char *fileName);
    static void saveEpipolarLines(const myArray<uchar> &myArr, const std::vector<PointPair> &allEpiPoints,  const char *fileName);
    static void saveEpipolarLinesPartBased(const myArray<uchar> &myArr, const std::vector<PointPair> &allEpiPoints,  const char *fileName);


    //unused
    static void saveOneEpipolarLine(const myArray<uchar> &myArr, LineCoefs& line, const char* fileName);
protected:
    DisplayTools();
    static std::vector<cv::Scalar> prepareColors(int partsNbr);
};

#endif // DISPLAYTOOLS_H
