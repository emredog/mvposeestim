#include "umpm_pose.h"

#include <QFile>
#include <QTextStream>
#include <iostream>

Umpm_pose::Umpm_pose()
{
    this->reset();
}

Umpm_pose::Umpm_pose(QString poseFileStr)
{
    // Open the file
    QFile poseFile(poseFileStr);

    if (!poseFile.open(QIODevice::ReadOnly))
    {
        std::cerr << "Could not open file: " << poseFileStr.toStdString() << "\n\n";

        return;
    }

    QTextStream in(&poseFile);
    bool ok;

    /* File should have the following format, with 15 lines:
        1,123.123,234.234
        2,123.123,234.234
        ...
        15,123.123,234.234

    */


    // ========================================================
    // ASSUMPTION: USE_NMS=NO --> SINGLE DETECTION IN EACH FILE
    // ========================================================

    for (int l=0; l< NB_PARTS; l++)
    {
        QString line = in.readLine();

        QStringList lineParts = line.split(',');

        //ASSUMPTION: will always ordered and complete lines for each part

        double tempX = lineParts.at(1).toDouble(&ok);
        if (!ok){std::cerr << "Cannot parse X value in line " << l << "\n\n"; return;}
        double tempY = lineParts.at(2).toDouble(&ok);
        if (!ok){std::cerr << "Cannot parse Y value in line " << l << "\n\n"; return;}
        this->positionData[l] = Position(tempX, tempY);
    }

    poseFile.close();
}

bool Umpm_pose::isValid()
{
    for (int i=0; i<NB_PARTS; i++)
        if (positionData[i].x < 0 && positionData[i].y < 0)
            return false;

    return true;
}

Position Umpm_pose::getPosition(int jIndex)
{
    if (jIndex < NB_PARTS)
        return this->positionData[jIndex];
    else
    {
        std::cerr << "Invalid joint requested: " << jIndex << std::endl;
        return Position();
    }
}

void Umpm_pose::reset()
{
    //fills the array with invalid positions
    std::fill(this->positionData, this->positionData+NB_PARTS, Position());
}

void Umpm_pose::insertPositionData(int jIndex, double x, double y)
{
    if (jIndex < NB_PARTS)
        this->positionData[jIndex] = Position(x, y);
    else
        std::cout << "WARNING: jIndex " << jIndex << " is out of range " << NB_PARTS << std::endl;
}

