#ifndef GROUNDTRUTH_H
#define GROUNDTRUTH_H

#include <he_pose.h>
#include <QString>
#include <QHash>

class Sequence
{
public:
    // constructors
    Sequence();
    Sequence(int id, QString subject, QString action, int trial);

    // a sequence is empty if its data is empty
    bool isEmpty(){return poseData.isEmpty();}

    // get frame info
    QList<int> getFrameIndices();


    // push pose data to the sequence
    void insertPose(int frameNo, HE_Pose pose);

    // get pose
    HE_Pose getPose(int frameNo) const;

    // properties
    int id;
    QString subject;
    QString action;
    int trial;

private:
    QHash<int, HE_Pose> poseData; //int key is the frame index
};

class GroundTruth
{
public:
    GroundTruth(QString fileName, QString viewName);

    QString view;

    HE_Pose getPose(QString subject, QString action, int trial, int frame) const;

    QStringList getSequenceKeys();
    Sequence& getSequence(QString seqKey);

private:
    QHash<QString, Sequence> sequenceData; // QString key is the standardized key that contains subect-action-trial name
    QString generateSeqKey(QString subject, QString action, int trial) const;

};



#endif // GROUNDTRUTH_H
