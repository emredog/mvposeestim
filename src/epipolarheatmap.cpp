#include "epipolarheatmap.h"
#include "epipolargeometry.h"

cv::Mat EpipolarHeatmap::calculateEpiHeatmap(LineCoefs coefs, int origRows, int origCols, int targetRows, int targetCols)
{
    if (targetRows <= 0 || targetCols <= 0)
        return cv::Mat();

    double coefA = coefs.coefA;
    double coefB = coefs.coefB;
    double coefC = coefs.coefC;

    //create a matrix with given size:
    cv::Mat binaryImg(origRows, origCols, CV_8UC1);
    binaryImg = cv::Scalar(255);

    // draw given line with cv::line function
    // get intersection Points
    int y0 = static_cast<int>(round((coefA * 0.0 + coefC) / -coefB)); //for x = 0
    int yEnd = static_cast<int>(round((coefA * static_cast<double>(origCols) + coefC) / -coefB)); //for x = origCols

    cv::line(binaryImg, cv::Point2d(0.0, y0), cv::Point2d(static_cast<double>(origCols), yEnd), cv::Scalar(0));

    //debug
//    cv::namedWindow( "cvLine", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
//    imshow( "cvLine", binaryImg );
//    cv::waitKey(0);
//    cv::destroyWindow("cvLine");
    //debug

    // Scale down the binary image to the desired size
    // -----------------------------------------------
    cv::Size targetSize(targetCols, targetRows);
    cv::Mat resizedBinaryImg;
    // FIXME: resize operation might be slow
    cv::resize(binaryImg, resizedBinaryImg, targetSize, 0.0, 0.0, CV_INTER_AREA); //from opencv doc: "To shrink an image,
                                                                                  //it will generally look best with CV_INTER_AREA interpolation..."

    // resized image is much more brighter than the original, so we paint the line in black with thresholding
    cv::Mat thresholded(resizedBinaryImg.size(), resizedBinaryImg.type());
    cv::threshold(resizedBinaryImg, thresholded, 254.0, 255.0, CV_THRESH_BINARY); //threshold is 254

    //debug
//    cv::namedWindow( "resized", CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
//    imshow( "resized", resizedBinaryImg );
//    cv::waitKey(0);
//    cv::destroyWindow("resized");
//    cv::namedWindow( "thresholded", CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
//    imshow( "thresholded", thresholded );
//    cv::waitKey(0);
//    cv::destroyWindow("thresholded");
    //debug


    // Apply Distance transform to the epipolar line
    // -------------------------
    cv::Mat distanceMap(targetRows, targetCols, CV_64FC1);
    cv::distanceTransform(thresholded, distanceMap, CV_DIST_L2, 3);
    //distanceMap = every pixel has the distance to closest zero-valued pixel
    //in our case, the epipolar line has zero, other points are non-zero
    // So, farther points has bigger values

    cv::Mat heatMap(distanceMap.size(), CV_64FC1);
    cv::sqrt(distanceMap, heatMap); // the distance should have even more effect (square of euclidian distance)
    heatMap = -heatMap; //and negate the value to define a cost from distance (==> large distance means colder = smaller value)


    // normalize the distance transform and scale it between [0, 1]
    double min, max;
    cv::minMaxIdx(heatMap, &min, &max);
    cv::Mat normalizedHeatMap(heatMap.size(), CV_64FC1);
    cv::normalize(heatMap, normalizedHeatMap, 1.0, 0.0, cv::NORM_MINMAX);
//    normalizedHeatMap = (heatMap - min) / (max-min);

    // debug
//            cv::namedWindow( "DistanceTransform", CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED );
//            imshow( "DistanceTransform", normalizedHeatMap);
//            cv::waitKey(0);
//            cv::destroyWindow("DistanceTransform");
    // debug

    return normalizedHeatMap;
}

cv::Mat EpipolarHeatmap::calculateEpiHeatmap(PointPair epiBorderPts, int origRows, int origCols, int targetRows, int targetCols)
{
    if (targetRows <= 0 || targetCols <= 0)
        return cv::Mat();

    //create a matrix with given size:
    cv::Mat binaryImg(origRows, origCols, CV_8UC1);
    binaryImg = cv::Scalar(255);

    // draw given line with cv::line function
    cv::line(binaryImg, epiBorderPts.first, epiBorderPts.second, cv::Scalar(0));

    // Scale down the binary image to the desired size
    // -----------------------------------------------
    cv::Size targetSize(targetCols, targetRows);
    cv::Mat resizedBinaryImg;
    // FIXME: resize operation might be slow
    cv::resize(binaryImg, resizedBinaryImg, targetSize, 0.0, 0.0, CV_INTER_AREA); //from opencv doc: "To shrink an image,
                                                                                  //it will generally look best with CV_INTER_AREA interpolation..."

    // resized image is much more brighter than the original, so we paint the line in black with thresholding
    cv::Mat thresholded(resizedBinaryImg.size(), resizedBinaryImg.type());
    cv::threshold(resizedBinaryImg, thresholded, 254.0, 255.0, CV_THRESH_BINARY); //threshold is 254

    // Apply Distance transform to the epipolar line
    // -------------------------
    cv::Mat distanceMap(targetRows, targetCols, CV_64FC1);
    cv::distanceTransform(thresholded, distanceMap, CV_DIST_L2, 3);
    //distanceMap = every pixel has the distance to closest zero-valued pixel
    //in our case, the epipolar line has zero, other points are non-zero
    // So, farther points has bigger values

    cv::Mat heatMap(distanceMap.size(), CV_64FC1);
    cv::sqrt(distanceMap, heatMap); // the distance should have even more effect (square of euclidian distance)
    heatMap = -heatMap; //and negate the value to define a cost from distance (==> large distance means colder = smaller value)

    // normalize the distance transform and scale it between [0, 1]
    double min, max;
    cv::minMaxIdx(heatMap, &min, &max);
    cv::Mat normalizedHeatMap(heatMap.size(), CV_64FC1);
    cv::normalize(heatMap, normalizedHeatMap, 1.0, 0.0, cv::NORM_MINMAX);

    return normalizedHeatMap;
}

