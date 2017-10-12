#include "convertmodelthread.h"

#include <QProcess>
#include <iostream>

using namespace std;

ConvertModelThread::ConvertModelThread(const int id, const QString exePath, const QDir dataDir, QObject *parent) :
    QThread(parent)
{
    this->dataDir = dataDir;
    this->program = exePath;
    this->id = id;
}

void ConvertModelThread::run()
{
    int counter = 1;
    QProcess* process;
    int total=0;
    int errorCounter = 0;

//    cout << "[" << this->id << "]\t" << "Processing files in " << dataDir.absolutePath().toStdString() << endl;

    // fetch the list of files:
    QStringList nameFilters;
    nameFilters << "*.txt";
    QStringList fileNames = dataDir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    total = fileNames.count();

//    cout << "[" << this->id << "]\t" << "Fetched " << total << " files to process.\n";

    foreach (QString fileName, fileNames)
    {
        process = new QProcess();

        QStringList args; args << dataDir.absoluteFilePath(fileName);

        //run the program
        process->start(this->program, args);

        //check if it started normally
        if (!process->waitForStarted(3000))
        {
            cerr << "[" << this->id << "]\tCould not start process for: " << fileName.toStdString() << endl;
            errorCounter++;
            continue;
        }

        //wait 1 minute for it to finish
        if (!process->waitForFinished(60000)) //wait for 1 minute
        {
            cerr << "[" << this->id << "]\tCould not complete process within 1 minute, for:" << fileName.toStdString() << endl;

            errorCounter++;
            continue;
        }

//        cout << QString("[%1] Process %2 completed. Remaining: %3").arg(this->id)
//                .arg(counter*this->id)
//                .arg(total-counter).toStdString() << endl;
        counter++;

        //clean-up
        delete process;
    }

//    cout << "\nThread " << this->id << " is completed.\n"
//             << "Processed:\t" << counter << endl
//             << "Error:\t"     << errorCounter << endl;

    this->quit();


}
