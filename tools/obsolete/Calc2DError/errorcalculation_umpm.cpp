#include "errorcalculation_umpm.h"

#include "math.h"
#include <numeric>
#include <float.h>
#include <iostream>

ErrorCalculation_umpm::ErrorCalculation_umpm()
{

}

QList<double> ErrorCalculation_umpm::calculateEuclidianDistance(Umpm_pose gtPose, Umpm_pose targetPose)
{
    QList<double> eucDistances;

    for (int j = 0; j < NB_PARTS; j++)
    {
        //get positions
        double gtX = gtPose.getPosition(j).x;
        double gtY = gtPose.getPosition(j).y;
        double trgtX = targetPose.getPosition(j).x;
        double trgtY = targetPose.getPosition(j).y;

        //calculate euclidian distance:
        double eucDist = sqrt((gtX - trgtX)*(gtX - trgtX) + (gtY - trgtY)*(gtY - trgtY));
        eucDistances.insert(j, eucDist);
    }

    return eucDistances;
}

QList<double> ErrorCalculation_umpm::calculateRelativeL2Norm(Umpm_pose gtPose, Umpm_pose targetPose)
{
    QList<double> relL2Norms;

    for (int j = 0; j < NB_PARTS; j++)
    {
        //get positions
        double gtX = gtPose.getPosition(j).x;
        double gtY = gtPose.getPosition(j).y;
        double trgtX = targetPose.getPosition(j).x;
        double trgtY = targetPose.getPosition(j).y;

        //calculate euclidian distance:
        double relL2Norm = (sqrt((gtX - trgtX)*(gtX - trgtX) + (gtY - trgtY)*(gtY - trgtY))) / sqrt(gtX*gtX + gtY*gtY);
        relL2Norms.insert(j, relL2Norm);
    }

    return relL2Norms;
}

double ErrorCalculation_umpm::calculateMeanError(QList<double> allErrors)
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

double ErrorCalculation_umpm::calculateVariance(QList<double> allErrors)
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

double ErrorCalculation_umpm::calculateStdDev(QList<double> allErrors)
{
    if (allErrors.size() == 0)
        return -DBL_MAX;

    return sqrt(calculateVariance(allErrors));
}

void ErrorCalculation_umpm::printErrors(QList<double> eucDistances)
{    
    for (int j = 0; j < NB_PARTS; j++)
    {
        std::cout << (j+1) << "," << eucDistances.value(j) << "\n";
    }
}

