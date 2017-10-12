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
                   QString outputFolder,                   
                   QString modelFile,
                   QString detectFastExe,
                   QStringList listToSkip,
                   QString view,
                   QStringList actionClasses,
                   double imgAmount, // ratio of the total images to process by this thread. 1.0 = all images, 0.25 = 1/4 of the all images
                   int imgPartitionIndex, // marks the index of the partition for images to process (zero indexed), 0: start from zero, 3: start from 0.75, if imgAmount = 0.25
                   int frameIncrement = 1)
    {
        this->concurId = concurId;
        this->dataDir = dataDir;
        this->listToSkip = listToSkip;
        this->modelFile = modelFile;
        this->outputFolder = outputFolder;        
        this->detectFastExe = detectFastExe;
        this->actionClasses = actionClasses;
        this->view = view;
        this->imgAmount = imgAmount;
        this->imgPartitionIndex = imgPartitionIndex;
        this->frameIncrement = frameIncrement;
    }

    int concurId;
    QDir dataDir;
    QString outputFolder;    
    QString modelFile;
    QString detectFastExe;
    QStringList listToSkip;
    QString view;
    QStringList actionClasses;
    double imgAmount;
    int imgPartitionIndex;
    int frameIncrement;
};

void executeDetectFastForViewPair(DetectFastArgs args);
void printInfo();


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printInfo();
    if (argc != 6)
    {
        std::cerr << "Usage: " << argv[0] << " <dataDir> <modelFile> <detectFastExe> "
                  << "<outputFolder> <filesToSkip> "
                  << std::endl << std::endl;
        return -1;
    }

    bool ok;

    // SET INPUT PARAMETERS HERE
    // ===========================================================================
    QDir dataDir(QString::fromStdString(argv[1]));
    QString modelFile = QString::fromStdString(argv[2]);
    QString detectFastExe = QString::fromStdString(argv[3]);
    QString outputFolder = QString::fromStdString(argv[4]);    
    QFile fileToSkip(QString::fromStdString(argv[5]));
    // ===========================================================================
    QStringList actionClasses; actionClasses << "Walking" << "Box" << "Gestures" << "Jog" << "ThrowCatch";
    QStringList viewNames; viewNames << "C1" << "C2" << "C3";
    int nbCores = 12;
    int nbConcurrentWorkers = 12; // since we always have 3 viewpoint arrangements for HumanEva, and 4 or 12 cores, 12 works for both of them
    QThreadPool::globalInstance()->setMaxThreadCount(nbCores);
    double factor = 1.0/4.0; // 3 views for SOLO case, always 12 workers ==> each worker process 1/4th of the images for a viewpoint.
    int frameIncrement = 1; // for validaiton cases, skip some frames to get the validation done faster.
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

    QList<QFuture<void> > futures;

    int workerPerView = (int)(1.0/factor); //it's always 4 in SOLO-HumanEva case
    int indexForAView = 0;

    QElapsedTimer timer;
    timer.start();
    // Create a process and launch it
    for (int i=0; i<nbConcurrentWorkers; i++)
    {
        int viewPairIndex = (int)std::floor((double)i * factor);
        QString view = viewNames[viewPairIndex];
        DetectFastArgs dfArgs(i+1, dataDir, outputFolder,
                                 modelFile, detectFastExe, listToSkip,
                                 view, actionClasses,
                                 factor, indexForAView,
                                 frameIncrement);
        std::cout << view.toStdString() << " - " << indexForAView << std::endl;
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

    std::cout << args.concurId <<" > Fetched " << allSeqs.length() / 7 << " sequences with 7 views each.\n";

    // check all views and skip the ones that are not processed by this worker
    for (int v=0; v<allSeqs.length(); v++)
    {
        // get info about the sequence from the first one
        QStringList fields = allSeqs.at(v).split('_'); // S1_Box_1_(Cx) or S1_Box_1_(BWx)
        QString subject = fields.at(0);
        QString action = fields.at(1);
        QString trial = fields.at(2);
        QString curView = fields.at(3);
        curView.remove('('); curView.remove(')'); //remove parantheses
        if (curView.compare(args.view) != 0)
            continue;

        // default skipping rules
        if (!args.actionClasses.contains(action)) continue;        
        if (trial.compare("1") != 0) continue;        

        // fetch folders        
        QDir dirView(args.dataDir.absoluteFilePath(allSeqs.at(v)));
        std::cout << args.concurId <<" > " << dirView.absolutePath().toStdString() << std::endl;

        // prepare prefix to form: "S1_Box_1_(C1)"
        QString prefix = QString("%1_%2_%3_(%4)").arg(subject).arg(action).arg(trial).arg(args.view);

        nameFilters.clear();
        nameFilters << "*.bmp";

        // fetch image names
        QStringList allImagesView = dirView.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);

        int nbImages = allImagesView.length();

        QDir outputdir = QDir(QString("%1%2_%3_%4").arg(args.outputFolder)
                              .arg(subject)
                              .arg(action)
                              .arg(args.view));

        int imgStart = qFloor(args.imgPartitionIndex*args.imgAmount*nbImages);
        int imgEnd = qCeil((args.imgPartitionIndex+1)*args.imgAmount*nbImages);

        std::cout << args.concurId << " > Processing [" << imgStart << "-" << imgEnd << "[ for " << prefix.toStdString() << std::endl
                  << args.concurId << " > Outputdir: " << outputdir.absolutePath().toStdString()  << std::endl;

        for (int f=imgStart; f<imgEnd; f += args.frameIncrement)
        {
            QString imFile = allImagesView.at(f);

            // check if already processed
            QString frameFilter = QString("%1*.txt").arg(imFile.left(6));
            //std::cout << args.concurId << " > " << frameFilter.toStdString() << "\n";
            fields.clear(); //will be used for other purposes
            fields.append(frameFilter);
            //std::cout << args.concurId << " > Namefilters count: " << fields.count() << "\n";
            if (outputdir.entryList(fields, QDir::Files | QDir::NoDotAndDotDot).count() > 0) //means that we already fully processed it obtained 5 txt per image
            {
                std::cout << args.concurId << " > Already processed " << imFile.toStdString() << "\n";
                continue;
            }

            // check if we should skip
            QString checkStr = QString("%1_%2_%3/%4").arg(subject).arg(action).arg(trial).arg(imFile.left(6));
            if (args.listToSkip.indexOf(checkStr) > 0) //means we should skip
            {
                std::cout << args.concurId << " > Skipping " << imFile.toStdString() << "\n";
                continue;
            }

            imFile = dirView.absoluteFilePath(imFile);

            // prepare arguments
            arguments.clear();
            arguments << imFile
                      << args.modelFile
                      << QString("%1%2").arg(args.outputFolder).arg(prefix);


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

            std::cout << args.concurId << " > \tProcess started for\n\t\t" << imFile.toStdString() << std::endl;

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

        std::cout << args.concurId << " > Completed for view " << prefix.toStdString() << std::endl;
    }
}

void printInfo()
{
    std::cout << "Version SOLO\nOptimized for HumanEva.\n\n";
}