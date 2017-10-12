#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QString>
#include <QDir>
#include <QList>

class Configuration
{
public:
    Configuration(QString configFileName);

    double getMinHmMultiplier() const;
    double getMaxHmMultiplier() const;
    double getHmStep() const;
#ifdef WITH_PT
    double getMinPtMultiplier() const;
    double getMaxPtMultiplier() const;
    double getPtStep() const;
#endif
    QString getYrmvAutoExePath() const;
    QDir getDataDir() const;
    QDir getEpipolarDir() const;
    QString getModelFile() const;
    QString getDetectFastExe() const;
    QString getConvertModelAutomatorExe() const;
    QString getCalc2dErrorAutomatorExe() const;
    QString getConvertModelExe() const;
    QString getCalc2dErrorExe() const;
    QString getGroundTruthFolder() const;
    QString getOutputFolder() const;
    QString getPrefixFormat() const;
    QString getFileToSkip() const;
    int getMaxIterations() const;
    double getRequiredEpsilon() const;
    //optional
    QList<double> getHmMultipliersToSkip() const;
#ifdef WITH_PT
    QDir getPtScoresDir() const;
    QList<double> getPtMultipliersToSkip() const;
#endif





private:
    void init(QString configFileName);

    double minHmMultiplier;// = 0.0235; // we actually found this one already.
    double maxHmMultiplier;// = 0.0235;
    double hmStep;// = 0.0020;
#ifdef WITH_PT
    double minPtMultiplier;// = 0.0;
    double maxPtMultiplier;// = 0.1;
    double ptStep;// = 0.01;
#endif



    QString yrmvAutoExePath; // = "/home/emredog/qt-builds/build-yrmv_automator-Desktop-Release/yrmv_automator";
    QDir dataDir; //("/media/emredog/research-data/HumanEva-I/set_TRAIN_subset2/");
    QDir epipolarDir; //("/media/emredog/research-data/HumanEva-I/EpipolarLines/");
    QString modelFile; // = "/home/emredog/git/yrposeestim/C-version/resources/HumanEva_sce02_model_07.txt";
    QString detectFastExe;// = "/home/emredog/git/yrposeestim/C-version/build/detect_fast_C_MV_PTC_LESSLEVELS";
    QString convertModelExe;
    QString convertModelAutomatorExe;// = "/home/emredog/qt-builds/build-ConvertModel_automator-Desktop-Release/ConvertModel_automator";
    QString calc2dErrorExe;
    QString calc2dErrorAutomatorExe;// = "/home/emredog/qt-builds/build-Calc2DError_automator-Desktop-Release/Calc2DError_automator";
    QString groundTruthFolder;
    QString outputFolder;// = "/home/emredog/output/20160106_learning/";
    QString prefixFormat;// = "%1_%2_%3_%4-%5"; // e.g. "S1_Walking_1_BW2-BW3";
    QString fileToSkip; //("/home/emredog/git/yrposeestim/Yang-20121128-pose-release-ver1.3/code-full/List_of_positive_samples/EmptyFile.txt");
    int maxIterations; // = 12;
    double requiredEpsilon; // = 0.1;
    //optional
    QList<double> hmMultipliersToSkip;
#ifdef WITH_PT
    QDir ptScoresDir; //("/media/emredog/research-data/HumanEva-I/PartTypeScores/");
    QList<double> ptMultipliersToSkip;
#endif
};

#endif // CONFIGURATION_H
