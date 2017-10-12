#include "groundtruth.h"

#include <QFile>
#include <QTextStream>
#include <iostream>

#include <iostream>

GroundTruth::GroundTruth(QString fileName, QString viewName)
{
    this->view = viewName;

    // open the file
    QFile gtFile(fileName);
    if (!gtFile.open(QIODevice::ReadOnly))
    {
        std::cerr << "Could not open file: " << fileName.toStdString() << "\n\n";

        return;
    }

    //some vars
    QTextStream in(&gtFile);
    bool ok;
    int seqNo;
    QString subject;
    QString action;
    int trial;
    int lineCounter = 0;
    int invalidCounter = 0;
    Sequence tempSeq;
    HE_Pose tempPose;
    int frameNo = -1;

    // start parsing the huge file (~4mb)
    while(!in.atEnd())
    {
        QString line = in.readLine();
        lineCounter++;


        //=============== Actual position data ====================
        if (line.startsWith("torso") || line.startsWith("upper") || line.startsWith("lower") || line.startsWith("head"))
        {
            QStringList lineParts = line.split(':');

            // check for the joint name
            JointIndex jIndex = tempPose.getJointIndex(lineParts.first());
            if ((int)jIndex < 0)
            {
                std::cerr << "Unexpected joint name in line: " << lineCounter << "\n"
                          << "\tGot:\t" << lineParts.first().toStdString() << "\n\n";
                return;
            }

            QStringList posValues = lineParts[1].split(',');
            double tempX = posValues[0].toDouble(&ok);
            if (!ok){std::cerr << "Cannot parse X value in line " << lineCounter << "\n\n"; return;}
            double tempY = posValues[1].toDouble(&ok);
            if (!ok){std::cerr << "Cannot parse Y value in line " << lineCounter << "\n\n"; return;}

            //we've read two values ok
            tempPose.insertPositionData(jIndex, tempX, tempY);
            continue;
        }

        if (line.startsWith("INVALID_POSE"))
        {
            invalidCounter++;
            continue;
        }


        // ========= Basic info that updates every sequence ==========
        // sequence number
        if (line.startsWith("Loading")) //--> "Loading sequence n"
        {
            QStringList lineParts = line.split(' ');
            seqNo = lineParts.last().toInt(&ok);
            tempSeq.id = seqNo;
            if (!ok) {std::cerr << "Cannot parse SEQUENCE NUMBER in line " << lineCounter << "\n\n"; return;}
            continue;
        }

        // subject
        if (line.startsWith("Subject")) //--> "Subject: S1"
        {
            QStringList lineParts = line.split(' ');
            subject = lineParts.last();
            tempSeq.subject = subject;
            continue;
        }

        // action
        if (line.startsWith("Action")) //--> "Action: Walking"
        {
            QStringList lineParts = line.split(' ');
            action = lineParts.last();
            tempSeq.action = action;
            continue;
        }

        // trial
        if (line.startsWith("Trial")) //--> "Trial: 1"
        {
            //there's a case like "empty sequence" where it reads: "Trial:3 is not available."
            if (line.compare("Trial:3 is not available.") == 0) // FIXME: is there any other line that matches this case?
                continue; //we just skip it (next line is always "====" so it will reset everything

            QStringList lineParts = line.split(' ');
            trial = lineParts.last().toInt(&ok);
            if (!ok) {std::cerr << "Cannot parse TRIAL NUMBER in line " << lineCounter << "\n\n"; return;}
            tempSeq.trial = trial;
            continue;
        }

        // camera view --> it's assumed that we already know it.
        if (line.startsWith("Camera"))
        {
            continue;
        }


        // =================== Sequence Completed ==================
        if (line.startsWith("==="))
        {            
            if (!tempSeq.isEmpty()) // if we have data for the sequence
            {
                //save the sequence, by pushing it to the groundtruth object
                this->sequenceData.insert(this->generateSeqKey(tempSeq.subject, tempSeq.action, tempSeq.trial), tempSeq);
                // and reset tempSeq
                tempSeq = Sequence();
            }

            //and reset all vars
            seqNo = -1;  subject = ""; action = ""; trial = -1; frameNo = -1;
            continue;
        }

        // ================ Frame Related ================
        if (line.startsWith("---")) // --> end/start of a HE_pose
        {
            if (tempPose.isValid()) // if it's valid (filled with data)
            {
                tempSeq.insertPose(frameNo, tempPose);
            }
            tempPose = HE_Pose();
            frameNo = -1;
            continue;
        }

        if (line.startsWith("FRAME")) // fetch the frame number for the HE_Pose
        {
            QStringList lineParts = line.split(' ');
            frameNo = lineParts.at(1).toInt(&ok);
            if (!ok) {std::cerr << "Cannot parse FRAME NUMBER in line " << lineCounter << "\n\n"; return;}
            continue;
        }
    }

    //fetch the last sequence
    //int lastSeqNo = this->sequenceData.end().value().id; FIXME : it prints 64????

//    std::cout << "Ground truth fetched for view: " << this->view.toStdString() << "\n" <<
//                 "\tTotal lines:\t" << lineCounter << "\n" <<
//                 "\tTotal Sequences:\t" << this->sequenceData.count() /*<< " of " << lastSeqNo */<< "\n" <<
//                 "\tInvalid Poses:\t" << invalidCounter << "\n";
}

HE_Pose GroundTruth::getPose(QString subject, QString action, int trial, int frame) const
{
    return this->sequenceData.value(this->generateSeqKey(subject, action, trial)).getPose(frame);
}

QStringList GroundTruth::getSequenceKeys()
{
    return this->sequenceData.keys();
}

Sequence &GroundTruth::getSequence(QString seqKey)
{
    return this->sequenceData[seqKey];
}

QString GroundTruth::generateSeqKey(QString subject, QString action, int trial) const
{
    return QString("%1_%2_%3").arg(subject).arg(action).arg(trial);
}

Sequence::Sequence()
{

}

Sequence::Sequence(int id, QString subject, QString action, int trial)
{
    this->id = id;
    this->subject = subject;
    this->action = action;
    this->trial = trial;
}

QList<int> Sequence::getFrameIndices()
{
    return this->poseData.keys();
}

void Sequence::insertPose(int frameNo, HE_Pose pose)
{
    this->poseData.insert(frameNo, pose);
}

HE_Pose Sequence::getPose(int frameNo) const
{
    return this->poseData.value(frameNo);
}
