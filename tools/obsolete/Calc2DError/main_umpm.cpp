#include "umpm_pose.h"
#include "groundtruth_umpm.h"
#include "errorcalculation_umpm.h"
#include <QStringList>

#include <iostream>


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " GroundTruthFile PoseFile\n";
        std::cerr << "Example: " << argv[0] << " someFolder/p1_chair_2_f.csv someFolder/p1_chair_2_f-wrt-l/im0592.png_A_Pose_UMPM.txt\n";
        return -1;
    }

    bool ok;
    //fetch arguments
    QString gtPath = QString::fromStdString(argv[1]);
    QString posePath = QString::fromStdString(argv[2]);

    //fetch info of the pose we're looking for
    QStringList elems = posePath.split('/');
    QString poseFileName = elems.at(elems.length()-1);
//    QString sequenceName  = elems.at(elems.length()-2);

    elems = poseFileName.split('.');
    QString imgNo = elems.at(0);
    imgNo = imgNo.right(4); // always 4 digits for the frame number
    int frameNo = imgNo.toInt(&ok);
    if (!ok)
    {
        std::cerr << "Fatal: Could not parse frameNo from image name.\n";
        return -1;
    }

//    elems = sequenceName.split('_');
//    QString subject = elems.at(0);
//    QString action = elems.at(1);
//    QString trialStr = elems.at(2);
//    int trial = trialStr.toInt(&ok);

    //fetch view from ground truth text file


    Umpm_pose pose = Umpm_pose(posePath);
    GroundTruth_umpm grTruth(gtPath);

    Umpm_pose gtPose = grTruth.getGtPose(frameNo); //ground truth file is 0-indexed, frames are not BUT IT'S FIXED IN groundtruth_umpm.cpp

    //std::cout << "Ground truth:\n";
    //for (int i=0;i<NB_PARTS;i++)
    //    std::cout << gtPose.getPosition(i).x << "\t" << gtPose.getPosition(i).y << std::endl;

    //check if pose exists in the ground truth
    Position checkPose = gtPose.getPosition(0);
    if (checkPose.x < 0 && checkPose.y < 0)
    {
        std::cerr << "INVALID GROUND TRUTH POSE: (" << checkPose.x << ", " << checkPose.y << ")\n";
        return -1;
    }

    QList<double>eucDists = ErrorCalculation_umpm::calculateEuclidianDistance(gtPose, pose);
//    std::cout << "L2 Norm Errors for given pose -------------\n";
    ErrorCalculation_umpm::printErrors(eucDists);
    std::cout << "mean," << ErrorCalculation_umpm::calculateMeanError(eucDists) << std::endl << std::flush;
//    std::cout << "Variance: " << ErrorCalculation::calculateVariance(eucDists.values()) << "\n";
//    std::cout << "StdDev: " << ErrorCalculation::calculateStdDev(eucDists.values()) << "\n";


    return 0;
}

