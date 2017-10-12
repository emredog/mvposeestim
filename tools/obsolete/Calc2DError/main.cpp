#include "he_pose.h"
#include "groundtruth.h"
#include "errorcalculation.h"
#include <QStringList>

#include <iostream>


int main(int argc, char *argv[])
{   
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " GroundTruthFile PoseFile\n";
        std::cerr << "Example: " << argv[0] << " someFolder/Train_C2.txt someFolder/S1_Walking_1_C2-C3/im0592.bmp_A_Pose_HE.txt\n";
        return -1;
    }

    bool ok;
    //fetch arguments
    QString gtPath = QString::fromStdString(argv[1]);
    QString posePath = QString::fromStdString(argv[2]);

    //fetch info of the pose we're looking for
    QStringList elems = posePath.split('/');
    QString poseFileName = elems.at(elems.length()-1);
    QString sequenceName  = elems.at(elems.length()-2);

    elems = poseFileName.split('.');
    QString imgNo = elems.at(0);
    imgNo = imgNo.right(4); // always 4 digits for the frame number
    int frameNo = imgNo.toInt(&ok);

    elems = sequenceName.split('_');
    QString subject = elems.at(0);
    QString action = elems.at(1);
    QString trialStr = elems.at(2);
    int trial = trialStr.toInt(&ok);

    //fetch view from ground truth text file
    QString view;
    {
        elems = gtPath.split('/');
        QString gtFileName = elems.last();
        int indUnderscore = gtFileName.lastIndexOf('_');
        int dot = gtFileName.lastIndexOf('.');
        view = gtFileName.mid(indUnderscore+1, dot-indUnderscore-1);
    }

    HE_Pose pose = HE_Pose(posePath);
    GroundTruth grTruth(gtPath, view);

    HE_Pose gtPose = grTruth.getPose(subject, action, trial, frameNo);

    //check if pose exists in the ground truth
    Position checkPose = gtPose.getPosition(torsoProximal);
    if (checkPose.x < 0 && checkPose.y < 0)
    {
        std::cout << "FAILED TO RETRIEVE GROUND TRUTH for:" << std::endl
                  << "\tview: " << view.toStdString() << std::endl
                  << "\tsubject: " << subject.toStdString() << std::endl
                  << "\taction: " << action.toStdString() << std::endl
                  << "\ttrial: " << trial << std::endl
                  << "\tframe: " << frameNo << std::endl;
        return 0;
    }

    QHash<JointIndex, double>eucDists = ErrorCalculation::calculateEuclidianDistance(gtPose, pose);
//    std::cout << "L2 Norm Errors for MultiView implementation-------------\n";
    ErrorCalculation::printErrors(eucDists);
    std::cout << "mean:" << ErrorCalculation::calculateMeanError(eucDists.values()) << std::endl << std::flush;
//    std::cout << "Variance: " << ErrorCalculation::calculateVariance(eucDists.values()) << "\n";
//    std::cout << "StdDev: " << ErrorCalculation::calculateStdDev(eucDists.values()) << "\n";


    return 0;
}
