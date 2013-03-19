/******************************************************************************
 * CVAC Software Disclaimer
 * 
 * This software was developed at the Naval Postgraduate School, Monterey, CA,
 * by employees of the Federal Government in the course of their official duties.
 * Pursuant to title 17 Section 105 of the United States Code this software
 * is not subject to copyright protection and is in the public domain. It is 
 * an experimental system.  The Naval Postgraduate School assumes no
 * responsibility whatsoever for its use by other parties, and makes
 * no guarantees, expressed or implied, about its quality, reliability, 
 * or any other characteristic.
 * We would appreciate acknowledgement and a brief notification if the software
 * is used.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Naval Postgraduate School, nor the name of
 *       the U.S. Government, nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without
 *       specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "SBDICETestI.h"
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/DetectorDataArchive.h>
#include <util/ServiceMan.h>
using namespace cvac;

std::string SBDICETestI::m_CVAC_DataDir = ""; // Stored on initilization of Detector
int SBDICETestI::m_verbosity = 0; // Implementation of static class variable

SBDICETestI::SBDICETestI()
: pSBDCV(NULL),fInitialized(false)
{

}

SBDICETestI::~SBDICETestI()
{
	delete pSBDCV;
	pSBDCV = NULL;
}

void SBDICETestI::initialize(::Ice::Int verbosity, const ::DetectorData& data, const ::Ice::Current& current)
{
    // Since constructor only called on service start and destroy can be called.  We need to make sure we have it
    if (pSBDCV == NULL)
        pSBDCV = new SBDCV();

	pSBDCV->osxTest();	//restore after fixing osx problem
	
	m_verbosity = (int)verbosity; // Save verbosity state within SBDICETestI
	
	
	Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
	vLogger.setLocalVerbosityLevel(props->getProperty("CVAC.ServicesVerbosity"));

	// Load the CVAC property: 'CVAC.DataDir'.  Used for the svm filename path, and to correct Runset paths
	m_CVAC_DataDir = props->getProperty("CVAC.DataDir");
	if(m_CVAC_DataDir.empty())
	{
		localAndClientMsg(VLogger::WARN, NULL, "Unable to locate CVAC Data directory, specified: 'CVAC.DataDir = path/to/dataDir' in </CVAC_Services/config.service>\n");
	}
	localAndClientMsg(VLogger::DEBUG, NULL, "CVAC Data directory configured as: %s \n", m_CVAC_DataDir.c_str());

	std::string _extFile = data.file.filename.substr(data.file.filename.rfind(".")+1,data.file.filename.length());
  localAndClientMsg(VLogger::DEBUG_2, NULL, "_extFile string: %s\n", _extFile.c_str());
	if (_extFile.compare("gz") == 0)
	{
		//////////////////////////////////////////////////////////////////////////
		// Setup: loading the SVM result from a .gz file  Note: no outer (.zip) wrapper in this case
    localAndClientMsg(VLogger::DEBUG, NULL, "Initializing Shot Boundary Detection from gz file: %s/%s\n", data.file.directory.relativePath.c_str(), data.file.filename.c_str());			

		//if(pSBDCV->test_loadSVM(strSeq[0],svmResult_xml))
		if(pSBDCV->test_loadSVM(data.file.directory.relativePath, data.file.filename))
		{
			fInitialized = true;
      localAndClientMsg(VLogger::DEBUG_2, NULL, "test_loadSVM was able to load SVM.  Initialized.");
		}
		else
		{
			fInitialized = false;
      localAndClientMsg(VLogger::DEBUG, NULL, "test_loadSVM was not able to load SVM.  Not initialized.");
		}
	}
	else // Uncompress (.zip) wrapper file like other CVAC detectors
	{
		// Use utils un-compression to get extracted file names
		std::string archiveFilePath; 
		if ((data.file.directory.relativePath.length() > 1 && data.file.directory.relativePath[1] == ':' )||
			data.file.directory.relativePath[0] == '/' ||
			data.file.directory.relativePath[0] == '\\')
		{  // absolute path
			archiveFilePath = data.file.directory.relativePath + "/" + data.file.filename;
		}
		else
		{ // prepend our prefix
			archiveFilePath = (m_CVAC_DataDir + "/" + data.file.directory.relativePath + "/" + data.file.filename);
		}

		std::vector<std::string> fileNameStrings =  expandSeq_fromFile(archiveFilePath, getName(current));
		
		// Need to strip off extra zeros
		std::string cwDirectory = std::string(getCurrentWorkingDirectory().c_str());
		std::string detectorNameStr = getName(current);
		std::string xmlFilename = fileNameStrings[0]; // Only 1 data file in the zip

		std::string dpath;
		//dpath.reserve(cwDirectory.length() + detectorNameStr.length() + 3);  //1
    dpath.reserve(detectorNameStr.length() + 3);  // Use only relative path
		//dpath += cwDirectory; //2
		//dpath += std::string("/"); //3
		dpath += ".";
		dpath += detectorNameStr;

    localAndClientMsg(VLogger::DEBUG, NULL, "SBD: loading SVM: dpath/filename: %s/%s\n", dpath.c_str(), xmlFilename.c_str());
    localAndClientMsg(VLogger::DEBUG, NULL, "SBD, cwd: %s\n", cwDirectory.c_str());
		if(pSBDCV->test_loadSVM(dpath, xmlFilename))
		{
			fInitialized = true;
      localAndClientMsg(VLogger::DEBUG, NULL, "SVM-load OK.\n");
		}
		else
		{
			fInitialized = false;
      localAndClientMsg(VLogger::DEBUG, NULL, "SVM-load failed.\n");
		}
	}
}


bool SBDICETestI::isInitialized(const ::Ice::Current& current)
{
	return fInitialized;
}

void SBDICETestI::destroy(const ::Ice::Current& current)
{
	if(pSBDCV != NULL)
		delete pSBDCV;
	pSBDCV = NULL;

	fInitialized = false;
}
std::string SBDICETestI::getName(const ::Ice::Current& current)
{
	return "SBDTest";
}
std::string SBDICETestI::getDescription(const ::Ice::Current& current)
{
	return "SBD Test - Empty Description";
}

void SBDICETestI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{

}

DetectorData SBDICETestI::createCopyOfDetectorData(const ::Ice::Current& current)
{
	// need to update (lekomin)
	DetectorData data;
	return data;
}

DetectorPropertiesPrx SBDICETestI::getDetectorProperties(const ::Ice::Current& current)
{
	// need to update (lekomin)
	return NULL;
}

ResultSetV2 SBDICETestI::processSingleVideo(DetectorPtr detector, const char* fullfilename)
{
	bool isVideoFile = false;
	std::string fullfilename_str = std::string(fullfilename);
	int _idxPeriod = fullfilename_str.rfind(".");
	std::string _extension = fullfilename_str.substr( (_idxPeriod + 1),fullfilename_str.length());

	if  (!strcmp(_extension.c_str(),"mpg")  || !strcmp(_extension.c_str(),"mpeg") || !strcmp(_extension.c_str(),"mp4")) 
	{
		isVideoFile = true;
	}

	if(isVideoFile)
	{
		std::string _ffullname = std::string(fullfilename);

		localAndClientMsg(VLogger::DEBUG, NULL, "%s is processing\n", _ffullname.c_str());

		SBDICETestI* _SBDCV = static_cast<SBDICETestI*>(detector.get());

		std::vector<long> _vFrameListOutput;
		std::string _resultFileName;

		_SBDCV->pSBDCV->test_run(_ffullname, _vFrameListOutput, true, _resultFileName, false);  
		//the third parameter: whether to export the result frames to a TXT file
		//the fifth parameter: whether to save the boundary images

		ResultSetV2 _resSet;	
		Result _tResult;
		_tResult.original = new Labelable();
		_tResult.original->sub.path.filename = _ffullname;
		// Add a LabeledVideo for each frame we found

		for(size_t k=0;k<_vFrameListOutput.size();k++)
		{
			LabeledVideoSegment *vlabel = new LabeledVideoSegment();
			vlabel->start.time = _vFrameListOutput[k];			
			vlabel->last.time = NULL;
			vlabel->loc = NULL;					
			_tResult.foundLabels.push_back(vlabel);						
		}
    
		/*LabeledTrack* videoLabel = new LabeledTrack();	
		videoLabel->sub.path.filename = _resultFileName;	*/

		/*for(size_t k=0;k<_vFrameListOutput.size();k++)
		{			
		FrameLocation _frameIdx;		
		_frameIdx.frame.time = _vFrameListOutput[k];		
		videoLabel->keyframesLocations.push_back(_frameIdx);						
		}
		videoLabel->interp = DISCRETE;
		_tResult.foundLabels.push_back(videoLabel);*/

		localAndClientMsg(VLogger::DEBUG, NULL, "%s is processed.\n", _ffullname.c_str());

 		_resSet.results.push_back(_tResult);

		return _resSet;
	}
	else
	{ // Not a valid video file
    localAndClientMsg(VLogger::DEBUG_3, NULL, "Not adding file with unaccepted format: %s.  Accepts: 'mpg', 'mpeg', 'mp4'\n", fullfilename_str.c_str());

		ResultSetV2 empty;
		return empty;
	}
}

void SBDICETestI::process(const Ice::Identity &client,const ::RunSet& runset,const ::Ice::Current& current)
{
	DetectorCallbackHandlerPrx _callback = DetectorCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());
	DoDetectFunc func = SBDICETestI::processSingleVideo;
	processRunSet(this, _callback, func, runset, m_CVAC_DataDir);
}
