#ifndef GROUNDTRUTH_UMPM_H
#define GROUNDTRUTH_UMPM_H

#include <QString>
#include <QHash>
#include "umpm_pose.h"

class GroundTruth_umpm
{
public:
    GroundTruth_umpm(QString filePath);

    Umpm_pose getGtPose(int frameIdx);

protected:
    QHash<int, Umpm_pose> poseData;
    int totalFrames;
};

#endif // GROUNDTRUTH_UMPM_H
