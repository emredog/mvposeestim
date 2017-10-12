#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>

#include <iostream>

void ConvertModelMapFunction(const QDir dataDir); //map function

QString exePath;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <fullPathToExe> <rootFolder>\n"
                  << "and it will traverse all S1/S2/S3 (and S4 if available) subjects for all actions for C1/C2/C3 view combinations\n\n";
        return -1;
    }

    exePath  =  QString::fromStdString(argv[1]);
    QString rootFolderPath = QString::fromStdString(argv[2]);
    QDir rootFolder = QDir(rootFolderPath);

    QStringList allSeqFolders = rootFolder.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    QList<QDir> dirsToProcess;
    foreach (QString seqFolder, allSeqFolders)
        dirsToProcess.append(QDir(rootFolder.absoluteFilePath(seqFolder)));

    QFuture<void> futures = QtConcurrent::map(dirsToProcess, ConvertModelMapFunction);

    futures.waitForFinished();

    std::cout << "\nAll folders are processed.\n";

    return 0;

}

void ConvertModelMapFunction(const QDir dataDir)
{
    int counter = 1;
    QProcess* process;
    int total=0;
    int errorCounter = 0;

    // fetch the list of files:
    QStringList nameFilters;
    nameFilters << "*.txt";
    QStringList fileNames = dataDir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    total = fileNames.count();

    foreach (QString fileName, fileNames)
    {
        process = new QProcess();

        QStringList args; args << dataDir.absoluteFilePath(fileName);

        //run the program
        process->start(exePath, args);

        //check if it started normally
        if (!process->waitForStarted(3000))
        {
            std::cerr << "\tCould not start process for: " << fileName.toStdString() << std::endl;
            errorCounter++;
            continue;
        }

        //wait 1 minute for it to finish
        if (!process->waitForFinished(60000)) //wait for 1 minute
        {
            std::cerr << "\tCould not complete process within 1 minute, for:" << fileName.toStdString() << std::endl;

            errorCounter++;
            continue;
        }

        counter++;

        //clean-up
        delete process;
    }

    std::cout << "Folder: " << dataDir.dirName().toStdString() << "\tProcessed: " << counter << "\tErrors: " << errorCounter << std::endl;

}
