#ifndef HE_POSE_H
#define HE_POSE_H

#include <QString>
#include <QHash>

#include "structs.h"

enum JointIndex
{
    torsoProximal = 0,
    torsoDistal,
    upperLArmProximal,
    upperLArmDistal,
    lowerLArmProximal,
    lowerLArmDistal,
    upperRArmProximal,
    upperRArmDistal,
    lowerRArmProximal,
    lowerRArmDistal,
    upperLLegProximal,
    upperLLegDistal,
    lowerLLegProximal,
    lowerLLegDistal,
    upperRLegProximal,
    upperRLegDistal,
    lowerRLegProximal,
    lowerRLegDistal,
    headProximal,
    headDistal
};

class HE_Pose
{
public:
    //default constructor (positionData is empty, and by default it should be invalid)
    HE_Pose();

    //constructor from a converted HE pose file (a file with *_HE.txt format)
    HE_Pose(QString poseFileStr);

    // checks the positionData and return false if all values are <0, otherwise it's considered valid
    bool isValid();

    // get the joint index for pushing joint data from outside of the class
    JointIndex getJointIndex(QString jointName){return jointNames.value(jointName, (JointIndex)-1);}
    // get joint name from the joint index
    QString getJointName(JointIndex jIndex){return jointNames.key(jIndex);}

    Position getPosition(JointIndex jIndex);

    // clears all position data
    void reset();

    // set a single element in position data
    void insertPositionData(QString jointName, double x, double y);
    // set a single element in position data --> should be a little faster
    void insertPositionData(JointIndex jIndex, double x, double y);

protected:
    Position positionData[20];

private:
    QHash<QString, JointIndex> jointNames;
    void initJointNames();
};

#endif // HE_POSE_H
