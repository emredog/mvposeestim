#include "parttypecompat.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "top.h"

using namespace std;

PartTypeCompat::PartTypeCompat(const char *csvFileName, int nbOfPartTypes, int nbOfParts)
{
	this->nbOfPartTypes = nbOfPartTypes;
	this->nbOfParts = nbOfParts;

    this->partTypeScores =  new myArray<FPTYPE>(nbOfPartTypes, nbOfPartTypes, nbOfParts); // every plane is for a part

    this->init(csvFileName);
}

PartTypeCompat::~PartTypeCompat()
{
    if (this->partTypeScores)
        delete this->partTypeScores;
}



void PartTypeCompat::init(const char *csvFileName)
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

       	// read the first column (part index)
        string field;
        if (!getline(strStream, field, ','))
        {
            std::cerr << "ERROR Unexpected file format. Line: " << lineCounter << " in file: " << csvFileName << endl;
            return;
        }

        int partId = atoi(field.c_str());	
        int row = lineCounter % nbOfPartTypes;       
        FPTYPE value;

        for (int col=0; col<nbOfPartTypes; col++) //for each comma separated field after we read the index(6 cols)
        {

        	//fetch the double value
        	if (!getline(strStream, field, ','))
       		{
            	std::cerr << "ERROR Unexpected file format. Line: " << lineCounter << " in file: " << csvFileName << endl;
            	return;
        	}

        	value = atof(field.c_str()); //FIXME: handle NaN's from Matlab !!!!!

            this->partTypeScores->set(value, row, col, partId-1);
        }


        lineCounter++;
    }
    
    FPTYPE duration = top(0);
    std::cout << "Part type compatibility: CSV file with " << lineCounter << " lines is read in "<< duration/1000.0<< " seconds\n";
    
    file.close();

}

