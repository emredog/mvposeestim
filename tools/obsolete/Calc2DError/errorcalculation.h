#ifndef ERRORCALCULATION_H
#define ERRORCALCULATION_H

#include "he_pose.h"

class ErrorCalculation
{
public:
    //calculates parts-based euclidian distance between the given poses
    static QHash<JointIndex, double> calculateEuclidianDistance(HE_Pose gtPose, HE_Pose targetPose);
    //calculates relative L2 Norm error
    static QHash<JointIndex, double> calculateRelativeL2Norm(HE_Pose gtPose, HE_Pose targetPose);

    //calculates mean and standart deviation
    static double calculateMeanError(QList<double> allErrors);
    static double calculateVariance(QList<double> allErrors);
    static double calculateStdDev(QList<double> allErrors);



    static void printErrors(QHash<JointIndex, double> eucDistances);

protected:
    ErrorCalculation();

};

#endif // ERRORCALCULATION_H
