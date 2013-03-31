#include <convert/convertVatic.h>
#include <fstream>
#include <iostream>

using namespace cvac;
using namespace std;

LabelableList* cvac::convert(std::string filePath) {

	fstream inputFile;
	inputFile.open(filePath.c_str(), ios::in);
    if(!inputFile.is_open()) {
		printf("Could not open data file for convert: %s\n", filePath.c_str());
    }


	LabelableList* returnList = NULL;
	inputFile.close();
	return(returnList);
}