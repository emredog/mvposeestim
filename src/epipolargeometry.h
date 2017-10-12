#ifndef EPIPOLARGEOMETRY_H
#define EPIPOLARGEOMETRY_H

#include <map>
#include <opencv2/opencv.hpp>

struct LineCoefs{
    FPTYPE coefA;
    FPTYPE coefB;
    FPTYPE coefC;

};

typedef std::pair<cv::Point2d, cv::Point2d> PointPair;

class EpipolarGeometry
{
public:
    EpipolarGeometry(const char *csvFileName, bool isEpiBorderPoints, int origRowSize = -1);

    //row and col should be zero indexed.
    LineCoefs getEpipolarLineCoefs(int col, int row) const;
    PointPair getEpiBorderPoints(int col, int row) const;
    int getOrigRowSize() const;
    int getOrigColSize() const;
protected:
    void initWithEpiLines(const char *csvFileName, int origHeight = -1);
    void initWithEpiBorderPoints(const char *csvFileName, int origHeight = -1);
    int height;
    int width;

    std::map<long, LineCoefs> lineData;
    std::map<long, PointPair> pointData;
};

#endif // EPIPOLARGEOMETRY_H
