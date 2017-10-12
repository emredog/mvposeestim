#include <QCoreApplication>

#include "../Calc2DError/groundtruth.h"
#include "../Calc2DError/he_pose.h"

#include <iostream>
#include <QFile>
#include <QTextStream>

inline Position calculateMidPoint(Position a, Position b, double ratio = 0.5)
{
    double xDist = a.x - b.x;
    double yDist = a.y - b.y;

    return Position (b.x + xDist*ratio, b.y + yDist*ratio);
}

void printGroundTruth_HE_CSV(GroundTruth gt, QString fileName)
{
    QStringList seqKeys = gt.getSequenceKeys();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite))
    {
        std::cerr <<  "COULD NOT OPEN FILE TO WRITE: " << fileName.toStdString() << std::endl << std::endl;
        return;
    }


    QTextStream stream( &file );

    // Write header
    stream << "Subject,Action,Trial,Frame,JointName,X,Y\n";



    foreach(QString seqKey, seqKeys)
    {
        Sequence seq = gt.getSequence(seqKey);

        QString subject = seq.subject;
        QString action = seq.action;
        int trial = seq.trial;
        if (trial != 1)
            continue;

        QList<int> frameIndices = seq.getFrameIndices();
        int min = INT32_MAX;
        int max = INT32_MIN;

        foreach(int frame, frameIndices)
        {
            if (frame < min) min = frame;
            if (frame > max) max = frame;
            HE_Pose pose = gt.getPose(subject, action, trial, frame);

            for (int i=torsoProximal; i<=headDistal; i++)
            {
                // begin writing
                stream << subject << "," << action << "," << trial << "," << frame << ",";
                stream << pose.getJointName((JointIndex)i) << ","; // write joint name
                Position p = pose.getPosition((JointIndex)i);
                stream << p.x << "," << p.y << "\n";
            }
        }
        std::cout << "Sequence completed: " << subject.toStdString() << " - " << action.toStdString() << " - " << trial << " [" << min << ", " << max << "]\n";
        stream.flush();
    }

    std::cout << "All sequences completed.\n";
    file.close();
}


void printGroundTruth_YR_CSV(GroundTruth gt, QString fileName)
{
    QStringList seqKeys = gt.getSequenceKeys();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite))
    {
        std::cerr <<  "COULD NOT OPEN FILE TO WRITE: " << fileName.toStdString() << std::endl << std::endl;
        return;
    }


    QTextStream stream( &file );

    // Write header
    stream << "Subject,Action,Trial,Frame,JointIndex,X,Y\n";

    foreach(QString seqKey, seqKeys)
    {
        Sequence seq = gt.getSequence(seqKey);

        QString subject = seq.subject;
        QString action = seq.action;
        int trial = seq.trial;
        if (trial != 1)
            continue;

        QList<int> frameIndices = seq.getFrameIndices();
        int min = INT32_MAX;
        int max = INT32_MIN;

        foreach(int frame, frameIndices)
        {
            if (frame < min) min = frame;
            if (frame > max) max = frame;
            HE_Pose pose = gt.getPose(subject, action, trial, frame);
            Position tempPosition;

            // write redundant data + joint data for all joint
            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)19);
            stream << 1 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)0);
            stream << 2 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)6);
            stream << 3 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)7), pose.getPosition((JointIndex)6), 0.1);
            stream << 4 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)7);
            stream << 5 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)7), pose.getPosition((JointIndex)9));
            stream << 6 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)9);
            stream << 7 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)14), pose.getPosition((JointIndex)6), 1.0/3.0);
            stream << 8 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)14), pose.getPosition((JointIndex)6), 2.0/3.0);
            stream << 9 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)14);
            stream << 10 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)15), pose.getPosition((JointIndex)14), 0.2);
            stream << 11 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)15);
            stream << 12 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)15), pose.getPosition((JointIndex)17));
            stream << 13 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)17);
            stream << 14 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)2);
            stream << 15 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)3), pose.getPosition((JointIndex)2), 0.1);
            stream << 16 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)3);
            stream << 17 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)3), pose.getPosition((JointIndex)5));
            stream << 18 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)5);
            stream << 19 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)10), pose.getPosition((JointIndex)2), 1.0/3.0);
            stream << 20 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)10), pose.getPosition((JointIndex)2), 2.0/3.0);
            stream << 21 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)10);
            stream << 22 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)11), pose.getPosition((JointIndex)10), 0.2);
            stream << 23 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)11);
            stream << 24 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = calculateMidPoint(pose.getPosition((JointIndex)11), pose.getPosition((JointIndex)13));
            stream << 25 << "," << tempPosition.x << "," << tempPosition.y << "\n";

            stream << subject << "," << action << "," << trial << "," << frame << ",";
            tempPosition = pose.getPosition((JointIndex)13);
            stream << 26 << "," << tempPosition.x << "," << tempPosition.y << "\n";
        }
        std::cout << "Sequence completed: " << subject.toStdString() << " - " << action.toStdString() << " - " << trial << " [" << min << ", " << max << "]\n";
        stream.flush();
    }

    std::cout << "All sequences completed.\n";
    file.close();
}
int main(int argc, char *argv[])
{
    if (argc !=3)
    {
        std::cerr << "Usage: " << argv[0] << " <GroundTruthFile> <HE/YR>\n";
        std::cerr << "Example: " << argv[0] << " someFolder/Train_C2.txt HE\n";
        return -1;
    }

    //fetch the argument
    QString gtPath = QString::fromStdString(argv[1]);

    bool isHE;
    QString heORyr = QString::fromStdString(argv[2]);
    if (heORyr.compare("he", Qt::CaseInsensitive) == 0)
        isHE = true;
    else if (heORyr.compare("yr", Qt::CaseInsensitive) == 0)
        isHE = false;
    else
    {
        std::cerr << "Could not parse 3rd parameter. Please enter HE or YR (case insensitive) to set output to HumanEva or Yang&Ramanan model.\n";
        return -1;
    }

    //fetch the view name "somepath/Train_C1.txt"
    QString view;
    {
        QString fileName = gtPath.split("/").last();
        fileName = fileName.split(".").first();
        view = fileName.split("_").last();
    }

    GroundTruth gTruth(gtPath, view);

    QString newFileName = gtPath;
    newFileName.chop(4);

    if (isHE)
    {
        newFileName += "_HE.csv";
        printGroundTruth_HE_CSV(gTruth, newFileName);
    }
    else
    {
        newFileName += "_YR.csv";
        printGroundTruth_YR_CSV(gTruth, newFileName);
    }




}

