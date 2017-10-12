#include "errorcalculation.h"
#include "math.h"

#include <iostream>
#include <numeric>
#include <float.h>

ErrorCalculation::ErrorCalculation()
{
}

QHash<JointIndex, double> ErrorCalculation::calculateEuclidianDistance(HE_Pose gtPose, HE_Pose targetPose)
{
    QHash<JointIndex, double> eucDistances;

    for (int j = (int)torsoProximal; j <= (int)headDistal; j++)
    {
        //get positions
        double gtX = gtPose.getPosition((JointIndex)j).x;
        double gtY = gtPose.getPosition((JointIndex)j).y;
        double trgtX = targetPose.getPosition((JointIndex)j).x;
        double trgtY = targetPose.getPosition((JointIndex)j).y;

        //calculate euclidian distance:
        double eucDist = sqrt((gtX - trgtX)*(gtX - trgtX) + (gtY - trgtY)*(gtY - trgtY));
        eucDistances.insert((JointIndex)j, eucDist);
    }

    return eucDistances;
}

QHash<JointIndex, double> ErrorCalculation::calculateRelativeL2Norm(HE_Pose gtPose, HE_Pose targetPose)
{
    QHash<JointIndex, double> relL2Norms;

    for (int j = (int)torsoProximal; j <= (int)headDistal; j++)
    {
        //get positions
        double gtX = gtPose.getPosition((JointIndex)j).x;
        double gtY = gtPose.getPosition((JointIndex)j).y;
        double trgtX = targetPose.getPosition((JointIndex)j).x;
        double trgtY = targetPose.getPosition((JointIndex)j).y;

        //calculate euclidian distance:
        double relL2Norm = (sqrt((gtX - trgtX)*(gtX - trgtX) + (gtY - trgtY)*(gtY - trgtY))) / sqrt(gtX*gtX + gtY*gtY);
        relL2Norms.insert((JointIndex)j, relL2Norm);
    }

    return relL2Norms;
}

double ErrorCalculation::calculateMeanError(QList<double> allErrors)
{
    if (allErrors.size() == 0)
        return -DBL_MAX;

    double totalError = 0.0;

    foreach (double val, allErrors)
    {
        totalError += val;
    }

    return (totalError / (double)allErrors.size());
}

double ErrorCalculation::calculateVariance(QList<double> allErrors)
{
    if (allErrors.size() == 0)
        return -DBL_MAX;

    double mean = calculateMeanError(allErrors);

    double sum = 0.0;

    foreach(double val, allErrors)
    {
        sum += (val - mean)*(val - mean);
    }

    return (sum / (double)allErrors.size());
}

double ErrorCalculation::calculateStdDev(QList<double> allErrors)
{
    if (allErrors.size() == 0)
        return -DBL_MAX;

    return sqrt(calculateVariance(allErrors));
}

void ErrorCalculation::printErrors(QHash<JointIndex, double> eucDistances)
{
    HE_Pose dummyPose;

    for (int j = (int)torsoProximal; j <= (int)headDistal; j++)
    {
        std::cout << dummyPose.getJointName((JointIndex)j).toStdString() << "\t\t" << eucDistances.value((JointIndex)j) << "\n";
    }

    std::cout << std::endl;
}
