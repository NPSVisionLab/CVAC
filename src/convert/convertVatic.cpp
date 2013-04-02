#include <convert/convertVatic.h>

using namespace cvac;
using namespace std;

LabelableList* cvac::convert(std::string filePath) {

	map<int, LabeledTrack*> objectTracks;
	LabelableList* anntdTracks = new LabelableList();  // push_back video labels into this list
	FrameLocationList* f = new FrameLocationList();
	fstream inputFile;
	string inputLine, labelStr;
	int trackObjId, xMin, yMin, xMax, yMax, frameIdNum, isLostInt, isOccludedInt, isGeneratedInt;
	boolean isLost = false, isOccluded = false, isGenerated = false;

	anntdTracks->clear();
	inputFile.open(filePath.c_str(), ios::in);
    if(!inputFile.is_open()) {
		localAndClientMsg(VLogger::WARN, NULL, "Could not open data file for convert: %s\n", filePath.c_str());
    }
	localAndClientMsg(VLogger::DEBUG_1, NULL, "Convert file is open.  %s\n", filePath.c_str());


	while(std::getline(inputFile, inputLine)) {

		// parse 9 ints then string
		stringstream ss(inputLine);
		ss >>	trackObjId >> xMin >> yMin >> xMax >> yMax >> frameIdNum >>	// ints
				isLostInt >> isOccludedInt >> isGeneratedInt >>  // Represent boolean
				labelStr;
		if(0 != isLostInt) {
			isLost = true;
		}
		if(0 != isOccludedInt) {
			isOccluded = true;
		}
		if(0 != isGeneratedInt) {
			isGenerated = true;
		}

		// Record frame number as frame-location timestamp
		FrameLocation frLoc;
		frLoc.frame.framecnt = frameIdNum;

		// Compute and record bounding box
		int boxHeight = (yMax - yMin);
		if(boxHeight < 0) {
			throw new exception("Problem converting Vatic data file: (boxHeight < 0)");
		}
		int boxWidth  = (xMax - xMin);
		if(boxWidth < 0) {
			throw new exception("Problem converting Vatic data file: (boxWidth < 0)");
		}
		
		BBoxPtr bbox = new BBox();				         // Fill-in bbox with parsed data
		bbox->height = boxHeight;
		bbox->width = boxWidth;
		bbox->x = xMin;
		bbox->y = yMin;
		frLoc.loc = bbox;
		

		LabeledTrack* curTrack;
		if(objectTracks.end() == objectTracks.find(trackObjId)) {  // Insert for new Object Id
			printf("objectTracks[%d]: inserting item.\n", trackObjId);
			objectTracks.insert(std::pair<int, LabeledTrack*>(trackObjId, new LabeledTrack()));
		}
		curTrack = objectTracks[trackObjId];
		if(NULL == curTrack) {
			throw new exception("LabeledTrack-map did not contain key for: 'Track Obj Id'");
		}

	////////////////////////////////////////////////////////////////
		// Function for: ConvertUtils.cpp
		Substrate trackSub;
		trackSub.isVideo = true;
		trackSub.isImage = false;

		FrameLocationList frameLabelsList;
		frameLabelsList.push_back(frLoc);  // Prepared by caller

		LabeledTrack* labeledTrk = new LabeledTrack();
		labeledTrk->confidence = 0;  // default value(?)
		labeledTrk->sub = trackSub;
		labeledTrk->keyframesLocations = frameLabelsList;
		labeledTrk->interp = DISCRETE;
	////////////////////////////////////////////////////////////////
	}

	// Insert map entries to main list
	for(map<int, LabeledTrack*>::iterator itr=objectTracks.begin(); itr!=objectTracks.end(); ++itr) {
	       
		LabeledTrack* curMapTrack = (*itr).second;
		
		//printf("Pushing back, height: %d\n", curMapTrack->sub.height);
		printf("List pushing back item.\n");
		anntdTracks->push_back(curMapTrack);
	}

	inputFile.close();
	return(anntdTracks);

return(NULL);//tmp
}