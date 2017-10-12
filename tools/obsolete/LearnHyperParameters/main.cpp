#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QElapsedTimer>

#include <iostream>

#include <configuration.h>


class AttemptError
{
public:
    AttemptError(double hmMultiplier, 
#ifdef WITH_PT
                 double ptMultiplier,
#endif
                 double error)
    {       
        this->hmMultiplier = hmMultiplier;
#ifdef WITH_PT
        this->ptMultiplier = ptMultiplier;
#endif
        this->error = error;
    }

    double hmMultiplier;
#ifdef WITH_PT
    double ptMultiplier;
#endif
    double error;

    bool operator<(const AttemptError &ae) const {return (error < ae.error);}
};

void printInfo();

int main(int argc, char *argv[])
{
    printInfo();

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <ConfigFile>\n";
        return -1;
    }

    Configuration config(QString::fromStdString(argv[1]));

    // FETCH PARAMETERS ==============================================================
    double minHmMultiplier = config.getMinHmMultiplier();
    double maxHmMultiplier = config.getMaxHmMultiplier();
    double hmStep = config.getHmStep();
    QList<double> hmMultipliersToSkip = config.getHmMultipliersToSkip();
#ifdef WITH_PT
    double minPtMultiplier = config.getMinPtMultiplier();
    double maxPtMultiplier = config.getMaxPtMultiplier();
    double ptStep = config.getPtStep();
    QDir ptScoresDir = config.getPtScoresDir();
    QList<double> ptMultipliersToSkip = config.getPtMultipliersToSkip();
#endif
    // -------------------------------------------------------------------------
    QString yrmvAutoExePath = config.getYrmvAutoExePath();
    QDir dataDir = config.getDataDir();
    QDir epipolarDir = config.getEpipolarDir();
    QString modelFile = config.getModelFile();
    QString detectFastExe = config.getDetectFastExe();
    QString outputFolder = config.getOutputFolder();

    QString fileToSkip = config.getFileToSkip();
    int maxIterations = config.getMaxIterations();
    double requiredEpsilon = config.getRequiredEpsilon();
    // END OF PARAMETERS ========================================================

//    QString stdOutput;
    QString errorOutput;
    QStringList params;
    QString testName;


    // Start from lower boundary, increment by step until we reach the upper boundary
    for (double heatMapMultiplier=minHmMultiplier; heatMapMultiplier <= maxHmMultiplier ; heatMapMultiplier += hmStep)
    {
        //FIXME: will probably fail (not a good idea to directly compare doubles with high precision)
        if (hmMultipliersToSkip.contains(heatMapMultiplier))
            continue;


#ifdef WITH_PT
        for (double ptMultiplier = minPtMultiplier; ptMultiplier < maxPtMultiplier + ptStep; ptMultiplier += ptStep)
        {
            //FIXME: will probably fail (not a good idea to directly compare doubles with high precision)
            if (ptMultipliersToSkip.contains(ptMultiplier))
                continue;
#endif
            QElapsedTimer timer;
            timer.start();

            testName = QString::number(heatMapMultiplier);
#ifdef WITH_PT
            testName = testName.append("-").append(QString::number(ptMultiplier));
#endif
            testName = testName.replace('.', '_');
            // YRMV ==============================================================
            std::cout << "Running yrmv_automator with heatMapMultiplier: " << heatMapMultiplier
#ifdef WITH_PT
                      << " and partTypeMultiplier: " << ptMultiplier
#endif
                      <<  "..." << std::flush;
            params << dataDir.absolutePath() 
                   << epipolarDir.absolutePath()                   
                   << modelFile
                   << detectFastExe
                   << QString("%1%2/").arg(outputFolder).arg(testName)
                   << fileToSkip
                   << QString::number(maxIterations)
                   << QString::number(requiredEpsilon)
#ifdef WITH_PT
                   << QString::number(heatMapMultiplier)
                   << ptScoresDir.absolutePath()
                   << QString::number(ptMultiplier);
#else
                   << QString::number(heatMapMultiplier);
#endif

            QProcess* yrmvProcess = new QProcess();
            std::cout << params.join(" ").toStdString() << std::endl;
            return -1;

            yrmvProcess->start(yrmvAutoExePath, params);
            if (!yrmvProcess->waitForStarted(1000))
            {
                std::cerr << " > ERROR Could not start yrmvProcess with following parameters:\n\t"
                          << params.join(" ").toStdString() << "\n\n";
                return -1;
            }

            if (!yrmvProcess->waitForFinished(600000000)) //wait for 10000 minutes
            {
                std::cerr << " > ERROR Could not complete yrmvProcess within 10000 minutes, with following parameters:\n\t"
                          << params.join(" ").toStdString() << "\n\n";
                return -1;
            }

//            stdOutput = QString(yrmvProcess->readAllStandardOutput());
            errorOutput = QString(yrmvProcess->readAllStandardError());
//            std::cout << stdOutput.toStdString() << std::endl;
            if (!errorOutput.isEmpty())
            {
                if (!errorOutput.startsWith("ERROR Could not create directory:"))
                {
                    std::cerr << ">>> YRMV AUTOMATOR FAILED.\n";
                    return -1;
                }
                std::cerr << "\n" << errorOutput.toStdString() << "\n";
            }

            delete yrmvProcess;
            params.clear();
            std::cout << "Done\n" << std::flush;

#ifdef WITH_PT
        } // for (double ptMultiplier = minPtMultiplier; ptMultiplier < maxPtMultiplier + ptStep; ptMultiplier += ptStep)
#endif
    }

    std::cout << "Grid search completed.\n";

}

void printInfo()
{
    std::cout << "Version "
#ifdef WITH_PT
              << "MVPTC"
#else
              << "MV"
#endif
              << std::endl;
}
