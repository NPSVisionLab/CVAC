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
#include "SBDICETrainI.h"
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
using namespace cvac;

SBDICETrainI::SBDICETrainI()
: pSBDCV(NULL),fInitialized(false)
{
	pSBDCV = new SBDCV();
}

SBDICETrainI::~SBDICETrainI()
{
	delete pSBDCV;
	pSBDCV = NULL;
}
::TrainerPropertiesPrx SBDICETrainI::getTrainerProperties(const ::Ice::Current& current)
{
	return NULL;
}


void SBDICETrainI::initialize(::Ice::Int verbosity,const ::Ice::Current& current)
{
	Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
	vLogger.setLocalVerbosityLevel(props->getProperty("CVAC.ServicesVerbosity"));
	
	// Load the CVAC property: 'CVAC.DataDir'  Used for the xml filename path, and to correct Runset paths
	std::string m_CVAC_DataDir = props->getProperty("CVAC.DataDir");
	if(m_CVAC_DataDir.empty()){
		localAndClientMsg(VLogger::WARN, NULL, "Unable to locate CVAC Data directory, specified: 'CVAC.DataDir = path/to/dataDir' in </CVAC_Services/config.service>\n");
	}
	localAndClientMsg(VLogger::DEBUG, NULL, "CVAC Data directory configured as: %s \n", m_CVAC_DataDir.c_str());

	fInitialized = true;
}

bool SBDICETrainI::isInitialized(const ::Ice::Current& current)
{
	return fInitialized;
}

void SBDICETrainI::destroy(const ::Ice::Current& current)
{
	if(pSBDCV != NULL)
		delete pSBDCV;
	pSBDCV = NULL;

	fInitialized = false;
}
std::string SBDICETrainI::getName(const ::Ice::Current& current)
{
	return "SBDTrain";
}
std::string SBDICETrainI::getDescription(const ::Ice::Current& current)
{
	return "SBDTrain - Empty Description";
}

void SBDICETrainI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{

}

//ResultSetV2
bool SBDICETrainI::processTrain(DetectorTrainerPtr trainer,string _filepath,string _videoFilename,std::vector<long>& _videoFrames,bool _doInitialize)
{	
	SBDICETrainI* _SBDCV = static_cast<SBDICETrainI*>(trainer.get());
    
	return _SBDCV->pSBDCV->train_extractFeature(_filepath,_videoFilename,_videoFrames,_doInitialize);
}

void SBDICETrainI::process(const Ice::Identity &client, const ::RunSet& runset,const ::Ice::Current& current)
{
	TrainerCallbackHandlerPrx _callback = TrainerCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());

	localAndClientMsg(VLogger::INFO, _callback, "Training process is started. \n");

	std::string _filepath;
	std::string _videoFilename;
	std::vector<long> _videoFrames;	
	std::string _resMsg;
	int _classID(0);
    
	Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
    std::string CVAC_DataDir = props->getProperty("CVAC.DataDir");

	if(runset.purposedLists.size() == 0)
	{
		_resMsg = "Error: no data (runset) for processing\n";
		localAndClientMsg(VLogger::WARN, _callback, _resMsg.c_str());
		return;
	}
	
	for (size_t i = 0; i < runset.purposedLists.size();i++)
	{
		_classID = runset.purposedLists[i]->pur.classID;
		PurposedLabelableSeq* lab = static_cast<PurposedLabelableSeq*>(runset.purposedLists[i].get());

		if(lab->labeledArtifacts.size() == 0)
		{
			_resMsg = "Error: no real data (in a runset) for processing\n";
			localAndClientMsg(VLogger::WARN, _callback, _resMsg.c_str());
			return;
		}

		for (size_t i =0; i< lab->labeledArtifacts.size(); i++)
		{
			LabeledTrack* videoLabel = static_cast<LabeledTrack*>(lab->labeledArtifacts[i].get());

			_videoFilename = videoLabel->sub.path.filename;
			_filepath = videoLabel->sub.path.directory.relativePath;
			_filepath = expandFilename(_filepath, CVAC_DataDir);
			_videoFrames.clear();

			for(size_t k=0; k< videoLabel->keyframesLocations.size();k++)
				_videoFrames.push_back(videoLabel->keyframesLocations[k].frame.time);

			if(_videoFrames.size() == 0)
			{
				_resMsg = "Error for processing: " + _filepath + "/" + _videoFilename + ": inappropriate frame lists\n";
				localAndClientMsg(VLogger::WARN, _callback, _resMsg.c_str());
				return;
			}

			_resMsg = _filepath + "/" + _videoFilename + " is processing ... \n";
			localAndClientMsg(VLogger::DEBUG, _callback, _resMsg.c_str());

			if(SBDICETrainI::processTrain(this,_filepath,_videoFilename,_videoFrames,(i==0)?true:false))
			{
				_resMsg = _filepath + "/" + _videoFilename + " is processed. \n";
				localAndClientMsg(VLogger::DEBUG, _callback, _resMsg.c_str());
			}
			else
			{
				_resMsg = "Error for processing: " + _filepath + "/" + _videoFilename + "\n";
				localAndClientMsg(VLogger::WARN, _callback, _resMsg.c_str());
			}
		}
	}

	pSBDCV->train_trainSVM();
	//pSBDCV->train_saveFeature(_filepath,svmResult_feature);	//Because a zip file will be used for saving all training results, this part would be skipped.
	pSBDCV->train_saveSVM(_filepath,svmResult_xml);

	DetectorData detectorData;
	//Method 1
	detectorData.type = ::cvac::FILE;
	detectorData.file.directory.relativePath = _filepath;
	detectorData.file.filename = svmResult_xml;

	// Method 2
// 	std::vector<std::string> strSeq;
// 	strSeq.push_back(_filepath);
// 	//strSeq.push_back(svmResult_feature);	//Because a zip file will be used for saving all training results, this part would be skipped.
// 	strSeq.push_back(svmResult_xml);
// 
// 	Ice::OutputStreamPtr out = Ice::createOutputStream(current.adapter->getCommunicator());	//communicator()
// 	out->write(strSeq);
// 	ByteSeq seq;
// 	out->finished(seq);
// 	detectorData.data = seq;

	_callback->createdDetector(detectorData);	//Sending a message for noticing a name of a saved file

	localAndClientMsg(VLogger::INFO, _callback, "Training process is done. \n");
}
