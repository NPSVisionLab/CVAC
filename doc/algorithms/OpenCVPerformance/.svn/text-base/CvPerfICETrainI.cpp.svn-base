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
#include "CvPerfICETrainI.h"
#include "file_open_options.h"
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>

// Use these filenames as internal trainer-knowledge
const char* INFO_FILE_NAME = "CvPerf_Trainer_Runset.dat";
const char* VEC_FILE_NAME = "CvPerf_Trainer_Samples.vec";
const char* NEG_FILE_NAME = "CvPerf_BackgroundImgs.txt";
const char* CV_PERFORMANCE_DIR = "trainResult";

CvPerf_ICETrainI::CvPerf_ICETrainI()
{
  initFlag = false;
}

CvPerf_ICETrainI::~CvPerf_ICETrainI()
{
}

void CvPerf_ICETrainI::initialize(::Ice::Int verbosity, const ::Ice::Current& current)
{
  cvac::localAndClientMsg(VLogger::INFO, NULL, "CVAC_OpenCV_Trainer Service initialized. \n");
  Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());

  cvac::vLogger.setLocalVerbosityLevel(props->getProperty("CVAC.ServicesVerbosity"));

  cvPerfTrainer = new CvPerfTrainer();
  initFlag = true;
}

bool CvPerf_ICETrainI::isInitialized(const ::Ice::Current& current)
{
	return initFlag;
}

void CvPerf_ICETrainI::destroy(const ::Ice::Current& current)
{
  if(NULL != cvPerfTrainer) {
    delete(cvPerfTrainer);
    cvPerfTrainer = NULL;
  }
	initFlag = false;
}

std::string CvPerf_ICETrainI::getName(const ::Ice::Current& current)
{
	return "CVAC_OpenCV_Trainer Service";
}
std::string CvPerf_ICETrainI::getDescription(const ::Ice::Current& current)
{
	return "CVAC_OpenCV_Trainer Service - Empty Description";
}

void CvPerf_ICETrainI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{
  // ToDo: add ICE-verbosity
}

// First convert samples from runset, then use OpenCV library call instead of command line
void CvPerf_ICETrainI::process(const Ice::Identity& client, const ::cvac::RunSet& runset, const ::Ice::Current& current)
{
  TrainerCallbackHandlerPrx callbackHandler =
          TrainerCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());
  ::cvac::RunSet tempRunSet = runset;
  ::cvac::DetectorData ddata;
  const int TRAINING_SEARCH_W = 20, TRAINING_SEARCH_H = 20,
            NUM_TRAIN_STAGES = 14;
  cvac::localAndClientMsg(VLogger::DEBUG, callbackHandler, "Begin OpenCv Haar training\n");
  std::string tempDir = cvac::fixupRunSet(tempRunSet);
  Pos_neg_counts numImgsFound = cvPerfTrainer->createOpenCvSamples_datafiles(tempRunSet, TRAINING_SEARCH_W,
                                                                 TRAINING_SEARCH_H, callbackHandler, std::string(INFO_FILE_NAME), 
                                                                 std::string(VEC_FILE_NAME), std::string(NEG_FILE_NAME));
  // clean-up all intermediate trainer output directories from previous runs
  std::string cleanUpTrainerPath("trainResult");
  deleteDirectory(cleanUpTrainerPath);
  // Prepare OpenCV samples
  int numPreparedSamples = numImgsFound.numPosImgs;
  cvPerfTrainer->prepareTrainingSamples(INFO_FILE_NAME, VEC_FILE_NAME,  // Phase 1 of 'violaJones.properties': opencv_createsamples
                                        numPreparedSamples, 
                                        TRAINING_SEARCH_W, TRAINING_SEARCH_H);

  // Run training
  cvPerfTrainer->trainDataset(callbackHandler, CV_PERFORMANCE_DIR,
                              VEC_FILE_NAME, NEG_FILE_NAME,
                              TRAINING_SEARCH_W, TRAINING_SEARCH_H,  // Same search window as prepare-samples
                              numImgsFound.numPosImgs, numImgsFound.numNegImgs,
                              NUM_TRAIN_STAGES);
  cvac::localAndClientMsg(VLogger::DEBUG, callbackHandler, "Done training.  Xml result stored to: 'trainResult.xml' \n");


  // XML file is written in library call 'cvCreateTreeCascadeClassifier' to be one level up from '/trainResult'
  ::FILE *xml = fopen("trainResult.xml", trainOpenStr);
  if(!xml)
	{
    cvac::localAndClientMsg(VLogger::WARN, callbackHandler, "Error, Unable to open file: 'trainResult.xml' after training.\n");
		return;
	}
  cvac::localAndClientMsg(VLogger::DEBUG_1, callbackHandler, "Writing back detector XML filename over Ice communicator using 'createdDetector(' \n");

  ddata.type = cvac::FILE;  // Full path makes it into Java String 'm_ddata.file.filename' for Detector XML in 'IceTrainer.java'
  std::string res = std::string(getCurrentWorkingDirectory().c_str());
  res.append("/trainResult.xml");
  ddata.file.filename = res;
  callbackHandler->createdDetector(ddata);
  cvac::deleteDirectory(tempDir);
}

::cvac::TrainerPropertiesPrx CvPerf_ICETrainI::getTrainerProperties(
                             const ::Ice::Current& current)
{
  return NULL;
}

const char* CvPerf_ICETrainI::getInfoFilename()     
{ 
  return(INFO_FILE_NAME); 
}
    
const char* CvPerf_ICETrainI::getVecFilename()      
{ 
  return(VEC_FILE_NAME);  
}

const char* CvPerf_ICETrainI::getNeg_bg_Filename()
{
  return(NEG_FILE_NAME);  
}

