#include "he_pose.h"

#include <QFile>
#include <QTextStream>
#include <iostream>

void HE_Pose::initJointNames()
{
    this->jointNames.insert("torsoProximal", torsoProximal);
    this->jointNames.insert("torsoDistal", torsoDistal);
    this->jointNames.insert("upperLArmProximal", upperLArmProximal);
    this->jointNames.insert("upperLArmDistal", upperLArmDistal);
    this->jointNames.insert("lowerLArmProximal", lowerLArmProximal);
    this->jointNames.insert("lowerLArmDistal", lowerLArmDistal);
    this->jointNames.insert("upperRArmProximal", upperRArmProximal);
    this->jointNames.insert("upperRArmDistal", upperRArmDistal);
    this->jointNames.insert("lowerRArmProximal", lowerRArmProximal);
    this->jointNames.insert("lowerRArmDistal", lowerRArmDistal);
    this->jointNames.insert("upperLLegProximal", upperLLegProximal);
    this->jointNames.insert("upperLLegDistal", upperLLegDistal);
    this->jointNames.insert("lowerLLegProximal", lowerLLegProximal);
    this->jointNames.insert("lowerLLegDistal", lowerLLegDistal);
    this->jointNames.insert("upperRLegProximal", upperRLegProximal);
    this->jointNames.insert("upperRLegDistal", upperRLegDistal);
    this->jointNames.insert("lowerRLegProximal", lowerRLegProximal);
    this->jointNames.insert("lowerRLegDistal", lowerRLegDistal);
    this->jointNames.insert("headProximal", headProximal);
    this->jointNames.insert("headDistal", headDistal);
}

HE_Pose::HE_Pose()
{
    //positionData is empty
    this->initJointNames();
}

HE_Pose::HE_Pose(QString poseFileStr)
{
    this->initJointNames();
    // Open the file
    QFile poseFile(poseFileStr);

    if (!poseFile.open(QIODevice::ReadOnly))
    {
        std::cerr << "Could not open file: " << poseFileStr.toStdString() << "\n\n";

        return;
    }

    QTextStream in(&poseFile);
    bool ok;

    /* File should have the following format, with 20 lines:
        torsoProximal:311.627,141.921
        torsoDistal:356.254,243.745
        upperLArmProximal:334.254,153.235
        ...

    */

    // Parse the file
    int lineCounter = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();

        QStringList lineParts = line.split(':');

        // check for the joint name

        if (lineParts.first().compare(this->jointNames.key((JointIndex)lineCounter)) != 0)
        {
            std::cerr << "Unexpected joint name in line: " << lineCounter << "\n"
                      << "\tExpected:\t" << this->jointNames.key((JointIndex)lineCounter).toStdString() << "\n"
                      << "\tGot:\t" << lineParts.first().toStdString() << "\n\n";
            return;
        }

        QStringList posValues = lineParts[1].split(',');
        double tempX = posValues[0].toDouble(&ok);
        if (!ok){std::cerr << "Cannot parse X value in line " << lineCounter << "\n\n"; return;}
        double tempY = posValues[1].toDouble(&ok);
        if (!ok){std::cerr << "Cannot parse Y value in line " << lineCounter << "\n\n"; return;}
        this->positionData[lineCounter] = Position(tempX, tempY);

        lineCounter++;
    }

    poseFile.close();

}

bool HE_Pose::isValid()
{
    for (int i=0; i<20; i++)
        if (positionData[i].x < 0 && positionData[i].y < 0)
            return false;

    return true;
}

Position HE_Pose::getPosition(JointIndex jIndex)
{
    return this->positionData[(int)jIndex];
}

void HE_Pose::reset()
{
    //fills the array with invalid positions
    std::fill(this->positionData, this->positionData+20, Position());
}

void HE_Pose::insertPositionData(QString jointName, double x, double y)
{
    this->positionData[(int)this->getJointIndex(jointName)] = Position(x, y);
}

void HE_Pose::insertPositionData(JointIndex jIndex, double x, double y)
{
    this->positionData[(int)jIndex] = Position(x, y);
}




