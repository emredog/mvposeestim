#include "displaytools.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

#define LINE_THICKNESS 2

#if defined(HUMANEVA)
#define ROWS 484;
#define COLS 644;
#elif defined(UMPM)
#define ROWS 486
#define COLS 644
#endif

inline void boxCenter(FPTYPE x1, FPTYPE y1, FPTYPE x2, FPTYPE y2,
                      /*OUTPUTS:*/ double *centerX, double *centerY)
{
    *centerX = (x2-x1)/2.0 + x1;
    *centerY = (y2-y1)/2.0 + y1;
}

DisplayTools::DisplayTools()
{
}

std::vector<cv::Scalar> DisplayTools::prepareColors(int partsNbr)
{
    std::vector<cv::Scalar> colorSet;

    if (partsNbr == 26)  // human eva
    {
        //colorset = {'g','g','y','m','m','m','m','y','y','y','r','r','r','r','y','c','c','c','c','y','y','y','b','b','b','b'};

        //                          BLUE GREEN RED
        colorSet.push_back(cv::Scalar(0, 255, 0));      //green
        colorSet.push_back(cv::Scalar(0, 255, 0));      //green
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(255, 0, 255));    //magenta
        colorSet.push_back(cv::Scalar(255, 0, 255));    //magenta
        colorSet.push_back(cv::Scalar(255, 0, 255));    //magenta
        colorSet.push_back(cv::Scalar(255, 0, 255));    //magenta
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(0, 0, 255));      //red
        colorSet.push_back(cv::Scalar(0, 0, 255));      //red
        colorSet.push_back(cv::Scalar(0, 0, 255));      //red
        colorSet.push_back(cv::Scalar(0, 0, 255));      //red
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(255, 255, 0));    //cyan
        colorSet.push_back(cv::Scalar(255, 255, 0));    //cyan
        colorSet.push_back(cv::Scalar(255, 255, 0));    //cyan
        colorSet.push_back(cv::Scalar(255, 255, 0));    //cyan
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(255, 0, 0));      //blue
        colorSet.push_back(cv::Scalar(255, 0, 0));      //blue
        colorSet.push_back(cv::Scalar(255, 0, 0));      //blue
        colorSet.push_back(cv::Scalar(255, 0, 0));      //blue
    }
    else if (partsNbr == 15)  // umpm
    {
        //                          BLUE GREEN RED
        colorSet.push_back(cv::Scalar(0, 255, 0));      //green
        colorSet.push_back(cv::Scalar(0, 255, 0));      //green
        colorSet.push_back(cv::Scalar(255, 0, 255));    //magenta
        colorSet.push_back(cv::Scalar(255, 0, 255));    //magenta
        colorSet.push_back(cv::Scalar(255, 0, 255));    //magenta
        colorSet.push_back(cv::Scalar(0, 255, 255));    //yellow
        colorSet.push_back(cv::Scalar(0, 0, 255));      //red
        colorSet.push_back(cv::Scalar(0, 0, 255));      //red
        colorSet.push_back(cv::Scalar(0, 0, 255));      //red
        colorSet.push_back(cv::Scalar(255, 0, 0));      //blue
        colorSet.push_back(cv::Scalar(255, 0, 0));      //blue
        colorSet.push_back(cv::Scalar(255, 0, 0));      //blue
        colorSet.push_back(cv::Scalar(255, 255, 0));    //cyan
        colorSet.push_back(cv::Scalar(255, 255, 0));    //cyan
        colorSet.push_back(cv::Scalar(255, 255, 0));    //cyan
    }
    else //unexpected number of parts
    {
        for (int p=0; p<partsNbr; p++)
            colorSet.push_back(cv::Scalar(255, 255, 255));
    }

    return colorSet;

}

void DisplayTools::displayMyArrayAsHeatMap(const myArray<FPTYPE> *myArr, const char *windowName, bool display, bool save)
{
    int rows = myArr->getRows();
    int cols = myArr->getCols();


    std::string nameForWindow = "";
    if (windowName != NULL)
        nameForWindow = std::string(windowName);
    else
        nameForWindow = "Normalized HeatMap";

    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_64FC1);
    cv::MatIterator_<FPTYPE> it, end = mat.end<FPTYPE>();

    for (it = mat.begin<FPTYPE>(); it != end; ++it)
    {
        cv::Point2i pos = it.pos();
        *it = myArr->get(pos.y, pos.x);
    }

    double min, max;
    cv::minMaxIdx(mat, &min, &max);



    if (display)
    {
        cv::Mat normalizedMat(mat.size(), CV_64FC1);
        cv::normalize(mat, normalizedMat, 1.0, 0.0, cv::NORM_MINMAX);

        cv::namedWindow(nameForWindow, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED );
        cv::imshow(nameForWindow, normalizedMat);
        cv::waitKey(0);
        cv::destroyWindow(nameForWindow);
    }
    if (save)
    {
        cv::Mat normalizedMat(mat.size(), CV_8UC1);
        cv::normalize(mat, normalizedMat, 255, 0.0, cv::NORM_MINMAX);

        cv::imwrite(nameForWindow, normalizedMat);
        //cv::imwrite(nameForWindow, mat);
    }

}

void DisplayTools::displayHeatMap(const cv::Mat &heatMap, const char *windowName, bool display, bool save)
{
    std::string nameForWindow = "";
    if (windowName != NULL)
        nameForWindow = std::string(windowName);
    else
        nameForWindow = "Normalized HeatMap";

    double min, max;
    cv::minMaxIdx(heatMap, &min, &max);

    if (display)
    {
        cv::Mat normalizedMat(heatMap.size(), CV_64FC1);
        cv::normalize(heatMap, normalizedMat, 1.0, 0.0, cv::NORM_MINMAX);

        cv::namedWindow( nameForWindow, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED );
        cv::imshow(nameForWindow, normalizedMat);
        cv::waitKey(0);
        cv::destroyWindow(nameForWindow);
    }
    if (save)
    {
        cv::Mat normalizedMat(heatMap.size(), CV_8UC1);
        cv::normalize(heatMap, normalizedMat, 255.0, 0.0, cv::NORM_MINMAX);

        //cv::imwrite(nameForWindow, normalizedMat);
        cv::imwrite(nameForWindow, heatMap);
    }
}

void DisplayTools::display_boxes_csvCompatible(std::vector<FPTYPE> *boxes, int partsNbr)
{
    int detectionsNbr = boxes->size() / (4*partsNbr+2);
    for(int d = 1; d <= detectionsNbr; d++)
    {
        printf("detection #%d:\n", d);

        // parts coordinates
        for(int k = 1; k <= partsNbr; k++)
        {
            //            printf(" - part #%d:", k);
            for(int i = 0; i < 4; i++)
                printf("%.5g, ", (*boxes)[(d-1)*(4*partsNbr+2) + 4*(k-1) + i]);
            printf("\n");
        }

        // component Id
        //        printf(" - component id: %.5g\n", (*boxes)[(d-1)*(4*partsNbr+2) + 4*partsNbr]);

        // score
        //        printf(" - score: %.5g\n", (*boxes)[(d-1)*(4*partsNbr+2) + 4*partsNbr+1]);
    }
}

void DisplayTools::displayDetection(const cv::Mat &img, const std::vector<double> &boxes, int partsNbr, int waitKey)
{
    //check for boxes size and part number compatibility

    if ( boxes.size() % (4*partsNbr+2) != 0 )
    {
        std::cerr << "ERROR boxes size (" << boxes.size() << ") and number of parts (" << partsNbr << ") are not compatible.\n\n";
        return;
    }

    char buffer[20];

    int nbOfDetections = boxes.size() / (4*partsNbr+2);

    //prepare colors
    std::vector<cv::Scalar> colorSet = DisplayTools::prepareColors(partsNbr);

    //convert img to RGB (so that the boxes may have different colors)
    cv::Mat imgRGB;
    if (img.channels() == 1)
        cv::cvtColor(img, imgRGB, cv::COLOR_GRAY2BGR);
    else
        imgRGB = img;

    //for each detection
    for (int d=0; d<nbOfDetections; d++)
    {
        //draw every box on the image
        for(int p = 0; p < partsNbr; p++)
        {
            //points
            cv::Point2d p1(boxes[d*boxes.size() + 4*p + 0], boxes[d*boxes.size() + 4*p + 1]);
            cv::Point2d p2(boxes[d*boxes.size() + 4*p + 2], boxes[d*boxes.size() + 4*p + 3]);
            //rectangle
            cv::rectangle(imgRGB, p1, p2, colorSet.at(p));
        }

        // display score
        int offset = img.size().height / 12;
        std::sprintf(buffer, "Score %d %.3f", d+1, boxes[d*boxes.size() + 4*partsNbr + 1]);
        cv::putText(imgRGB, buffer, cv::Point2i(offset, imgRGB.size().height - (d+1)*offset),
                    cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0,0,255));
    }


    std::sprintf(buffer, "Detections_%d", waitKey);

    cv::namedWindow( buffer, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
    imshow( buffer, imgRGB );

    if (waitKey == 0) // keep the window open until keypress
    {
        cv::waitKey(0);
        cv::destroyAllWindows(); // probably there are other windows
        return;
    }
    else
    {
        cv::waitKey(waitKey);
        return;
    }
}

void DisplayTools::displayDetection(const myArray<uchar> &myArr, const std::vector<double> &boxes, int partsNbr, int waitKey)
{
    int rows = myArr.getRows();
    int cols = myArr.getCols();

    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::MatIterator_<uchar> it, end = mat.end<uchar>();

    for (it = mat.begin<uchar>(); it != end; ++it)
    {
        cv::Point2i pos = it.pos();
        *it = myArr.get(pos.y, pos.x);
    }

    return DisplayTools::displayDetection(mat, boxes, partsNbr, waitKey);
}

void DisplayTools::saveDetection(const myArray<uchar> &myArr, const std::vector<FPTYPE> &boxes, int partsNbr,
                                 const char *fileName)
{
    int rows = myArr.getRows();
    int cols = myArr.getCols();

    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::MatIterator_<uchar> it, end = mat.end<uchar>();

    for (it = mat.begin<uchar>(); it != end; ++it)
    {
        cv::Point2i pos = it.pos();
        *it = myArr.get(pos.y, pos.x);
    }

    //check for boxes size and part number compatibility

    if ( boxes.size() % (4*partsNbr+2) != 0 )
    {
        std::cerr << "ERROR boxes size (" << boxes.size() << ") and number of parts (" << partsNbr << ") are not compatible.\n\n";
        return;
    }

    int nbOfDetections = boxes.size() / (4*partsNbr+2);

    //prepare colors
    std::vector<cv::Scalar> colorSet = DisplayTools::prepareColors(partsNbr);

    //convert img to RGB (so that the boxes may have different colors)
    cv::Mat imgRGB;
    cv::cvtColor(mat, imgRGB, cv::COLOR_GRAY2BGR);

    //for each detection
    for (int d=0; d<nbOfDetections; d++)
    {
        //draw every box on the image
        for(int p = 0; p < partsNbr; p++)
        {
            //points
            cv::Point2d p1(boxes[d*boxes.size() + 4*p + 0], boxes[d*boxes.size() + 4*p + 1]);
            cv::Point2d p2(boxes[d*boxes.size() + 4*p + 2], boxes[d*boxes.size() + 4*p + 3]);
            //rectangle
            cv::rectangle(imgRGB, p1, p2, colorSet.at(p), LINE_THICKNESS);
        }
    }

    if (!cv::imwrite(fileName, imgRGB))
        std::cerr << "Imwrite failed for DisplayTools::saveDetection !\n\n";
}

void DisplayTools::saveDetectionPartBased(const myArray<uchar> &myArr, const std::vector<double> &boxes, int partsNbr, const char *fileName)
{
    int rows = myArr.getRows();
    int cols = myArr.getCols();

    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::MatIterator_<uchar> it, end = mat.end<uchar>();

    for (it = mat.begin<uchar>(); it != end; ++it)
    {
        cv::Point2i pos = it.pos();
        *it = myArr.get(pos.y, pos.x);
    }

    //check for boxes size and part number compatibility

    if ( boxes.size() % (4*partsNbr+2) != 0 )
    {
        std::cerr << "ERROR boxes size (" << boxes.size() << ") and number of parts (" << partsNbr << ") are not compatible.\n\n";
        return;
    }

    char buffer[200];
    double centerX, centerY;

    int nbOfDetections = boxes.size() / (4*partsNbr+2);

    //prepare colors
    std::vector<cv::Scalar> colorSet = DisplayTools::prepareColors(partsNbr);

    //convert img to RGB (so that the boxes may have different colors)
    cv::Mat imgRGB, imgRGBClean;
    cv::cvtColor(mat, imgRGBClean, cv::COLOR_GRAY2BGR);

    //for each detection
    for (int d=0; d<nbOfDetections; d++)
    {
        //draw every box on the image
        for(int p = 0; p < partsNbr; p++)
        {
            imgRGB = imgRGBClean.clone();
            //points
            cv::Point2d p1(boxes[d*boxes.size() + 4*p + 0], boxes[d*boxes.size() + 4*p + 1]);
            cv::Point2d p2(boxes[d*boxes.size() + 4*p + 2], boxes[d*boxes.size() + 4*p + 3]);
            //rectangle
            cv::rectangle(imgRGB, p1, p2, colorSet.at(p));

            // display part number
            int offset = imgRGB.size().height / 12;
            boxCenter(p1.x, p1.y, p2.x, p2.y, &centerX, &centerY);
            std::sprintf(buffer, "Part %d: (%.2f, %.2f)", p+1, centerX, centerY);
            cv::putText(imgRGB, buffer, cv::Point2i(offset, imgRGB.size().height - (d+1)*offset),
                        cv::FONT_HERSHEY_PLAIN, 1.2, colorSet.at(p));

            std::sprintf(buffer, "%s_p%d.png", fileName, p+1);

            if (!cv::imwrite(buffer, imgRGB))
                std::cerr << "Imwrite failed for DisplayTools::saveDetection !\n\n";
        }


    }
}

void DisplayTools::saveEpipolarLines(const myArray<uchar> &myArr, const std::vector<LineCoefs>& lines, const char *fileName)
{
    // assumption: linecoefs has nbOfParts (26/15) elements
    int partsNbr = lines.size();

    int rows = myArr.getRows();
    int cols = myArr.getCols();

    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::MatIterator_<uchar> it, end = mat.end<uchar>();

    for (it = mat.begin<uchar>(); it != end; ++it)
    {
        cv::Point2i pos = it.pos();
        *it = myArr.get(pos.y, pos.x);
    }

    //prepare colors
    std::vector<cv::Scalar> colorSet = DisplayTools::prepareColors(partsNbr);

    //convert img to RGB (so that the boxes may have different colors)
    cv::Mat imgRGB;
    cv::cvtColor(mat, imgRGB, cv::COLOR_GRAY2BGR);

    // FIXME: get these from a global constant
    int origRows = 484;
    int origCols = 644;

    double scale = static_cast<double>(rows) / static_cast<double>(origRows);
    // scaling is assumed same in X and Y dimensions

    //draw every line on the image
    for(int p = 0; p < partsNbr; p++)
    {
        double coefA = lines.at(p).coefA;
        double coefB = lines.at(p).coefB;
        double coefC = lines.at(p).coefC;

        // draw given line with cv::line function
        // get intersection Points
        double origY0 = (coefA * 0.0 + coefC) / -coefB; //for x = 0
        double origYEnd = (coefA * static_cast<double>(origCols) + coefC) / -coefB; //for x = origCols


        cv::line(imgRGB, cv::Point2d(0.0, origY0*scale),
                 cv::Point2d(static_cast<double>(cols), origYEnd*scale), colorSet.at(p));
    }

    if (!cv::imwrite(fileName, imgRGB))
        std::cerr << "Imwrite failed for DisplayTools::saveEpipolarLines !\n\n";
}

void DisplayTools::saveEpipolarLines(const myArray<uchar> &myArr, const std::vector<PointPair> &allEpiPoints, const char *fileName)
{
    // assumption: allEpiPoints has nbOfParts (26/15) elements
    int partsNbr = allEpiPoints.size();

    int rows = myArr.getRows();
    int cols = myArr.getCols();

    int origRows = ROWS;
    int origCols = COLS;

    // scaling is assumed same in X and Y dimensions

    //create a cv::mat from myArray
    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::MatIterator_<uchar> it, end = mat.end<uchar>();

    for (it = mat.begin<uchar>(); it != end; ++it)
    {
        cv::Point2i pos = it.pos();
        *it = myArr.get(pos.y, pos.x);
    }

    //prepare colors
    std::vector<cv::Scalar> colorSet = DisplayTools::prepareColors(partsNbr);

    //convert img to RGB (so that the boxes may have different colors)
    cv::Mat imgRGB;
    cv::cvtColor(mat, imgRGB, cv::COLOR_GRAY2BGR);
    cv::Mat imRGBresized;
    cv::resize(imgRGB, imRGBresized, cv::Size(origCols, origRows));

    //draw every line on the image
    for(int p = 0; p < partsNbr; p++)
    {
        PointPair ptPair = allEpiPoints.at(p);

        // draw given line with cv::line function
        cv::line(imRGBresized, ptPair.first, ptPair.second, colorSet.at(p), LINE_THICKNESS);
    }

    if (!cv::imwrite(fileName, imRGBresized))
        std::cerr << "Imwrite failed for DisplayTools::saveEpipolarLines !\n\n";
}

void DisplayTools::saveEpipolarLinesPartBased(const myArray<uchar> &myArr, const std::vector<PointPair> &allEpiPoints, const char *fileName)
{
    // assumption: allEpiPoints has nbOfParts (26) elements
    int partsNbr = allEpiPoints.size();

    int rows = myArr.getRows();
    int cols = myArr.getCols();

    // scaling is assumed same in X and Y dimensions

    //create a cv::mat from myArray
    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::MatIterator_<uchar> it, end = mat.end<uchar>();

    for (it = mat.begin<uchar>(); it != end; ++it)
    {
        cv::Point2i pos = it.pos();
        *it = myArr.get(pos.y, pos.x);
    }

    //prepare colors
    std::vector<cv::Scalar> colorSet = DisplayTools::prepareColors(partsNbr);

    char buffer[200];

    //convert img to RGB (so that the boxes may have different colors)
    cv::Mat imgRGB, imRGBClean;
    cv::cvtColor(mat, imRGBClean, cv::COLOR_GRAY2BGR);

    //draw every line on the image
    for(int p = 0; p < partsNbr; p++)
    {
        imgRGB = imRGBClean.clone();
        PointPair ptPair = allEpiPoints.at(p);

        // draw given line with cv::line function
        cv::line(imgRGB, ptPair.first, ptPair.second, colorSet.at(p));

        std::sprintf(buffer, "%s_p%d.png", fileName, p+1);
        if (!cv::imwrite(buffer, imgRGB))
            std::cerr << "Imwrite failed for DisplayTools::saveEpipolarLinesPartBased !\n\n";
    }
}

void DisplayTools::saveOneEpipolarLine(const myArray<uchar> &myArr, LineCoefs &line, const char *fileName)
{
    int rows = myArr.getRows();
    int cols = myArr.getCols();

    cv::Mat mat = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::MatIterator_<uchar> it, end = mat.end<uchar>();

    for (it = mat.begin<uchar>(); it != end; ++it)
    {
        cv::Point2i pos = it.pos();
        *it = myArr.get(pos.y, pos.x);
    }

    // draw given line with cv::line function
    // get intersection Points
    int y0 = static_cast<int>(round((line.coefA * 0.0 + line.coefC) / -line.coefB)); //for x = 0
    int yEnd = static_cast<int>(round((line.coefA * static_cast<double>(cols) + line.coefC) / -line.coefB)); //for x = origCols

    cv::line(mat, cv::Point2d(0.0, y0), cv::Point2d(static_cast<double>(cols), yEnd), cv::Scalar(255));
    if (!cv::imwrite(fileName, mat))
        std::cerr << "Imwrite failed for DisplayTools::saveOneEpipolarLine !\n\n";
}

void DisplayTools::displayBothDetections(const cv::Mat &imgA, const std::vector<double> &boxesA,
                                         const cv::Mat &imgB, const std::vector<double> &boxesB, int partsNbr)
{
    DisplayTools::displayDetection(imgA, boxesA, partsNbr, 30);
    DisplayTools::displayDetection(imgB, boxesB, partsNbr, 0);
}




