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
#include "VideoValidatorForOpenCVI.h"
#include <iostream>
#include <vector>

#include <Ice/Identity.h>
#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>

VideoValidatorForOpenCVI::VideoValidatorForOpenCVI()
: pValidate(NULL),fInitialized(false)
{
	pValidate = new VideoValidatorForOpenCV();
}

VideoValidatorForOpenCVI::~VideoValidatorForOpenCVI()
{
	delete pValidate;
	pValidate = NULL;
}
::cvac::TrainerPropertiesPrx VideoValidatorForOpenCVI::getTrainerProperties(const ::Ice::Current& current)
{	
	return NULL;
}


void VideoValidatorForOpenCVI::initialize(::Ice::Int verbosity,const ::Ice::Current& current)
{
	fInitialized = true;
}

bool VideoValidatorForOpenCVI::isInitialized(const ::Ice::Current& current)
{
	return fInitialized;
}

void VideoValidatorForOpenCVI::destroy(const ::Ice::Current& current)
{
	if(pValidate != NULL)
		delete pValidate;
	pValidate = NULL;

	fInitialized = false;
}
std::string VideoValidatorForOpenCVI::getName(const ::Ice::Current& current)
{
	return "VideoValidatorForOpenCV";
}
std::string VideoValidatorForOpenCVI::getDescription(const ::Ice::Current& current)
{
	return "VideoValidatorForOpenCV - Empty Description";
}

void VideoValidatorForOpenCVI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{

}

//cvac::ResultSetV2
string VideoValidatorForOpenCVI::processTrain(cvac::DetectorTrainerPtr trainer,string _filepath,string _videoFilename,bool& _flagSuccess)
{	
 	VideoValidatorForOpenCVI* _validator = static_cast<VideoValidatorForOpenCVI*>(trainer.get());
	std::string _returnMsg;
 	_flagSuccess = _validator->pValidate->runTest(_filepath,_videoFilename,_returnMsg);

	//std::string _result = _filepath + "/" + _videoFilename + ": " + _returnMsg;
	std::string _result = _returnMsg;
	if(!_flagSuccess)
	{
		_result += " Your system may show wrong results for running video-processing.";
		_result += " On your system, ffmpeg may not be used. Please, check use of ffmpeg.";
	}
	return _result;
}

void VideoValidatorForOpenCVI::process(const Ice::Identity& client,const ::cvac::RunSet& runset,const ::Ice::Current& current)
{
    const cvac::TrainerCallbackHandlerPrx& callbackHandler =
            cvac::TrainerCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client));
	callbackHandler->message(0,"Testing procedure is starting.. ");

	std::string _filepath;
	std::string _videoFilename;
	std::vector<long> _videoFrames;
	int _classID(0);
	std::string _resStr;
	for (size_t i = 0; i < runset.purposedLists.size();i++)
	{
		_classID = runset.purposedLists[i]->pur.classID;
		cvac::PurposedLabelableSeq* lab = static_cast<cvac::PurposedLabelableSeq*>(runset.purposedLists[i].get());

		for (size_t i =0; i< lab->labeledArtifacts.size(); i++)
		{
			cvac::LabeledTrack* videoLabel = static_cast<cvac::LabeledTrack*>(lab->labeledArtifacts[i].get());

			_videoFilename = videoLabel->sub.path.filename;
			_filepath = videoLabel->sub.path.directory.relativePath;

			_videoFrames.clear();
			for(size_t k=0; k< videoLabel->keyframesLocations.size();k++)
				_videoFrames.push_back(videoLabel->keyframesLocations[k].frame.time);

			bool _flagSuccess;
			_resStr =  VideoValidatorForOpenCVI::processTrain(this,_filepath,_videoFilename,_flagSuccess);
			callbackHandler->message(0,_resStr);
		}
	}

	callbackHandler->message(0,"Testing process is done..");
}
