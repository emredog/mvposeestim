#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QTextStream>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>

#include <iostream>

typedef QPair<int, int> ViewPair;

struct DetectFastArgs
{
    DetectFastArgs(int concurId,
                   QDir dataDir,
                   QDir epipolarDir,
                   QString epipolarFormat,
                   QString outputFolder,                   
                   QString modelFile,
                   int maxIterations,
                   double requiredEpsilon,
                   QString detectFastExe,
                   QStringList listToSkip,
                   ViewPair viewPair,
                   QStringList viewNames,
                   QStringList actionClasses,
                   double imgAmount, // ratio of the total images to process by this thread. 1.0 = all images, 0.25 = 1/4 of the all images
                   int imgPartitionIndex, // marks the index of the partition for images to process (zero indexed), 0: start from zero, 3: start from 0.75, if imgAmount = 0.25
#ifdef WITH_PT
                   double heatMapMultiplier,
                   QDir ptScoresDir,
                   QString ptScoresFormat,
                   double ptScoreMultiplier,
                   int frameIncrement = 1)
#else
                   double heatMapMultiplier,
                   int frameIncrement = 1)
#endif
    {
        this->concurId = concurId;
        this->dataDir = dataDir;
        this->epipolarDir = epipolarDir;        
        this->epipolarFormat = epipolarFormat;        
        this->listToSkip = listToSkip;
        this->maxIterations = maxIterations;
        this->modelFile = modelFile;
        this->outputFolder = outputFolder;        
        this->requiredEpsilon = requiredEpsilon;
        this->detectFastExe = detectFastExe;
        this->viewNames  = viewNames;
        this->actionClasses = actionClasses;
        this->viewPair = viewPair;
        this->heatMapMultiplier = heatMapMultiplier;
        this->imgAmount = imgAmount;
        this->imgPartitionIndex = imgPartitionIndex;
        this->frameIncrement = frameIncrement;
#ifdef WITH_PT
        this->ptScoresDir = ptScoresDir;
        this->ptScoresFormat = ptScoresFormat;
        this->ptScoreMultiplier = ptScoreMultiplier;
#endif

    }

    int concurId;
    QDir dataDir;
    QDir epipolarDir;
    QString epipolarFormat;
    QString outputFolder;    
    QString modelFile;
    int maxIterations;
    double requiredEpsilon;
    QString detectFastExe;
    QStringList listToSkip;
    ViewPair viewPair;
    QStringList viewNames;
    QStringList actionClasses;
    double imgAmount;
    int imgPartitionIndex;
    double heatMapMultiplier;
    int frameIncrement;
#ifdef WITH_PT
    QDir ptScoresDir;
    QString ptScoresFormat;
    double ptScoreMultiplier;
#endif
};

void executeDetectFastForViewPair(DetectFastArgs args);
void printInfo();


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printInfo();

#ifdef WITH_PT
    if (argc != 12)
#else
    if (argc != 10)
#endif
    {
        std::cerr << "Usage: " << argv[0] << " <dataDir> <epipolarDir> <modelFile> <detectFastExe> "
                  << "<outputFolder> <filesToSkip> "
                  << "<maxIterations> <requiredEpsilon> <heatMapMultiplier> "
#ifdef WITH_PT
                  << "<partTypeScoresDir> <partTypeScoreMultiplier>"
#endif
                  << std::endl << std::endl;
        return -1;
    }

    bool ok;

    // SET INPUT PARAMETERS HERE
    // ===========================================================================
    QDir dataDir(QString::fromStdString(argv[1]));
    QDir epipolarDir(QString::fromStdString(argv[2]));
    QString modelFile = QString::fromStdString(argv[3]);
    QString detectFastExe = QString::fromStdString(argv[4]);
    QString outputFolder = QString::fromStdString(argv[5]);

    QFile fileToSkip(QString::fromStdString(argv[6]));
    int maxIterations = QString::fromStdString(argv[7]).toInt(&ok);
    double requiredEpsilon = QString::fromStdString(argv[8]).toDouble(&ok);
    double heatMapMultiplier = QString::fromStdString(argv[9]).toDouble(&ok);
#ifdef WITH_PT
    //to build ptScoreFormat:
    QString modelName;
    {
        // modelFile: /home/emredog/git/yrposeestim/C-version/resources/models/HumanEva_13_S1BoxWalk.txt
        QStringList parts = modelFile.split('/');
        modelName = parts.last();
        parts = modelName.split('.');
        modelName = parts.first();
    }
    QDir ptScoreDir(QString::fromStdString(argv[10]));
    double ptScoreMultiplier = QString::fromStdString(argv[11]).toDouble(&ok);
    QString ptScoreFormat  = "PTC_" + modelName + "_%1wrt%2.csv"; //PTC_HumanEva_13_S1WalkBox_C1wrtC2.csv
#endif
    // ===========================================================================
    QString epipolarFormat = "%1_epiBorderPointsIn%2wrt%3.csv"; // "S1_epiBorderPointsInBW1wrtBW2.csv"    
    QStringList actionClasses; actionClasses << "Walking" << "Box" << "Gestures" << "Jog" << "ThrowCatch";
    QStringList viewNames; viewNames << "C1" << "C2" << "C3";

    // Configuration for jumala:
//    int nbCores = 12;
//    int nbConcurrentWorkers = 12; // since we always have 3 viewpoint arrangements for HumanEva, and 4 or 12 cores, 12 works for both of them
//    QThreadPool::globalInstance()->setMaxThreadCount(nbCores);
//    double factor = 0.25; // always 3 viewpoint arrangements, always 12 workers ==> each worker process 1/4th of the images for a viewpoint arrng.

    // Configuration for gemini
    int nbCores = 24;
    int nbConcurrentWorkers = 24; // since we always have 3 viewpoint arrangements for HumanEva, and 24 workers
    QThreadPool::globalInstance()->setMaxThreadCount(nbCores);
    double factor = 0.125; // always 3 viewpoint arrangements, and 24 workers ==> each worker process 1/8th of the images for a viewpoint arrng.

    int frameIncrement = 1; // for validaiton cases, skip some frames to get the validation done faster.
    std::cout << "# of cores: " << nbCores << std::endl
              << "# of concurrent workers: " << nbConcurrentWorkers << std::endl
              << "data factor: " << factor << std::endl
              << "Frame increment: " << frameIncrement << std::endl;
    // ===========================================================================

    QStringList listToSkip;

    if (fileToSkip.exists() && fileToSkip.open(QIODevice::ReadOnly))
    {
        QTextStream in(&fileToSkip);
        QString line; QStringList parts;
        while (!in.atEnd())
        {
            line = in.readLine();
            parts = line.split('/');
            parts[2].chop(5); //S3_Box_1_(C2) becomes S3_Box_1
            parts[3].chop(5); //im0713.jpg' becomes im0713
            line = QString("%1/%2").arg(parts[2]).arg(parts[3]);
            if (listToSkip.indexOf(line) < 0) //do not insert duplicates
                listToSkip.append(line);
        }
    }

    std::cout << "Started at: " << QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss").toStdString() << "\n";
    std::cout << "Data folder: " << dataDir.absolutePath().toStdString() << std::endl;

    //prepare view pairs
    QList<ViewPair> viewPairs;
    viewPairs << ViewPair(0, 1); //c1  - c2
    viewPairs << ViewPair(1, 2); //c2  - c3
    viewPairs << ViewPair(2, 0); //c3  - c1

    QList<QFuture<void> > futures;

    int workerPerView = (int)(1.0/factor); //it's always 2
    int indexForAView = 0;

    QElapsedTimer timer;
    timer.start();
    // Create a process and launch it
    for (int i=0; i<nbConcurrentWorkers; i++)
    {
        int viewPairIndex = (int)((double)i * factor);
        ViewPair viewPair = viewPairs[viewPairIndex];
        DetectFastArgs dfArgs(i+1, dataDir, epipolarDir, epipolarFormat, outputFolder,
                                 modelFile, maxIterations, requiredEpsilon, detectFastExe, listToSkip,
                                 viewPair, viewNames, actionClasses,
                                 factor, indexForAView,
    #ifdef WITH_PT
                                 heatMapMultiplier, ptScoreDir, ptScoreFormat, ptScoreMultiplier, frameIncrement);
    #else
                                 heatMapMultiplier, frameIncrement);
    #endif

        QFuture<void> fut = QtConcurrent::run(executeDetectFastForViewPair, dfArgs);
        futures.append(fut);

        indexForAView++;
        if (indexForAView == workerPerView) // should be 0, 1, 2 or 3
            indexForAView = 0;
    }


    // wait for processes to terminate
    foreach (QFuture<void> fut, futures)
    {
        fut.waitForFinished();
    }

    std::cout << "\nElapsed time: " << timer.elapsed() / 1000.0 << " seconds.\n";

    return 0;
}

void executeDetectFastForViewPair(DetectFastArgs args)
{
    //common variables
    QStringList nameFilters;
    QStringList arguments;

    // Get all views of all sequences
    QStringList allSeqs = args.dataDir.entryList(nameFilters, QDir::Dirs | QDir::NoDotAndDotDot);

    std::cout << args.concurId <<" > Fetched " << allSeqs.length() / 3 << " sequences with 3 views each.\n";

    //we have 3 views for each seq, so iterate by 3
    for (int v=0; v<allSeqs.length(); v += 3)
    {
        // get info about the sequence from the first one
        QStringList fields = allSeqs.at(v).split('_');
        QString subject = fields.at(0);
        QString action = fields.at(1);
        QString trial = fields.at(2);

        // default skips
        if (!args.actionClasses.contains(action)) continue;
        if (trial.compare("1") != 0) continue;

        std::cout << args.concurId << " > Processing: " << subject.toStdString() << " " << action.toStdString() << " " << trial.toStdString() << std::endl;

        // fetch folders
        std::cout << args.concurId <<" > \tPair: " << args.viewNames.at(args.viewPair.first).toStdString()
                  << " & " << args.viewNames.at(args.viewPair.second).toStdString() << std::endl;
        QDir dirViewA(args.dataDir.absoluteFilePath(allSeqs.at(v + args.viewPair.first)));
        QDir dirViewB(args.dataDir.absoluteFilePath(allSeqs.at(v + args.viewPair.second)));

        // prepare epipolar file names
        QString epipolarFileAtoB = QString(args.epipolarFormat).arg(subject).arg(args.viewNames.at(args.viewPair.first)).arg(args.viewNames.at(args.viewPair.second));
        QString epipolarFileBtoA = QString(args.epipolarFormat).arg(subject).arg(args.viewNames.at(args.viewPair.second)).arg(args.viewNames.at(args.viewPair.first));
        epipolarFileAtoB = args.epipolarDir.absoluteFilePath(epipolarFileAtoB);
        epipolarFileBtoA = args.epipolarDir.absoluteFilePath(epipolarFileBtoA);

#ifdef WITH_PT
        // prepare part type score file names
        QString ptScoreFileAtoB = QString(args.ptScoresFormat).arg(args.viewNames.at(args.viewPair.first)).arg(args.viewNames.at(args.viewPair.second));
        QString ptScoreFileBtoA = QString(args.ptScoresFormat).arg(args.viewNames.at(args.viewPair.second)).arg(args.viewNames.at(args.viewPair.first));
        ptScoreFileAtoB = args.ptScoresDir.absoluteFilePath(ptScoreFileAtoB);
        ptScoreFileBtoA = args.ptScoresDir.absoluteFilePath(ptScoreFileBtoA);
#endif

        // prepare prefix
        QString prefix = QString("%1_%2_%3_%4-wrt-%5").arg(subject).arg(action).arg(trial).arg(args.viewNames.at(args.viewPair.first)).arg(args.viewNames.at(args.viewPair.second));

        nameFilters.clear();
        nameFilters << "*.bmp";

        // fetch image names
        QStringList allImagesViewA = dirViewA.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
        QStringList allImagesViewB = dirViewB.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);

        int nbImages = (allImagesViewA.length() <= allImagesViewB.length()) ? allImagesViewA.length() : allImagesViewB.length();

        QDir outputdir = QDir(QString("%1%2").arg(args.outputFolder).arg(prefix));

        std::cout << args.concurId << " > " << outputdir.absolutePath().toStdString() << "\n";
        int imgStart = qFloor(args.imgPartitionIndex*args.imgAmount*nbImages);
        int imgEnd = qCeil((args.imgPartitionIndex+1)*args.imgAmount*nbImages);

        std::cout << "Processing [" << imgStart << "-" << imgEnd << "[ for " << prefix.toStdString() << std::endl;

        for (int f=imgStart; f<imgEnd; f += args.frameIncrement)
        {
            QString imFileA = allImagesViewA.at(f);
            QString imFileB = allImagesViewB.at(f);

            // check if already processed
            QString frameFilter = QString("%1*.txt").arg(imFileA.left(6));
            std::cout << args.concurId << " > " << frameFilter.toStdString() << "\n";
            fields.clear(); //will be used for other purposes
            fields.append(frameFilter);

            if (outputdir.entryList(fields, QDir::Files | QDir::NoDotAndDotDot).count() == 5) //means that we already fully processed it, and obtained 5 txt per image
            {
                std::cout << args.concurId << " > Already processed " << imFileA.toStdString() << "\n";
                continue;
            }

            // check if we should skip
            QString checkStr = QString("%1_%2_%3/%4").arg(subject).arg(action).arg(trial).arg(imFileA.left(6));
            if (args.listToSkip.indexOf(checkStr) > 0) //means we should skip
            {
                std::cout << args.concurId << " > Skipping " << imFileA.toStdString() << "\n";
                continue;
            }

            if (imFileA.compare(imFileB) != 0)
            {
                std::cerr << args.concurId << " > WARNING! Filename mismatch: " << imFileA.toStdString() << " vs. " << imFileB.toStdString() << std::endl;
                continue;
            }

            imFileA = dirViewA.absoluteFilePath(imFileA);
            imFileB = dirViewB.absoluteFilePath(imFileB);

            // prepare arguments
            arguments.clear();
            arguments << imFileA << imFileB
                      << epipolarFileBtoA << epipolarFileAtoB
#ifdef WITH_PT
                      << ptScoreFileBtoA << ptScoreFileAtoB
#endif
                      << QString::number(args.maxIterations)
                      << QString::number(args.requiredEpsilon)
                      << QString("%1%2").arg(args.outputFolder).arg(prefix)
                      << args.modelFile
#ifdef WITH_PT
                      << QString::number(args.heatMapMultiplier)
                      << QString::number(args.ptScoreMultiplier);
#else
                      << QString::number(args.heatMapMultiplier);
#endif

//            std::cout << arguments.join(" ").toStdString() << std::endl;
//            return;

            // execute the detect fast program
            QProcess* process = new QProcess();
            process->start(args.detectFastExe, arguments);

            //check if it started normally
            if (!process->waitForStarted(3000))
            {
                std::cerr << args.concurId << " > ERROR Could not start process with following parameters:\n\t"
                          << arguments.join(" ").toStdString() << "\n\n";
                continue;
            }

            std::cout << args.concurId << " > \tProcess started for\n\t\t" << imFileA.toStdString() << "\n\t\t" << imFileB.toStdString() << std::endl;

            if (!process->waitForFinished(600000)) //wait for 10 minutes
            {
                std::cerr << args.concurId << " > ERROR Could not complete process within 10 minutes, with following parameters:\n\t"
                          << arguments.join(" ").toStdString() << "\n\n";
                continue;
            }

            std::cout << args.concurId << " > \tDone.\n";

            QString errorOutput = QString(process->readAllStandardError());
            if (!errorOutput.isEmpty())
            {
                std::cerr << errorOutput.toStdString() << std::endl;
            }

            delete process;
        }

        std::cout << args.concurId << " > Completed for this view pair\n\n";
    }
}

void printInfo()
{
    std::cout << "Version "
#ifdef WITH_PT
              << "MVPTC"
#else
              << "MV"
#endif
              << std::endl << "Optimized for HumanEva\n\n";
}
