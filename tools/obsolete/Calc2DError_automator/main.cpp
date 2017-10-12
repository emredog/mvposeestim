#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QtConcurrent/QtConcurrent>

#include <iostream>

//#define HUMANEVA
#define UMPM

#ifdef HUMANEVA
double calc2dError(QStringList args); //map function
void sum2dError(double &sum, const double &error); // sum operator
#elif defined(UMPM)
#define NB_PARTS 15
QMap<QString, double> calc2dError(QStringList args); //map function
void sum2dError(QMap<QString, double> &sum, const QMap<QString, double> &error); // sum operator
QMap<QString, double> parseCalc2DoutputUmpm(QString output);
#endif



QString getGroundTruthPath(QStringList allGts, QString view);

static int errorCounter = 0;

int main(int argc, char *argv[])
{    
    if (argc <= 4)
    {
        std::cerr << "Usage: " << argv[0] << "<fullPathToExe> <outputPoseFilesDir> <groundTruthRootFolder> [mv/solo]\n(default is mv)\n\n";
        return -1;
    }

    QString calc2dErrorExe = QString::fromStdString(argv[1]);
    QDir hmDir = QDir(QString::fromStdString(argv[2]));
    QString gtRootFolder = QString::fromStdString(argv[3]);

    QString mvOrSolo = "mv";
    if (argc == 5)
        mvOrSolo = QString::fromStdString(argv[4]);

    bool isMv = true;
    if (mvOrSolo.compare("mv", Qt::CaseInsensitive) == 0)
        isMv = true;
    else if (mvOrSolo.compare("solo", Qt::CaseInsensitive) == 0)
        isMv = false;



    QStringList nameFilters;

#ifdef HUMANEVA
    QStringList gtPaths;
    // Groundt Truth files======================================================
    gtPaths << QString("%1Train_C1.txt").arg(gtRootFolder);
    gtPaths << QString("%1Train_C2.txt").arg(gtRootFolder);
    gtPaths << QString("%1Train_C3.txt").arg(gtRootFolder);
#endif

#ifdef UMPM
    QHash<QString, QString> gtPaths;
    // fetch all gt files
    {
        nameFilters << "*_2D_vm.csv";
        QDir gtRootDir(gtRootFolder);
        QStringList gtFileNames = gtRootDir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
        foreach(QString gtFileName, gtFileNames)
        {
            QString seqName = gtFileName;
            seqName.chop(10); // remove "_2D_vm.csv" part
            gtPaths.insert(seqName, gtRootDir.absoluteFilePath(gtFileName));
        }
    }
#endif



    std::cout << "Calculating error against:\n";
    foreach(QString s, gtPaths) std::cout << s.toStdString() << std::endl;




    // this is the list of all the executions
    QList<QStringList> allArgsToProcess;
    // Fetch all subfolders (2 levels deep)
    nameFilters.clear();
    QStringList seqPaths = hmDir.entryList(nameFilters, QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(QString seqPath, seqPaths) // for each sequence folder (each of which contains several results)
    {
#ifdef HUMANEVA
        QStringList elems = seqPath.split('_');
        QString viewComb = elems.last();
        elems = viewComb.split('-');
        QString viewA = elems.first();
        QString viewB = elems.last();
        QString gtForViewA = getGroundTruthPath(gtPaths, viewA);
        QString gtForViewB = getGroundTruthPath(gtPaths, viewB);
#endif

#ifdef UMPM
        QStringList elems = seqPath.split('/'); // "/home/emredog/output/bla_bla_bla/p1_chair_2_f-wrt-l"
        QString seqNameWithViewComb = elems.last(); // "p1_chair_2_f-wrt-l"
        elems = seqNameWithViewComb.split('_');
        QString seqName = QString("%1_%2_%3").arg(elems.at(0)).arg(elems.at(1)).arg(elems.at(2)); // "p1_chair_2"
        QString viewComb = elems.last(); // "f-wrt-l"
        elems = viewComb.split('-');
        QString viewA = elems.first(); // "f"
        QString viewB = elems.last();  // "l"
        QString gtForViewA = gtPaths[QString("%1_%2").arg(seqName).arg(viewA)];
        QString gtForViewB = gtPaths[QString("%1_%2").arg(seqName).arg(viewB)];
#endif

        QDir seqDir = QDir(hmDir.absoluteFilePath(seqPath));
        nameFilters.clear();
#ifdef HUMANEVA
        if (isMv)
            nameFilters << "*bmp_A_Pose_HE.txt"; //don't take Solo poses, or poses that are not converted to HE model
        else
            nameFilters << "*bmp_Solo_A_Pose_HE.txt"; //don't take MV poses, or poses that are not converted to HE model
#elif defined(UMPM)
        if (isMv)
            nameFilters << "*png_A_Pose_UMPM.txt"; //don't take Solo poses or poses that are not converted to UMPM model
        else
            nameFilters << "*png_Solo_A_Pose_UMPM.txt"; //don't take MV poses or poses that are not converted to UMPM model
#endif

        QStringList mvPosesA = seqDir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
        std::cout << "[" << seqName.toStdString()  <<"] Fetched "  << mvPosesA.length() << " poses for view "<< viewA.toStdString() << "\n";
        nameFilters.clear();
#ifdef HUMANEVA
        if (isMv)
            nameFilters << "*bmp_B_Pose_HE.txt"; //don't take Solo poses, or poses that are not converted to HE model
        else
            nameFilters << "*bmp_Solo_B_Pose_HE.txt"; //don't take MV poses, or poses that are not converted to HE model
#elif defined(UMPM)
        if (isMv)
            nameFilters << "*png_B_Pose_UMPM.txt"; //don't take Solo poses or poses that are not converted to UMPM model
        else
            nameFilters << "*png_Solo_B_Pose_UMPM.txt"; //don't take MV poses or poses that are not converted to UMPM model
#endif


        QStringList mvPosesB = seqDir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
        std::cout << "[" << seqName.toStdString()  <<"] Fetched "  << mvPosesB.length() << " poses for view "<< viewB.toStdString() << "\n";
        if (mvPosesA.length() != mvPosesB.length())
        {
            std::cerr << "[WARNING] Inconsistent number of *_A_Pose_HE_.txt and *_B_Pose_HE_.txt images in: " << seqPath.toStdString() << "\n\n";
            continue;
        }

        for (int i=0; i<mvPosesA.length(); i++) // for each result pair *bmp_A_Pose_HE_.txt and *bmp_B_Pose_HE_.txt
        {
            QStringList argsForA, argsForB;
            argsForA << calc2dErrorExe << gtForViewA; //add executable path & ground truth path
            argsForB << calc2dErrorExe << gtForViewB;

            argsForA << seqDir.absoluteFilePath(mvPosesA.at(i)); //add pose file path
            argsForB << seqDir.absoluteFilePath(mvPosesB.at(i));

            allArgsToProcess.append(argsForA); //add them to the grand "arguments" file.
            allArgsToProcess.append(argsForB);
        }
//        std::cout << "." << std::flush;
    }

    std::cout << "Preprocessing complete.\n";

    QElapsedTimer timer;
    timer.start();

#ifdef HUMANEVA
    QFuture<double> errorsSumFut = QtConcurrent::mappedReduced(allArgsToProcess, calc2dError, sum2dError);
    errorsSumFut.waitForFinished();
    double errorSum = errorsSumFut.result();
    double errorMean = errorSum / (double)(allArgsToProcess.length() - errorCounter);
    std::cout << "Sum:" << errorSum << std::endl;
    std::cout << "Mean:" << errorMean << std::endl;
#elif defined(UMPM)
    QFuture<QMap<QString, double> > errorsSumFut = QtConcurrent::mappedReduced(allArgsToProcess, calc2dError, sum2dError);
    errorsSumFut.waitForFinished();
    QMap<QString, double>  errorSum = errorsSumFut.result();

    std::cout << "\nOperation took " << (double)timer.elapsed()/1000.0 << " seconds\n";

    if (isMv)
        std::cout << "MULTIVIEW ERRORS\n";
    else
        std::cout << "SOLO ERRORS\n";
    std::cout << "PART BASED ERRORS for all processed actions and view arrangements (" << allArgsToProcess.length() << ")\n";

    // hack: keys are ordered such as 1,10,11,12,13,14,15,2,...
    // but I want output the other way, so
    QStringList keys; keys << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8"
                           << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "mean";

    for(int i=0; i<keys.length(); i++)
    {
        QString key = keys.at(i);
        std::cout << key.toStdString() << "\t" << (errorSum[key] / (double)allArgsToProcess.length()) << "\n";
    }

#endif

    return 0;

}

#ifdef HUMANEVA
double calc2dError(QStringList args)
#elif defined(UMPM)
QMap<QString, double> calc2dError(QStringList args)
#endif

{
#ifdef HUMANEVA
    double error = -1.0;
#elif defined(UMPM)
    QMap<QString, double> error;
#endif

    QProcess* process = new QProcess();

    QString calc2dExe = args.takeFirst();

    process->start(calc2dExe, args);

    //check if it started normally
    if (!process->waitForStarted(1000))
    {
        std::cerr << " > ERROR Could not start process with following parameters:\n\t"
                  << args.join(" ").toStdString() << "\n\n" << std::flush;
        return error;
    }

    if (!process->waitForFinished(2000)) //wait for 2sec to finish
    {
        std::cerr << " > ERROR Could not complete process within 2 secs, with following parameters:\n\t"
                  << args.join(" ").toStdString() << "\n\n" << std::flush;
        return error;
    }

    QString output(process->readAllStandardOutput());
    QString runtimeErrors(process->readAllStandardError());
    if (!runtimeErrors.isEmpty())
        std::cerr << "\n" << runtimeErrors.toStdString() << "\n";

    std::cout << "." << std::flush;

    process->kill();
    delete process;

#ifdef HUMANEVA
    bool ok;
    error = output.toDouble(&ok);
    if (!ok)
    {
        std::cerr << " > ERROR Could not parse the output of Calc2DError for parameters: " << args.join(" ").toStdString() << "\n\n" << std::flush;
        errorCounter++;
    }

    return error;
#elif defined(UMPM)
    // parse output from process and return set of part based errors

    QMap<QString, double> errorsForImg = parseCalc2DoutputUmpm(output);

    if (errorsForImg.empty() || errorsForImg.keys().length() != NB_PARTS+1)
    {
        errorCounter++;
        return QMap<QString, double>();
    }
    else
    {
        return errorsForImg;
    }
#endif
}


#ifdef HUMANEVA
void sum2dError(double &sum, const double &error)
{
    if (error >= 0)
        sum += error;
}
#endif

#ifdef UMPM
void sum2dError(QMap<QString, double> &sums, const QMap<QString, double> &errors)
{
    QStringList keys = errors.keys();

    foreach(QString key, keys)
    {
        sums[key] = sums[key] + errors[key];
    }
}

#endif


#ifdef UMPM
QMap<QString, double> parseCalc2DoutputUmpm(QString output)
{
    output = output.trimmed();
    QStringList lines = output.split("\n");
    if (lines.length() != NB_PARTS+1) // for each part + mean
    {
        std::cerr << "Output parsing error: Unexpected number of lines\n\n";
        std::cerr << output.toStdString() << std::endl;
        return QMap<QString, double>();
    }

    QMap<QString, double> errors;
    QStringList elems;
    bool ok;

    foreach(QString l, lines)
    {
        elems.clear();
        elems = l.split(",");
        if (elems.length() != 2)
        {
            std::cerr << "Unexpected number of data in line:\n\t" << l.toStdString() << std::endl;
            continue;
        }

        double err = elems.at(1).toDouble(&ok);
        if (!ok)
        {
            std::cerr << "Could not parse the double value in line:\n\t" << l.toStdString() << std::endl;
            continue;
        }
//        std::cout << "Key: " << elems.at(0).toStdString() << ", Value: " << err << "\n";
        errors.insert(elems.at(0), err);
    }

    return errors;
}

#endif

#ifdef HUMANEVA
QString getGroundTruthPath(QStringList allGts, QString view)
{
    //it's a fairly stupid function
    if (view.compare("C1") == 0)
        return allGts.at(0);
    else if (view.compare("C2") == 0)
        return allGts.at(1);
    else if (view.compare("C3") == 0)
        return allGts.at(2);
    else
        return QString();
}
#endif

