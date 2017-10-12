#ifndef UMPM_POSE_H
#define UMPM_POSE_H

#define NB_PARTS 15

#include "structs.h"
#include <QString>

class Umpm_pose
{
public:
    // default constructor
    Umpm_pose();

    // default constructor
    Umpm_pose(QString poseFileStr);

    // checks the positionData and return false if all values are <0, otherwise it's considered valid
    bool isValid();

    // get position for a joint
    Position getPosition(int jIndex);

    // clears all position data
    void reset();

    // set a single element in position data
    void insertPositionData(int jIndex, double x, double y);


protected:
    Position positionData[NB_PARTS];


};

#endif // UMPM_POSE_H
