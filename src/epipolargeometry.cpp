#include "epipolargeometry.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "top.h"

using namespace std;

EpipolarGeometry::EpipolarGeometry(const char *csvFileName, bool isEpiBorderPoints, int origRowSize)
{

    if (isEpiBorderPoints)
        this->initWithEpiBorderPoints(csvFileName, origRowSize);
    else
        this->initWithEpiLines(csvFileName, origRowSize);

}

LineCoefs EpipolarGeometry::getEpipolarLineCoefs(int x, int y) const
{
    if (this->lineData.empty())
    {
        std::cerr << "WARNING lineData is empty. Maybe you should've ask for an PointPair object?\n\n";
        return LineCoefs();
    }

    if (y <= 0 || y >= height || x <= 0 || x >= width)
    {
        std::cerr << "WARNING EpipolarGeometry::getEpipolarLineCoefs was called for a point that is out of original image boundaries!!\n"
                  << "\trow: " << y << "\tcol: " << x;
        return LineCoefs();
    }

    //fetch & return the coefficients:
    return this->lineData.at((y-1)*width + (x-1)); //row and col are 1-indexed, so fix them before fetching anything
}

PointPair EpipolarGeometry::getEpiBorderPoints(int x, int y) const
{
    if (this->pointData.empty())
    {
        std::cerr << "WARNING pointData is empty. Maybe you should've ask for an LineCoefs object?\n\n";
        return PointPair();
    }

    if (y <= 0 || y >= height || x <= 0 || x >= width)
    {
        std::cerr << "WARNING EpipolarGeometry::getEpiBorderPoints was called for a point that is out of original image boundaries!!\n"
                  << "\tcol: " << x << "\trow: " << y << std::endl;
        return PointPair();
    }

    //fetch & return the point pair:
    return this->pointData.at((y-1)*width + (x-1)); //x and y are 1-indexed, so fix them before fetching anything
}

int EpipolarGeometry::getOrigRowSize() const
{
    return this->height;
}

int EpipolarGeometry::getOrigColSize() const
{
    return this->width;
}

void EpipolarGeometry::initWithEpiLines(const char *csvFileName, int origHeight)
{
    //read the file -- it's a big one
    top(0);
    ifstream file;
    file.open(csvFileName);
    long lineCounter = 0;

    if (!file.is_open())
    {
        cerr << "ERROR Cannot open file: " << csvFileName << endl;
        return;
    }

    int y = -1, x = -1;

    while (file) //for each line
    {
        string line;
        if (!getline(file, line)) //get the line
            break;

        stringstream strStream(line);

        //we know the number of fields: 5 (int,int,double,double,double)
        double coefs[3] = {0.0, 0.0, 0.0};

        for (int i=0; i<5; i++) //for each comma separated field
        {
            string field;
            if (!getline(strStream, field, ','))
            {
                std::cerr << "ERROR Unexpected file format. Line: " << lineCounter << " in file: " << csvFileName << endl;
                break;
            }

            switch (i) {
            case 0:
                x = atoi(field.c_str());
                break;
            case 1:
                y = atoi(field.c_str());
                break;
            case 2:
                coefs[0] = atof(field.c_str());
                break;
            case 3:
                coefs[1] = atof(field.c_str());
                break;
            case 4:
                coefs[2] = atof(field.c_str());
                break;
            default:
                break;
            }
        }

        if (origHeight > 0) //don't bother if we don't know the size
        {
            // matlab indexing --> zero indexing
            y--;
            x--;

            if (lineCounter != x * origHeight + y)
                cerr << "ERROR lineCounter =" << lineCounter << " and col * origRowSize + row = " << x * origHeight + y << " are not equal.\n";
        }

        LineCoefs linecoefs;
        linecoefs.coefA = coefs[0];
        linecoefs.coefB = coefs[1];
        linecoefs.coefC = coefs[2];

        this->lineData[lineCounter] = linecoefs; //line counter should be equal to (col * rowsize + row)

        lineCounter++;
    }

    if (origHeight > 0) //rewind the row & col value if we fixed the indexing before
    {
        // zero indexing --> matlab indexing
        y--;
        x--;
    }
    //get the final row sizes
    this->height = y;
    this->width = x;

    FPTYPE duration = top(0);
    std::cout << "Epipolar geometry: CSV file with " << lineCounter << " lines is read in "<< duration/1000.0<< " seconds\n";

    file.close();
}

void EpipolarGeometry::initWithEpiBorderPoints(const char *csvFileName, int origHeight)
{
    //read the file -- it's a big one
    top(0);
    ifstream file;
    file.open(csvFileName);
    long lineCounter = 0;

    if (!file.is_open())
    {
        cerr << "ERROR Cannot open file: " << csvFileName << endl;
        return;
    }

    int y = -1, x = -1;
    PointPair ptPair;

    while (file) //for each line
    {
        string line;
        if (!getline(file, line)) //get the line
            break;

        stringstream strStream(line);

        //we know the number of fields: 6 (int,int,double,double,double,double)

        for (int i=0; i<6; i++) //for each comma separated field
        {
            string field;
            if (!getline(strStream, field, ','))
            {
                std::cerr << "ERROR Unexpected file format. Line: " << lineCounter << " in file: " << csvFileName << endl;
                break;
            }

            switch (i) {
            case 0:
                x = atoi(field.c_str());
                break;
            case 1:
                y = atoi(field.c_str());
                break;
            case 2:
                ptPair.first.x = atof(field.c_str());
                break;
            case 3:
                ptPair.first.y = atof(field.c_str());
                break;
            case 4:
                ptPair.second.x = atof(field.c_str());
                break;
            case 5:
                ptPair.second.y = atof(field.c_str());
                break;
            default:
                break;
            }
        }

        if (origHeight > 0) //don't bother if we know the size
        {
            // matlab indexing --> zero indexing
            y--;
            x--;

            if (lineCounter != x * origHeight + y)
                cerr << "ERROR lineCounter =" << lineCounter << " and col * origRowSize + row = " << x * origHeight + y << " are not equal.\n";
        }

        this->pointData[lineCounter] = ptPair; //line counter should be equal to (col * rowsize + row)

        lineCounter++;
    }

    if (origHeight > 0) //rewind the row & col value if we fixed the indexing before
    {
        // zero indexing --> matlab indexing
        y--;
        x--;
    }
    //get the final row sizes
    this->height = y;
    this->width = x;

    FPTYPE duration = top(0);
    std::cout << "Epipolar geometry: CSV file with " << lineCounter << " lines is read in "<< duration/1000.0<< " seconds\n";

    file.close();
}
