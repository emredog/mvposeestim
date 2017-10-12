#include "groundtruth_umpm.h"
#include <QFile>
#include <QTextStream>
#include <iostream>

GroundTruth_umpm::GroundTruth_umpm(QString filePath)
{
    // ~~~CAUTION: gt file has joint names and its own ordering of joints
    // ~~~while pose estation result has a different ordering for joints
    // ~~~this difference it is corrected here.

    // NO THAT's OBSOLETE. DONT NEED TO DO THAT ANYMORE (with transback function etc)
    // so replacing with original indices

    QHash<QString, int> jointToCorrectIndex;
    jointToCorrectIndex.insert(":pelvis",       0);
    jointToCorrectIndex.insert(":thorax",       1);
    jointToCorrectIndex.insert(":head",         2);
    jointToCorrectIndex.insert(":rshoulder",    3);
    jointToCorrectIndex.insert(":relbow",       4);
    jointToCorrectIndex.insert(":rwrist",       5);
    jointToCorrectIndex.insert(":lshoulder",    6);
    jointToCorrectIndex.insert(":lelbow",       7);
    jointToCorrectIndex.insert(":lwrist",       8);
    jointToCorrectIndex.insert(":rhip",         9);
    jointToCorrectIndex.insert(":lhip",         10);
    jointToCorrectIndex.insert(":rknee",        11);
    jointToCorrectIndex.insert(":lknee",        12);
    jointToCorrectIndex.insert(":rankle",       13);
    jointToCorrectIndex.insert(":lankle",       14);

    // open the file
    QFile gtFile(filePath);
    if (!gtFile.open(QIODevice::ReadOnly))
    {
        std::cerr << "Could not open file: " << filePath.toStdString() << "\n\n";

        return;
    }

    //some vars
    QTextStream in(&gtFile);
    bool ok;
    int lineCounter = 0;
    Umpm_pose tempPose;
    int frame = -1;
    int prevFrame = -1;
    double tempX = -1.0, tempY = -1.0;

    // start parsing the huge file (~4mb)
    while(!in.atEnd())
    {
        QString line = in.readLine();
        if (lineCounter == 0) // just have read the header line, skip.
        {
            lineCounter++;
            continue;
        }

        // file format:
//        0,:pelvis   ,231.1672,218.7656
//        0,:thorax   ,230.8904,116.6917
//        0,:head     ,230.9666,85.0523
//        0,:rshoulder,250.8489,125.7024
//        0,:relbow   ,301.0986,146.3393
//        0,:rwrist   ,297.4608,98.2197
//        0,:lshoulder,206.8848,122.3771
//        0,:lelbow   ,226.4807,135.1654
//        0,:lwrist   ,234.0712,98.7036
//        0,:rhip     ,246.0635,223.5147
//        0,:lhip     ,217.1148,214.279
//        0,:rknee    ,279.9511,287.3507
//        0,:lknee    ,230.6753,274.1482
//        0,:rankle   ,271.8957,341.3143
//        0,:lankle   ,221.881,319.3998
//        1,:pelvis   ,231.1561,218.8272
//        ...

        QStringList lineParts = line.split(',');
        frame = lineParts.at(0).toInt(&ok);
        if (!ok){std::cerr << "Cannot parse FRAME value in line " << lineCounter << "\n\n"; return;}

        //===== IMPORTANT =====
        // skip every odd frame, and save the pose data with correct frame index
        // because images are 50fps but groung truth is 100fps
        if (frame % 2 == 1)
            continue;

        int correctFrameIdx = (frame/2) + 1;
//        std::cout << "GT Index: " << frame << "\tCorrected index: " << correctFrameIdx << std::endl;

        QString jointName = lineParts.at(1).trimmed();

        if (frame != prevFrame && prevFrame >= 0) // save the old pose and switch to a new one
        {
            this->poseData[correctFrameIdx] = tempPose;
            tempPose.reset();
        }

        int partIndex = jointToCorrectIndex[jointName];

        tempX = lineParts.at(2).toDouble(&ok);
        if (!ok){std::cerr << "Cannot parse X value in line " << lineCounter << "\n\n"; return;}
        tempY = lineParts.at(3).toDouble(&ok);
        if (!ok){std::cerr << "Cannot parse Y value in line " << lineCounter << "\n\n"; return;}

        tempPose.insertPositionData(partIndex, tempX, tempY);

        lineCounter++;
        prevFrame = frame;
    }

    this->totalFrames = frame;
//    std::cout << "GT file is parsed. Obtained poses: " << this->totalFrames << std::endl;
}

Umpm_pose GroundTruth_umpm::getGtPose(int frameIdx)
{
//    std::cout << "Fetching pose for " << frameIdx << std::endl;
    if (frameIdx > this->totalFrames || frameIdx < 0)
    {
        std::cerr << "Frame " << frameIdx << " out of range: " << this->totalFrames << std::endl;
        return Umpm_pose();
    }

    return this->poseData[frameIdx];
}

