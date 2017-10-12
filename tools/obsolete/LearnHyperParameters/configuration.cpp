#include "configuration.h"
#include <QXmlStreamReader>
#include <QFile>

#include <iostream>

Configuration::Configuration(QString configFileName)
{
    this->init(configFileName);
}

void Configuration::init(QString configFileName)
{
    QFile cnfgFile(configFileName);
    if (!cnfgFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        std::cerr << "ERROR: could not open config file: " << configFileName.toStdString() << std::endl << std::endl;
    }

    //parse the file
    QXmlStreamReader xml(&cnfgFile);
    while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();
        if (xml.isStartElement())
        {
            QString name = xml.name().toString();
            if (name == "minHmMultiplier")
            {
                bool ok;
                QString str = xml.readElementText();
                this->minHmMultiplier = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: minHmMultiplier could not be parsed: " << str.toStdString() << std::endl;
            }
            else if (name == "maxHmMultiplier")
            {
                bool ok;
                QString str = xml.readElementText();
                this->maxHmMultiplier = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: maxHmMultiplier could not be parsed: " << str.toStdString() << std::endl;
            }
            else if (name == "hmStep")
            {
                bool ok;
                QString str = xml.readElementText();
                this->hmStep = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: hmStep could not be parsed: " << str.toStdString() << std::endl;
            }
#ifdef WITH_PT
            else if (name == "minPtMultuplier")
            {
                bool ok;
                QString str = xml.readElementText();
                this->minPtMultiplier = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: minPtMultuplier could not be parsed: " << str.toStdString() << std::endl;
            }
            else if (name == "maxPtMultiplier")
            {
                bool ok;
                QString str = xml.readElementText();
                this->maxPtMultiplier = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: maxPtMultiplier could not be parsed: " << str.toStdString() << std::endl;
            }
            else if (name == "ptStep")
            {
                bool ok;
                QString str = xml.readElementText();
                this->ptStep = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: ptStep could not be parsed: " << str.toStdString() << std::endl;
            }
#endif
            else if (name == "yrmvAutoExePath")
            {
               this->yrmvAutoExePath = xml.readElementText();
            }
            else if (name == "dataDir")
            {
                QString dirStr = xml.readElementText();
                if (!dirStr.endsWith("/"))
                    dirStr = dirStr.append("/");
                this->dataDir = dirStr;
            }
            else if (name == "epipolarDir")
            {
                QString dirStr = xml.readElementText();
                if (!dirStr.endsWith("/"))
                    dirStr = dirStr.append("/");
                this->epipolarDir = dirStr;
            }
            else if (name == "modelFile")
            {
                this->modelFile = xml.readElementText();
            }
            else if (name == "detectFastExe")
            {
                this->detectFastExe = xml.readElementText();
            }
            else if (name == "convertModelExe")
            {
                this->convertModelExe = xml.readElementText();
            }
            else if (name == "convertModelAutomatorExe")
            {
                this->convertModelAutomatorExe = xml.readElementText();
            }
            else if (name == "calc2dErrorExe")
            {
                this->calc2dErrorExe = xml.readElementText();
            }
            else if (name == "calc2dErrorAutomatorExe")
            {
                this->calc2dErrorAutomatorExe = xml.readElementText();
            }
            else if (name == "groundTruthFolder")
            {
                this->groundTruthFolder = xml.readElementText();
            }
            else if (name == "outputFolder")
            {
                QString dirStr = xml.readElementText();
                if (!dirStr.endsWith("/"))
                    dirStr = dirStr.append("/");
                this->outputFolder = dirStr;
            }
            else if (name == "prefixFormat")
            {
                this->prefixFormat = xml.readElementText();
            }
            else if (name == "fileToSkip")
            {
                this->fileToSkip = xml.readElementText();
            }
            else if (name == "maxIterations")
            {
                bool ok;
                QString str = xml.readElementText();
                this->maxIterations = str.toInt(&ok);
                if (!ok) std::cerr << "ERROR: maxIterations could not be parsed: " << str.toStdString() << std::endl;
            }
            else if (name == "requiredEpsilon")
            {
                bool ok;
                QString str = xml.readElementText();
                this->requiredEpsilon = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: requiredEpsilon could not be parsed: " << str.toStdString() << std::endl;
            }
            else if (name == "hmmskip")
            {
                bool ok;
                QString str = xml.readElementText();
                double val = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: hmmskip could not be parsed: " << str.toStdString() << std::endl;
                this->hmMultipliersToSkip.append(val);
            }
#ifdef WITH_PT
            else if (name == "ptScoresDir")
            {
                QString dirStr = xml.readElementText();
                if (!dirStr.endsWith("/"))
                    dirStr = dirStr.append("/");
                this->ptScoresDir = dirStr;
            }
            else if (name == "ptskip")
            {
                bool ok;
                QString str = xml.readElementText();
                double val = str.toDouble(&ok);
                if (!ok) std::cerr << "ERROR: ptskip could not be parsed: " << str.toStdString() << std::endl;
                this->ptMultipliersToSkip.append(val);
            }
#endif
        }
    }
    if (xml.hasError())
    {
        std::cerr << "XML error: " << xml.errorString().toStdString() << std::endl;
    }
    else if (xml.atEnd())
    {
        std::cout << "Configuration file parsed successfully." << std::endl;
    }
}
QString Configuration::getCalc2dErrorExe() const
{
    return calc2dErrorExe;
}

QString Configuration::getConvertModelExe() const
{
    return convertModelExe;
}



#ifdef WITH_PT
double Configuration::getPtStep() const
{
    return ptStep;
}

double Configuration::getMaxPtMultiplier() const
{
    return maxPtMultiplier;
}

double Configuration::getMinPtMultiplier() const
{
    return minPtMultiplier;
}
#endif

QString Configuration::getGroundTruthFolder() const
{
    return groundTruthFolder;
}

double Configuration::getHmStep() const
{
    return hmStep;
}

double Configuration::getMaxHmMultiplier() const
{
    return maxHmMultiplier;
}

double Configuration::getMinHmMultiplier() const
{
    return minHmMultiplier;
}

QList<double> Configuration::getHmMultipliersToSkip() const
{
    return hmMultipliersToSkip;
}

QDir Configuration::getEpipolarDir() const
{
    return epipolarDir;
}

QString Configuration::getYrmvAutoExePath() const
{
    return yrmvAutoExePath;
}

QDir Configuration::getDataDir() const
{
    return dataDir;
}

QString Configuration::getModelFile() const
{
    return modelFile;
}

QString Configuration::getDetectFastExe() const
{
    return detectFastExe;
}

QString Configuration::getConvertModelAutomatorExe() const
{
    return convertModelAutomatorExe;
}

QString Configuration::getCalc2dErrorAutomatorExe() const
{
    return calc2dErrorAutomatorExe;
}

QString Configuration::getOutputFolder() const
{
    return outputFolder;
}

QString Configuration::getPrefixFormat() const
{
    return prefixFormat;
}

QString Configuration::getFileToSkip() const
{
    return fileToSkip;
}

int Configuration::getMaxIterations() const
{
    return maxIterations;
}

double Configuration::getRequiredEpsilon() const
{
    return requiredEpsilon;
}

#ifdef WITH_PT
QDir Configuration::getPtScoresDir() const
{
    return ptScoresDir;
}

QList<double> Configuration::getPtMultipliersToSkip() const
{
    return ptMultipliersToSkip;
}
#endif












