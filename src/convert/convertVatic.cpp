#include <convert/convertVatic.h>
#include <util/FileUtils.h>
#include <fstream>
#include <iostream>

using namespace cvac;
using namespace std;

LabelableList* cvac::convert(std::string filePath) {

	LabelableList* returnList = new LabelableList();  // push_back video labels into this list
	fstream inputFile;
	string inputLine, labelStr;
	int trackId, xMin, yMin, xMax, yMax, frameId, isLostInt, isOccludedInt, isGeneratedInt;
	boolean isLost = false, isOccluded = false, isGenerated = false;

	returnList->clear();
	inputFile.open(filePath.c_str(), ios::in);
    if(!inputFile.is_open()) {
		localAndClientMsg(VLogger::WARN, NULL, "Could not open data file for convert: %s\n", filePath.c_str());
    }
	localAndClientMsg(VLogger::DEBUG_1, NULL, "Convert file is open.  %s\n", filePath.c_str());

	// parse 9 ints (last 3 are boolean), then string
	while(std::getline(inputFile, inputLine)) {

		stringstream ss(inputLine);
		ss >> trackId >> xMin >> yMin >> xMax >> yMax >> frameId >> isLostInt >> isOccludedInt >> isGeneratedInt >> labelStr;

		if(isLostInt) {
			isLost = true;
		}
		if(isLostInt) {
			isLost = true;
		}
		if(isOccludedInt) {
			isOccluded = true;
		}
		if(isGeneratedInt) {
			isGenerated = true;
		}

		// Create new LabeledVideoSegment, fill in data, push_back to LabelableList
		LabeledVideoSegment* baseSeg = new LabeledVideoSegment();
		baseSeg->lab.name = trackId;
		baseSeg->lab.hasLabel = true;

		Labelable* vidLabel = baseSeg;
		returnList->push_back(vidLabel);
	}


	inputFile.close();
	return(returnList);
}