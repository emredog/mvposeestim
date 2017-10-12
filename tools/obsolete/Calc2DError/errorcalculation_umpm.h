#ifndef ERRORCALCULATION_UMPM_H
#define ERRORCALCULATION_UMPM_H

#include "umpm_pose.h"
#include <QList>

class ErrorCalculation_umpm
{
public:
    //calculates parts-based euclidian distance between the given poses
    static QList<double> calculateEuclidianDistance(Umpm_pose gtPose, Umpm_pose targetPose);
    //calculates relative L2 Norm error
    static QList<double> calculateRelativeL2Norm(Umpm_pose gtPose, Umpm_pose targetPose);

    //calculates mean and standart deviation
    static double calculateMeanError(QList<double> allErrors);
    static double calculateVariance(QList<double> allErrors);
    static double calculateStdDev(QList<double> allErrors);

    static void printErrors(QList<double> eucDistances);
protected:
    ErrorCalculation_umpm();
};

#endif // ERRORCALCULATION_UMPM_H
