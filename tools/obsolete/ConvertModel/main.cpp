#include <QString>
#include <QFile>
#include <iostream>
#include <QMap>
#include <QTextStream>
#include <cmath>

//#define HUMANEVA
#define UMPM

#ifdef UMPM
#define NB_PARTS 15
#endif

struct Position
{
    double x;
    double y;
};
#ifdef HUMANEVA
QMap<int, QString> initHumanEvaNames();
#endif

void midPoint(double x1, double y1, double x2, double y2, double &midX, double &midY);
bool writeAsText(QMap<int, Position> partPositions, QString outFileName);
bool writeAsXml(QMap<int, Position> partPositions, QString outFileName);

// DONT USE THIS
// Use Yang-20121128-pose-release-ver1.3/code-full/{HumanEva_trans.m, UMPM_trans.m, HumanEva_transback.m, UMPM_transback.m} instead
// int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\n\tConvertModel <inputFile>\n";
        return -1;
    }

    //input file
    QString inFileName(argv[1]);
    QFile inFile(inFileName);

    if (!inFile.open(QIODevice::ReadOnly))
    {
        std::cerr << "Could not open file: " << inFileName.toStdString() << "\n\n";
    }


    // -------------------------------------------------------
    //  PARSING THE YR POSE FILE
    // -------------------------------------------------------
    QTextStream in(&inFile);
    QList<QMap<int, Position> > detectionPartPositions;
    QList<double> detectionScores;
    int detectionCount = 0;

    QString line;
    bool ok;
    while (!in.atEnd())
    {
        line = in.readLine();
        if (line.startsWith("detection"))
        {
            //fetch score
            QStringList parts = line.split(':');
            double score = parts.last().toDouble(&ok);
            if (!ok){ std::cerr << "Could not parse SCORE: " << parts.last().toStdString() << "\n\n"; return -1;}

            // push it to the list
            detectionScores.append(score);
            detectionPartPositions.append(QMap<int, Position>());

            detectionCount++;
            continue;
        }

        QStringList fields = line.split(',');
        if (fields.length() != 3)
        {
            std::cerr << "Unexpected file content.\n\n";
            return -1;
        }

        Position pos;
        int partNo = fields[0].right(2).toInt(&ok);
        if (!ok){ std::cerr << "Could not parse the number " << fields[0].right(2).toStdString() << "\n\n"; return -1;}
        pos.x = fields[1].toDouble(&ok);
        if (!ok){ std::cerr << "Could not parse the number " << fields[1].toStdString() << "\n\n"; return -1;}
        pos.y = fields[2].toDouble(&ok);
        if (!ok){ std::cerr << "Could not parse the number " << fields[2].toStdString() << "\n\n"; return -1;}

        detectionPartPositions[detectionCount-1].insert(partNo, pos);
    }
    inFile.close();

    // find the best scoring detection, if multiple detections
    double maxScore = *std::max_element(detectionScores.begin(), detectionScores.end());
    int indexOfMax = detectionScores.indexOf(maxScore);
    if (indexOfMax < 0)
    {
        std::cerr << "Fatal error while looking for the max score of detections\n\n";
        return -1;
    }

    //configure output file & write for
    QString outFileName = inFileName;
    outFileName.chop(4); //remove the file extension and the dot
#ifdef HUMANEVA
    outFileName += "_HE.txt";
#elif defined(UMPM)
    outFileName += "_UMPM.txt";
#endif
    if (writeAsText(detectionPartPositions[indexOfMax], outFileName))
        std::cout << "Write completed for detection " << detectionCount << " .\n";
    else
        std::cerr << "Could not open file to write: " << outFileName.toStdString() << "\n\n";

}

void midPoint(double x1, double y1, double x2, double y2, /*OUTPUTS*/ double &midX, double &midY)
{
    midX = 0.5*x1 + 0.5*x2;
    midY = 0.5*y1 + 0.5*y2;
//    std::cout << "midX:\t" << midX << std::endl;
//    std::cout << "midY:\t" << midY << std::endl;
}

#ifdef UMPM
QMap<int, Position> umpm_transback(QMap<int, Position> yrPose)
{
    if (yrPose.keys().length() != 26)
    {
        std::cerr << "Invalid yrPose. There must be 26 parts.\n\n";
        return QMap<int, Position>();
    }

    //----------------------------------------------------------
    //  CONVERT TO UMPM MODEL
    //----------------------------------------------------------
    QMap<int, Position> umpmPose;

    // please check UMPM_trans.m and UMPM_transback.m for details
    // NOTE: since it's a Qmap and not a QList, we can directly use 1-indexed matlab indices
    double pelvisX, pelvisY;
    midPoint(yrPose[10].x, yrPose[10].y, yrPose[22].x, yrPose[22].y, pelvisX, pelvisY);
    Position posPelvis; posPelvis.x = pelvisX; posPelvis.y = pelvisY;
    umpmPose[1] = posPelvis;
    umpmPose[2]  = yrPose[2];  // det(2,:)  =  [bx(2) by(2)];     % neck / thorax
    umpmPose[3]  = yrPose[1];  // det(3,:)  =  [bx(1) by(1)];     % head
    umpmPose[4]  = yrPose[3];  // det(4,:)  =  [bx(3) by(3)];     % R shoulder
    umpmPose[5]  = yrPose[5];  // det(5,:)  =  [bx(5) by(5)];     % R elbow
    umpmPose[6]  = yrPose[7];  // det(6,:)  =  [bx(7) by(7)];     % R wrist
    umpmPose[7]  = yrPose[15]; // det(7,:)  =  [bx(15) by(15)];   % L shoulder
    umpmPose[8]  = yrPose[17]; // det(8,:)  =  [bx(17) by(17)];   % L elbow
    umpmPose[9]  = yrPose[19]; // det(9,:)  =  [bx(19) by(19)];   % L wrist
    umpmPose[10] = yrPose[10]; // det(10,:) =  [bx(10) by(10)];   % R hip
    umpmPose[11] = yrPose[22]; // det(11,:) =  [bx(22) by(22)];   % L hip
    umpmPose[12] = yrPose[12]; // det(12,:) =  [bx(12) by(12)];   % R knee
    umpmPose[13] = yrPose[24]; // det(13,:) =  [bx(24) by(24)];   % L knee
    umpmPose[14] = yrPose[14]; // det(14,:) =  [bx(14) by(14)];   % R ankle
    umpmPose[15] = yrPose[26]; // det(15,:) =  [bx(26) by(26)];   % L ankle

    return umpmPose;

}

bool writeAsText(QMap<int, Position> partPositions, QString outFileName)
{
    QFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QMap<int, Position> umpmPose = umpm_transback(partPositions);

    QTextStream out(&outFile);
    for (int i=1; i<=NB_PARTS; i++)
        out << (i) << "," << umpmPose[i].x << "," << umpmPose[i].y << "\n";

    outFile.close();

    return true;
}

#endif // ifdef UMPM

#ifdef HUMANEVA
bool writeAsText(QMap<int, Position> partPositions, QString outFileName)
{
    QFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly))
    {

        return false;
    }

    QTextStream out(&outFile);
    QMap<int, QString> humEvaNames = initHumanEvaNames();
    double midX = 0.0, midY = 0.0;
    //this is done as described in the doc file: https://docs.google.com/document/d/17aiUHu5Y93kFaDxuANtABTiBGXAJH3m84ymXvYcw0_I/edit?usp=sharing
    out << humEvaNames[1] << ":" <<  partPositions[2].x << "," << partPositions[2].y << "\n";

    midPoint(partPositions[10].x, partPositions[10].y, partPositions[22].x, partPositions[22].y, midX, midY);
    out << humEvaNames[2] << ":" <<  midX << "," << midY << "\n";

    out << humEvaNames[3] << ":" <<  partPositions[15].x << "," << partPositions[15].y << "\n";
    out << humEvaNames[4] << ":" <<  partPositions[17].x << "," << partPositions[17].y << "\n";
    out << humEvaNames[5] << ":" <<  partPositions[17].x << "," << partPositions[17].y << "\n";
    out << humEvaNames[6] << ":" <<  partPositions[19].x << "," << partPositions[19].y << "\n";
    out << humEvaNames[7] << ":" <<  partPositions[3].x << "," << partPositions[3].y << "\n";
    out << humEvaNames[8] << ":" <<  partPositions[5].x << "," << partPositions[5].y << "\n";
    out << humEvaNames[9] << ":" <<  partPositions[5].x << "," << partPositions[5].y << "\n";
    out << humEvaNames[10] << ":" <<  partPositions[7].x << "," << partPositions[7].y << "\n";
    out << humEvaNames[11] << ":" <<  partPositions[22].x << "," << partPositions[22].y << "\n";

    //midPoint(partPositions[24].x, partPositions[24].y, partPositions[25].x, partPositions[25].y, midX, midY);
    out << humEvaNames[12] << ":" <<  partPositions[24].x << "," << partPositions[24].y << "\n";
    out << humEvaNames[13] << ":" <<  partPositions[24].x << "," << partPositions[24].y << "\n";

    out << humEvaNames[14] << ":" <<  partPositions[26].x << "," << partPositions[26].y << "\n";
    out << humEvaNames[15] << ":" <<  partPositions[10].x << "," << partPositions[10].y << "\n";

    //midPoint(partPositions[12].x, partPositions[12].y, partPositions[13].x, partPositions[13].y, midX, midY);
    out << humEvaNames[16] << ":" <<  partPositions[12].x << "," << partPositions[12].y << "\n";
    out << humEvaNames[17] << ":" <<  partPositions[12].x << "," << partPositions[12].y << "\n";

    out << humEvaNames[18] << ":" <<  partPositions[14].x << "," << partPositions[14].y << "\n";
    out << humEvaNames[19] << ":" <<  partPositions[2].x << "," << partPositions[2].y << "\n";
    out << humEvaNames[20] << ":" <<  partPositions[1].x << "," << partPositions[1].y << "\n";

    outFile.close();

    return true;
}

QMap<int, QString> initHumanEvaNames()
{
    QMap<int, QString> humEvaNames;
    humEvaNames.insert(1, "torsoProximal");
    humEvaNames.insert(2, "torsoDistal");
    humEvaNames.insert(3, "upperLArmProximal");
    humEvaNames.insert(4, "upperLArmDistal");
    humEvaNames.insert(5, "lowerLArmProximal");
    humEvaNames.insert(6, "lowerLArmDistal");
    humEvaNames.insert(7, "upperRArmProximal");
    humEvaNames.insert(8, "upperRArmDistal");
    humEvaNames.insert(9, "lowerRArmProximal");
    humEvaNames.insert(10, "lowerRArmDistal");
    humEvaNames.insert(11, "upperLLegProximal");
    humEvaNames.insert(12, "upperLLegDistal");
    humEvaNames.insert(13, "lowerLLegProximal");
    humEvaNames.insert(14, "lowerLLegDistal");
    humEvaNames.insert(15, "upperRLegProximal");
    humEvaNames.insert(16, "upperRLegDistal");
    humEvaNames.insert(17, "lowerRLegProximal");
    humEvaNames.insert(18, "lowerRLegDistal");
    humEvaNames.insert(19, "headProximal");
    humEvaNames.insert(20, "headDistal");

    return humEvaNames;
}
#endif // ifdef HUMANEVA
