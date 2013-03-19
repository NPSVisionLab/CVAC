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
#include "BowICETrainI.h"
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
using namespace cvac;

BowICETrainI::BowICETrainI()
: pBowCV(NULL),fInitialized(false)
{
	
	pBowCV = new bowCV();
}

BowICETrainI::~BowICETrainI()
{
	delete pBowCV;
	pBowCV = NULL;
}

void BowICETrainI::initialize(::Ice::Int verbosity,const ::Ice::Current& current)
{
	//lekomin: how to get these initial and tunable parameters
	string	_nameFeature("SURF");	//SURF, SIFT, FAST, STAR, MSER, GFTT, HARRIS
	string	_nameDescriptor("SURF");	//SURF, SIFT, OpponentSIFT, OpponentSURF
	string	_nameMatcher("BruteForce-L1");	//BruteForce-L1, BruteForce, FlannBased  
	int		_countWords = 150;	

	// Load CVAC verbosity
	Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
	vLogger.setLocalVerbosityLevel(props->getProperty("CVAC.ServicesVerbosity"));

	if(pBowCV->train_initialize(_nameFeature,_nameDescriptor,_nameMatcher,_countWords))
		fInitialized = true;
	else
		fInitialized = false;	
}

bool BowICETrainI::isInitialized(const ::Ice::Current& current)
{
	return fInitialized;
}

void BowICETrainI::destroy(const ::Ice::Current& current)
{
	if(pBowCV != NULL)
		delete pBowCV;
	pBowCV = NULL;

	fInitialized = false;
}
std::string BowICETrainI::getName(const ::Ice::Current& current)
{
	return "bowTrain";
}
std::string BowICETrainI::getDescription(const ::Ice::Current& current)
{
	return "BOW - Empty Description";
}

void BowICETrainI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{

}
::TrainerPropertiesPrx BowICETrainI::getTrainerProperties(const ::Ice::Current &current)
{
    return NULL;
}


//ResultSetV2
string BowICETrainI::processSingleImg(DetectorTrainerPtr trainer,string _filepath,string _filename,int _classID,const ::LocationPtr& _ploc)
{
	std::string _strFilepath = std::string(_filepath);
	std::string _strFilename = std::string(_filename);	
	std::string _strFullname(_strFilepath + "/" + _strFilename);

	std::ostringstream _ostr;	_ostr << _classID;
	std::string _strClassID(_ostr.str());

	BowICETrainI* _bowCV = static_cast<BowICETrainI*>(trainer.get());

	std::string _resMsg;
	if(_ploc == NULL)
		_bowCV->pBowCV->train_stackTrainImage(_strFullname,atoi(_strClassID.c_str()));
	else
	{
		if(_ploc->ice_isA("::cvac::BBox"))
		{
			BBoxPtr pbox = BBoxPtr::dynamicCast(_ploc);		
			_bowCV->pBowCV->train_stackTrainImage(_strFullname,atoi(_strClassID.c_str()),pbox->x,pbox->y,pbox->width,pbox->height);
		}
		else
		{
			_resMsg = "Error: " + _strFullname + " is not added to a class because only cvac::BBox type is accepted \n";
			return _resMsg;
		}		
	}	
	
	std::ostringstream _omsg;	
	_omsg << _strFilename << " is added into class: |" << _strClassID << "| for training. \n";	
	_resMsg = _omsg.str();

	//localAndClientMsg(VLogger::DEBUG, NULL, "%s is added into class: |%s| for training. \n", _strFilename.c_str(), _strClassID.c_str());
	//localAndClientMsg(VLogger::DEBUG, NULL, _resMsg.c_str());

	return _resMsg;
}



void BowICETrainI::process(const Ice::Identity &client,const ::RunSet& runset,const ::Ice::Current& current)
{	
	TrainerCallbackHandlerPrx _callback = TrainerCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());		

	std::string _filepath;
	std::string _filename;	
	std::string _resStr;
	int _classID;

	Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
	std::string CVAC_DataDir = props->getProperty("CVAC.DataDir");

	if(runset.purposedLists.size() == 0)
	{
		_resStr = "Error: no data (runset) for processing\n";
		localAndClientMsg(VLogger::WARN, _callback, _resStr.c_str());
		return;
	}

	for (size_t i = 0; i < runset.purposedLists.size();i++)
	{
		_classID = runset.purposedLists[i]->pur.classID;
		PurposedLabelableSeq* lab = static_cast<PurposedLabelableSeq*>(runset.purposedLists[i].get());

		if(lab->labeledArtifacts.size() == 0)
		{
			_resStr = "Error: no real data (in a runset) for processing\n";
			localAndClientMsg(VLogger::WARN, _callback, _resStr.c_str());
			return;
		}

		for (size_t i =0; i< lab->labeledArtifacts.size(); i++)
		{						
			_filename = lab->labeledArtifacts[i]->sub.path.filename;
			_filepath = lab->labeledArtifacts[i]->sub.path.directory.relativePath;
			_filepath = expandFilename(_filepath, CVAC_DataDir);
		
			if(lab->labeledArtifacts[i]->ice_isA("::cvac::LabeledLocation"))
			{
				LabeledLocationPtr plabeledLocation = LabeledLocationPtr::dynamicCast(lab->labeledArtifacts[i]);
				LocationPtr pLoc = plabeledLocation->loc;				
				_resStr =  BowICETrainI::processSingleImg(this,_filepath,_filename, _classID,pLoc);		
			}
			else
			{
				_resStr =  BowICETrainI::processSingleImg(this,_filepath,_filename, _classID,NULL);		
			}
					
			localAndClientMsg(VLogger::DEBUG, _callback, _resStr.c_str());
		}
	}

	localAndClientMsg(VLogger::INFO, _callback, "Training procedure is started.. \n");	//lekomin_suspended

	pBowCV->train_run(_filepath, logfile_BowTrainResult);	

	DetectorData detectorData;
	// Method 1	
	detectorData.type = ::cvac::FILE;
 	detectorData.file.directory.relativePath = _filepath;
 	detectorData.file.filename = logfile_BowTrainResult;

	// Method 2
// 	std::vector<std::string> strSeq;
// 	strSeq.push_back(_filepath);
// 	strSeq.push_back(logfile_BowTrainResult);
// 	Ice::OutputStreamPtr out = Ice::createOutputStream(current.adapter->getCommunicator());	//communicator()
// 	out->write(strSeq);
// 	ByteSeq seq;
// 	out->finished(seq);
// 	detectorData.data = seq;	

	_callback->createdDetector(detectorData);

	localAndClientMsg(VLogger::INFO, _callback, "Training procedure is done..\n");
}
