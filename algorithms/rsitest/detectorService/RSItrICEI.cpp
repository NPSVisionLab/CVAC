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
#include "RSItrICEI.h"
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/DetectorDataArchive.h>
#include <util/ServiceManI.h>

using namespace cvac;


///////////////////////////////////////////////////////////////////////////////
// This is called by IceBox to get the service to communicate with.
extern "C"
{
  //
  // ServiceManager handles all the icebox interactions so we construct
  // it and set a pointer to our detector.
  //
  ICE_DECLSPEC_EXPORT IceBox::Service* create(Ice::CommunicatorPtr communicator)
  {
    RSItrICEI *rsitr = new RSItrICEI();
    ServiceManagerI *sMan = new ServiceManagerI( rsitr, rsitr );
    rsitr->setServiceManager( sMan );
    return sMan;
  }
}


///////////////////////////////////////////////////////////////////////////////

RSItrICEI::RSItrICEI()
: fInitialized(false), mRunsetWrapper(NULL), mRunsetIterator(NULL)
{
  mServiceMan = NULL; 
}

RSItrICEI::~RSItrICEI()
{
  if(mRunsetIterator != NULL)
    delete mRunsetIterator;
  mRunsetIterator = NULL;

  if(mRunsetWrapper != NULL)
    delete mRunsetWrapper;
  mRunsetWrapper = NULL;
}

void RSItrICEI::initialize(int verbosity,const ::cvac::FilePath &file,
                           const::Ice::Current &current)
{
  fInitialized = false;

  // Set CVAC verbosity according to ICE properties
  Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
  string verbStr = props->getProperty("CVAC.ServicesVerbosity");
  if (!verbStr.empty())
  {
    vLogger.setLocalVerbosityLevel( verbStr );
  }
  m_CVAC_DataDir = mServiceMan->getDataDir();  

  fInitialized = true;
  mRunsetConstraint.clear();
  mRunsetConstraint.addType("jpg");
  mRunsetConstraint.addType("png");    
  mRunsetConstraint.addType("tif");
}

bool RSItrICEI::isInitialized()
{
  return fInitialized;
}

void RSItrICEI::setServiceManager(cvac::ServiceManagerI *sman)
{
  mServiceMan = sman;
}

std::string RSItrICEI::getName(const ::Ice::Current& current)
{
  return "RSItrTest_Detector";
}


std::string RSItrICEI::getDescription(const ::Ice::Current& current)
{
  return "Runset Iterator Tester";
}

void RSItrICEI::destroy(const ::Ice::Current& current)
{
  if(mRunsetIterator != NULL)
    delete mRunsetIterator;
  mRunsetIterator = NULL;

  if(mRunsetWrapper != NULL)
    delete mRunsetWrapper;
  mRunsetWrapper = NULL;

  fInitialized = false;
}

bool RSItrICEI::cancel(const Ice::Identity &client, const ::Ice::Current& current)
{
  stopping();
  mServiceMan->waitForStopService();
  if (mServiceMan->isStopCompleted())
    return true;
  else 
    return false;
}

DetectorProperties RSItrICEI::getDetectorProperties(const ::Ice::Current& current)
{	
  //TODO get the real detector properties but for now return an empty one.
  DetectorProperties props;
  return props;
}

void RSItrICEI::starting()
{
  // Do anything needed on service starting
}

void RSItrICEI::stopping()
{
  mServiceMan->stopService();
}

void RSItrICEI::process(const Ice::Identity &client,const ::RunSet& runset,
                        const ::cvac::FilePath &detectorData,
                        const::cvac::DetectorProperties &props,
                        const ::Ice::Current& current)
{
  DetectorCallbackHandlerPrx _callback = 
    DetectorCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());
  
  initialize(props.verbosity, detectorData, current);

  if (!isInitialized())
  {
    localAndClientMsg(VLogger::ERROR, _callback,
                      "RSItrTest is not initialized, aborting.\n");
    return;
  }

  //////////////////////////////////////////////////////////////////////////
  // Start - Initialization of RunsetWrapper
  if(mRunsetWrapper!=NULL)
    delete mRunsetWrapper;
  mRunsetWrapper = NULL;  

  mServiceMan->setStoppable();
  mRunsetWrapper = new RunSetWrapper(&runset,m_CVAC_DataDir,mServiceMan);  
  //mRunsetWrapper->showList(); //for debugging
  mServiceMan->clearStop();

  if(!mRunsetWrapper->isInitialized())
  {
    localAndClientMsg(VLogger::ERROR, _callback,
      "RSItrTest is not initialized because of RunsetWrapper, aborting.\n");    
    return;
  }
  // End - Initialization of RunsetWrapper
  //////////////////////////////////////////////////////////////////////////

  
  //////////////////////////////////////////////////////////////////////////
  // Start - Initialization of RunsetIterator
  if(mRunsetIterator!=NULL)
    delete mRunsetIterator;
  mRunsetIterator = NULL;  
  
  mServiceMan->setStoppable();
  int tnSkipFrames = 150;
  mRunsetIterator = new RunSetIterator(mRunsetWrapper,mRunsetConstraint,
                                       mServiceMan,_callback,tnSkipFrames);
  mServiceMan->clearStop();
  // End - Initialization of RunsetIterator
  //////////////////////////////////////////////////////////////////////////  

  if(mRunsetIterator->isInitialized())
  {
    mServiceMan->setStoppable();
    while(mRunsetIterator->hasNext())
    {
      if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
      {        
        mServiceMan->stopCompleted();
        break;
      }
      ResultSet tResSet = processSingleFile(this,mRunsetIterator->getNext());
      _callback->foundNewResults(tResSet);
    }
    mServiceMan->clearStop();
  }
  else
  {
    localAndClientMsg(VLogger::ERROR, _callback,
      "RSItrTest is not initialized because of RunsetIterator, aborting.\n");
    return;
  }  
}

ResultSet RSItrICEI::processSingleFile(DetectorPtr detector,LabelablePtr _lPtrOriginal)
{
  ResultSet tResSet;	 

  // Detail the current file being processed (DEBUG_1)
  std::string tPathFile = _lPtrOriginal->sub.path.directory.relativePath
    + "/" + _lPtrOriginal->sub.path.filename;
  localAndClientMsg(VLogger::DEBUG_1, NULL, "%s is processing.\n",
    tPathFile.c_str());

  int tBestClass(7);  //for testing 
  bool tFlagResult(true);

  //BowICEItrI* _bowCV = static_cast<BowICEItrI*>(detector.get());
  //float confidence = 0.5f; // TODO: obtain some confidence from BOW
  //tFlagResult = _bowCV->pBowCV->detect_run(m_CVAC_DataDir + "/" + tPathFile, tBestClass);

  if(true == tFlagResult)
  {
    localAndClientMsg(VLogger::DEBUG_1, NULL, "Detection, %s as Class: %d\n", 
      tPathFile.c_str(),
      tBestClass);

    Result _tResult;

    _tResult.original = _lPtrOriginal;

    LabelablePtr tlPtrRes = new Labelable();
    char buff[32];
    sprintf(buff, "%d", tBestClass);
    tlPtrRes->confidence = 0.5f;
    tlPtrRes->lab.hasLabel = true;
    tlPtrRes->lab.name = buff;
    _tResult.foundLabels.push_back(tlPtrRes);

    tResSet.results.push_back(_tResult);
  }

  return tResSet;
}
