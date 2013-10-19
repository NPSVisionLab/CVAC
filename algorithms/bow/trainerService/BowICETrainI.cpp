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
#include <ctime>  //for seeding the function "rand()"

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/ServiceManI.h>
#include <util/DetectorDataArchive.h>
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
        
        BowICETrainI *bow = new BowICETrainI();
        ServiceManagerI *sMan = new ServiceManagerI(bow, bow);
        bow->setServiceManager(sMan);
        return sMan;

	}
}

BowICETrainI::BowICETrainI()
{
    mServiceMan = NULL;	
}

BowICETrainI::~BowICETrainI()
{
}

void BowICETrainI:: setServiceManager(cvac::ServiceManagerI *sman)
{
    mServiceMan = sman;
}

void BowICETrainI::starting()
{
    // Do anything needed on service starting
}

void BowICETrainI::stopping()
{
    // stop the training service
    mServiceMan->stopService();
}

bowCV* BowICETrainI::initialize(int verbosity,const ::Ice::Current& current)
{
	bowCV* pBowCV = new bowCV();
	string	_nameFeature("SIFT");	//SURF, SIFT, FAST, STAR, MSER, GFTT, HARRIS
	string	_nameDescriptor("SIFT");	//SURF, SIFT, OpponentSIFT, OpponentSURF
	string	_nameMatcher("BruteForce-L1");	//BruteForce-L1, BruteForce, FlannBased  
	int		_countWords = 150;	

	// Set CVAC verbosity according to ICE properties
	Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
	string verbStr = props->getProperty("CVAC.ServicesVerbosity");
	if (!verbStr.empty())
	{
	    vLogger.setLocalVerbosityLevel( verbStr );
	}

        bool fInitialized =
          pBowCV->train_initialize(_nameFeature,_nameDescriptor,_nameMatcher,_countWords);
        if (fInitialized)
        {
          return pBowCV;
        }
        else
        {
          return NULL;
        }
}


void BowICETrainI::destroy(const ::Ice::Current& current)
{
}
std::string BowICETrainI::getName(const ::Ice::Current& current)
{
  if (mServiceMan) return mServiceMan->getServiceName();
  return "bowTrain";
}
std::string BowICETrainI::getDescription(const ::Ice::Current& current)
{
	return "BOW - Bag of Words trainer";
}

bool BowICETrainI::cancel(const Ice::Identity &client, const ::Ice::Current& current)
{
    stopping(); 
    mServiceMan->waitForStopService();
    if (mServiceMan->isStopCompleted())
        return true;
    else 
        return false;
 
}
cvac::TrainerProperties BowICETrainI::getTrainerProperties(const ::Ice::Current &current)
{
    //TODO get the real trainer properties but for now return an empty one.
    TrainerProperties tprops;
    return tprops;
}

void BowICETrainI::processSingleImg(string _filepath,string _filename,int _classID,
                                    const ::LocationPtr& _ploc,
                                    bowCV* pBowCV,
                                    TrainerCallbackHandlerPrx& _callback)
{
	std::string _strFilepath = std::string(_filepath);
	std::string _strFilename = std::string(_filename);	
	std::string _strFullname(_strFilepath + "/" + _strFilename);

	std::ostringstream _ostr;	_ostr << _classID;
	std::string _strClassID(_ostr.str());

	if(_ploc.get() == NULL)
        {
          pBowCV->train_stackTrainImage(_strFullname,atoi(_strClassID.c_str()));
        }
	else
	{
          if(_ploc->ice_isA("::cvac::Silhouette"))
          {
            localAndClientMsg(VLogger::DEBUG, _callback, 
                              "Converting Silhouette in %s into BBox.\n",
                              _strFilename.c_str() );
            SilhouettePtr psil = SilhouettePtr::dynamicCast(_ploc);
            int xmax=-1, xmin=INT_MAX, ymax=-1, ymin=INT_MAX;
            for ( vector<Point2DPtr>::iterator ptit = psil->points.begin();
                  ptit!=psil->points.end(); ptit++)
            {
              Point2DPtr pt = *ptit;
              if (pt->x > xmax) xmax = pt->x;
              if (pt->x < xmin) xmin = pt->x;
              if (pt->y > ymax) ymax = pt->y;
              if (pt->y < ymin) ymin = pt->y;
            }
            pBowCV->train_stackTrainImage(_strFullname,atoi(_strClassID.c_str()),
                                          xmin, ymin, xmax-xmin, ymax-ymin);
          }
          else if(_ploc->ice_isA("::cvac::BBox"))
          {
            BBoxPtr pbox = BBoxPtr::dynamicCast(_ploc);         
            pBowCV->train_stackTrainImage(_strFullname,atoi(_strClassID.c_str()),
                                          pbox->x,pbox->y,pbox->width,pbox->height);
          }
          else
          {
            localAndClientMsg(VLogger::WARN, _callback,
                "Not adding %s because %s (not BBox or Silhouette) type.\n",
                _strFilename.c_str(), _strClassID.c_str());
            return;
          }
        }
        localAndClientMsg(VLogger::DEBUG, _callback, 
                          "Adding %s into training class %s.\n",
                          _strFilename.c_str(), _strClassID.c_str());
}



void BowICETrainI::process(const Ice::Identity &client,const ::RunSet& runset,
                           const ::cvac::TrainerProperties &tprops,
                           const ::Ice::Current& current)
{	
  localAndClientMsg(VLogger::DEBUG, NULL, "starting BOW training process\n");
  TrainerCallbackHandlerPrx _callback =
    TrainerCallbackHandlerPrx::uncheckedCast(
      current.con->createProxy(client)->ice_oneway());		
  localAndClientMsg( VLogger::DEBUG_2, _callback, 
                     "starting BOW training process, got callback pointer\n");

  Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
  std::string CVAC_DataDir = props->getProperty("CVAC.DataDir");

  if(runset.purposedLists.size() == 0)
  {
    localAndClientMsg(VLogger::WARN, _callback, 
                      "Error: no data (runset) for processing\n");
    return;
  }
  localAndClientMsg(VLogger::DEBUG, _callback, "got %d purposed lists\n",
                    runset.purposedLists.size());

  bowCV* pBowCV = initialize(tprops.verbosity, current);
  if ( NULL==pBowCV )
  {
    localAndClientMsg(VLogger::ERROR, _callback,
                      "Trainer not initialized, aborting.\n");
    return;
  }

  // ingest the data for processing, one purposed list at a time.
  for (size_t listidx = 0; listidx < runset.purposedLists.size(); listidx++)
  {
    processPurposedList( runset.purposedLists[listidx], pBowCV,
                         _callback, CVAC_DataDir );
  }

  // create a sandbox for this client
  std::string connectName = cvac::getClientConnectionName(current);
  std::string clientName = mServiceMan->getSandbox()->createClientName(
    mServiceMan->getServiceName(), connectName);
  std::string tTempDir = mServiceMan->getSandbox()->createTrainingDir(clientName);
  // TODO: when should this tTempDir be deleted?

  localAndClientMsg(VLogger::INFO, _callback, 
                    "Starting actual training procedure...\n"); 
  // Tell ServiceManager that we will listen for stop
  mServiceMan->setStoppable();

  //
  // run the actual training procedure on the previously ingested data
  //
  bool fTrain = pBowCV->train_run(tTempDir, logfile_BowTrainResult, mServiceMan);
  
  // Tell ServiceManager that we are done listening for stop
  mServiceMan->clearStop();  
  if(!fTrain)
  {
    deleteDirectory(tTempDir);
    localAndClientMsg(VLogger::ERROR, _callback,
                      "Error during the training of BoW.\n");
    return;
  }

  // create the archive of the trained model
  FilePath trainedModel =
    createArchive( pBowCV, clientName, CVAC_DataDir, tTempDir );
  _callback->createdDetector(trainedModel);

  delete pBowCV;
  pBowCV = NULL;

  localAndClientMsg(VLogger::INFO, _callback, "Training procedure completed.\n");
}


void BowICETrainI::processPurposedList( PurposedListPtr purList,
                                        bowCV* pBowCV,
                                        TrainerCallbackHandlerPrx& _callback,
                                        const string& CVAC_DataDir )
{
  int _classID = purList->pur.classID;
  PurposedLabelableSeq* lab = static_cast<PurposedLabelableSeq*>(purList.get());
  assert(NULL!=lab);
  
  if(lab->labeledArtifacts.size() == 0)
  {
    localAndClientMsg(VLogger::WARN, _callback,
                      "no actual labeledArtifacts in purposed list (%d)\n",
                      purList->pur.classID );
    // ignore and continue
  }
  
  for (size_t artfct=0; artfct< lab->labeledArtifacts.size(); artfct++)
  {						
    std::string _filepath;
    std::string _filename;	
    std::string fullName = getFSPath(lab->labeledArtifacts[artfct]->sub.path,
                                     CVAC_DataDir);
    _filename = getFileName(fullName);
    _filepath = getFileDirectory(fullName);
    
    LocationPtr pLoc = NULL;
    if(lab->labeledArtifacts[artfct]->ice_isA("::cvac::LabeledLocation"))
    {
      LabeledLocationPtr plabeledLocation =
        LabeledLocationPtr::dynamicCast(lab->labeledArtifacts[artfct]);
      if (plabeledLocation)
      {
        assert( NULL!=plabeledLocation.get() );
        pLoc = plabeledLocation->loc;				
      }
      else
      {
        printf("TODO: *** RunSetWrapper needs to fix this, it ignores the loc:\n");
        printf("WEIRD: *** why is ice_isA true but the cast returns null?\n");
      }
    }
    processSingleImg(_filepath,_filename, _classID, pLoc, pBowCV, _callback);		
  }
}

/** archive the trained model and return a CVAC style path to the archive
 */
FilePath BowICETrainI::createArchive( bowCV* pBowCV,
                                      const string& clientName,
                                      const string& CVAC_DataDir,
                                      const string& tempDir )
{
  std::string clientDir = mServiceMan->getSandbox()->createClientDir(clientName);
  std::string archiveFilename = getDateFilename(clientDir,  "bow")+ ".zip";
  DetectorDataArchive dda;
 
  dda.setArchiveFilename(archiveFilename);
  dda.addFile(RESID, tempDir + "/" + logfile_BowTrainResult);
  dda.addFile(VOCID, tempDir + "/" + pBowCV->filenameVocabulary);
  dda.addFile(SVMID, tempDir + "/" + pBowCV->filenameSVM);
  dda.createArchive(tempDir);
  mServiceMan->getSandbox()->deleteTrainingDir(clientName);

  // create a CVAC FilePath out of the DDA file system path
  FilePath file;
  file.filename = getFileName(archiveFilename);
  std::string relDir;
  int idx = clientDir.find(CVAC_DataDir.c_str(), 0, CVAC_DataDir.length());
  if (idx == 0)
  {
      relDir = clientDir.substr(CVAC_DataDir.length() + 1);
  }else
  {
      relDir = clientDir;
  }
  file.directory.relativePath = relDir;
  
  return file;
}
