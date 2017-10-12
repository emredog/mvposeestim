#include "estimateyrerrorcnn.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "top.h"

using namespace std;

EstimateYrErrorCNN::EstimateYrErrorCNN(const char *csvFileName)
{
    this->init(csvFileName);
}

#ifdef HUMANEVA
std::vector<double> EstimateYrErrorCNN::getErrors(string subject, string action, string view, int frame)
{
    string keyForBase = subject; // on the csv file, 1st: subject, 2nd: action, 3rd: frame, 4th: view
    keyForBase.append(action);
    char buffer[10];
    sprintf(buffer,"%d",frame);
    keyForBase.append(string(buffer));
    keyForBase.append(view);

    std::map<std::string, std::vector<FPTYPE> >::iterator it;
    it = this->errorBase.find(keyForBase);

    if (it != this->errorBase.end())
        return it->second;
    else
    {
        cerr << "Estimate YR Error - CNN : Not found!" << keyForBase.c_str() << endl;
        return std::vector<FPTYPE>();
    }
}
#endif

#ifdef UMPM
std::vector<double> EstimateYrErrorCNN::getErrors(string action, string view, int frame)
{
    string keyForBase = action; // on the csv file, 1st: action, 2nd: frame, 3rd: view
    char buffer[10];
    sprintf(buffer,"%d",frame);
    keyForBase.append(string(buffer));
    keyForBase.append(view);

    std::map<std::string, std::vector<FPTYPE> >::iterator it;
    it = this->errorBase.find(keyForBase);

    if (it != this->errorBase.end())
        return it->second;
    else
    {
        cerr << "Estimate YR Error - CNN : Not found!" << keyForBase.c_str() << endl;
        return std::vector<FPTYPE>();
    }
}
#endif

void EstimateYrErrorCNN::init(const char *csvFileName)
{
    top(0);
    ifstream file;
    file.open(csvFileName);
    long lineCounter = 0;
    //const std::string nanStr = "NaN";

    if (!file.is_open())
    {
        cerr << "ERROR Cannot open file: " << csvFileName << endl;
        return;
    }

    while (file) //for each line
    {
        string line;
        if (!getline(file, line)) //get the line
            break;

        stringstream strStream(line);


        string field;
        string keyForBase = "";
        vector<FPTYPE> valForBase;

        for (int col=0; col<CSV_COLUMNS; col++) //for each comma separated field after we read the index(29/30 cols)
        {

            //fetch the double value
            if (!getline(strStream, field, ','))
            {
                std::cerr << "ERROR Unexpected file format. Line: " << lineCounter << " in file: " << csvFileName << endl;
                std::cout << endl << endl << field.c_str() << endl << "of" << endl;
                std::cout << endl << endl << line.c_str() << endl << endl;
                std::cout << endl << endl << "CSV_COLUMNS: " << CSV_COLUMNS << endl << endl;
                std::cout << endl << endl << "KEY_COLS: " << KEY_COLS << endl << endl;
                std::cout << endl << endl << "keyForBase: " << keyForBase.c_str() << endl << endl;
                return;
            }

            if (col < KEY_COLS) // get key
            {
                keyForBase.append(field);
            }
            else
            {
                valForBase.push_back(atof(field.c_str()));
            }
        }

        this->errorBase.insert(std::pair<std::string, std::vector<FPTYPE> >(keyForBase, valForBase));


        lineCounter++;
    }

    FPTYPE duration = top(0);
    std::cout << "Estimate YR Error - CNN : CSV file with " << lineCounter << " lines is read in "<< duration/1000.0<< " seconds\n";

    file.close();

}

