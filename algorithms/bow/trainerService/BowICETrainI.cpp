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
#include <util/ServiceMan.h>
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
        ServiceManager *sMan = new ServiceManager();
        BowICETrainI *bow = new BowICETrainI(sMan);
        sMan->setService(bow, bow->getName());
        return (::IceBox::Service *) sMan->getIceService();

	}
}

BowICETrainI::BowICETrainI(ServiceManager *serv)
: pBowCV(NULL),fInitialized(false)
{
    mServiceMan = serv;	
	pBowCV = new bowCV();
}

BowICETrainI::~BowICETrainI()
{
	delete pBowCV;
	pBowCV = NULL;
}

void BowICETrainI::initialize(::Ice::Int verbosity,const ::Ice::Current& current)
{
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

        fInitialized =
          pBowCV->train_initialize(_nameFeature,_nameDescriptor,_nameMatcher,_countWords);
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
	return "BOW - Bag of Words trainer";
}

void BowICETrainI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{

}
::TrainerPropertiesPrx BowICETrainI::getTrainerProperties(const ::Ice::Current &current)
{
    return NULL;
}

//ResultSetV2
void BowICETrainI::processSingleImg(string _filepath,string _filename,int _classID,
                                    const ::LocationPtr& _ploc,
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
		if(!_ploc->ice_isA("::cvac::BBox"))
		{
                  localAndClientMsg(VLogger::WARN, _callback, 
                                    "Not adding %s because not of cvac::BBox type.\n",
                                    _strFilename.c_str(), _strClassID.c_str());
                  return;
		}		
                BBoxPtr pbox = BBoxPtr::dynamicCast(_ploc);		
                pBowCV->train_stackTrainImage(_strFullname,atoi(_strClassID.c_str()),
                                              pbox->x,pbox->y,pbox->width,pbox->height);
        }
        localAndClientMsg(VLogger::DEBUG, _callback, "Adding %s into training class %s.\n",
                          _strFilename.c_str(), _strClassID.c_str());
}



void BowICETrainI::process(const Ice::Identity &client,const ::RunSet& runset,const ::Ice::Current& current)
{	
  localAndClientMsg(VLogger::DEBUG, NULL, "starting BOW training process\n");
  TrainerCallbackHandlerPrx _callback =
    TrainerCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());		
  localAndClientMsg( VLogger::DEBUG_2, _callback, 
                     "starting BOW training process, got callback pointer\n");

  int _classID;

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

  for (size_t listidx = 0; listidx < runset.purposedLists.size(); listidx++)
  {
    _classID = runset.purposedLists[listidx]->pur.classID;
    PurposedLabelableSeq* lab = static_cast<PurposedLabelableSeq*>(runset.purposedLists[listidx].get());
    assert(NULL!=lab);
    
    if(lab->labeledArtifacts.size() == 0)
    {
      localAndClientMsg(VLogger::WARN, _callback,
                        "no actual labeledArtifacts in purposed list %d\n", listidx );
      // ignore and continue
    }

    for (size_t artfct=0; artfct< lab->labeledArtifacts.size(); artfct++)
    {						
      std::string _filepath;
      std::string _filename;	
      _filename = lab->labeledArtifacts[artfct]->sub.path.filename;
      _filepath = lab->labeledArtifacts[artfct]->sub.path.directory.relativePath;
      _filepath = expandFilename(_filepath, CVAC_DataDir);
      
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
            printf("TODO: ****** RunSetWrapper needs to fix this, it ignores the loc:\n");
            printf("WEIRD: ********* why is ice_isA true but the cast returns null?\n"); 
          }
      }
      processSingleImg(_filepath,_filename, _classID,pLoc, _callback);		
    }
  }

  std::string tCurdir = getCurrentWorkingDirectory();  
#ifdef WIN32
  char *tTempDir_Char = _tempnam(tCurdir.c_str(), NULL);
#else
  char *tTempDir_Char = tempnam(tCurdir.c_str(), NULL);
#endif /* WIN32 */
  std::string tTempDir = tTempDir_Char;  
  makeDirectory(tTempDir);  //For saving bow training results temporary  

  localAndClientMsg(VLogger::INFO, _callback, 
                    "Starting actual training procedure...\n"); 
  // Tell ServiceManager that we will listen for stop
  mServiceMan->setStoppable();
  bool fTrain = pBowCV->train_run(tTempDir, logfile_BowTrainResult, mServiceMan);

  // Tell ServiceManager that we are done listening for stop
  mServiceMan->clearStop();

  srand((unsigned)time(NULL));
  std::ostringstream tConvNum2Str;
  tConvNum2Str << rand();

  std::string tFilenameDetectorData = "bowDetectorData_" + tConvNum2Str.str() + ".zip";
  std::string tDirectoryDetectorData = CVAC_DataDir;
  if(!fTrain)
  {
    deleteDirectory(tTempDir);
    localAndClientMsg(VLogger::ERROR, _callback, "Error during the training of BoW.\n");
    return;
  }
  else
  {
    std::string tPathUsageOrder = tTempDir + "/usageOrder.txt";
    std::ofstream tusageFile;
    tusageFile.open(tPathUsageOrder.c_str(),std::ofstream::out);
    
    if(tusageFile.is_open())
    {
      tusageFile << logfile_BowTrainResult << std::endl;      
      tusageFile.close();

      std::vector<std::string> tListFiles;
      tListFiles.push_back(tTempDir + "/usageOrder.txt");
      tListFiles.push_back(tTempDir + "/" + logfile_BowTrainResult);
      tListFiles.push_back(tTempDir + "/" + pBowCV->filenameVocabulary);
      tListFiles.push_back(tTempDir + "/" + pBowCV->filenameSVM);
      if(!writeZipArchive(tDirectoryDetectorData + "/" + tFilenameDetectorData,tListFiles))
      {
        localAndClientMsg(VLogger::ERROR, NULL,
          "Detector data is not generated correctly.\n");
        return;
      }
    }
    else
    {
      localAndClientMsg(VLogger::ERROR, NULL,
        "Archive file did not contain a file ordering in 'usageOrder.txt'. Returning empty vector<string>.");
      return;
    }
    deleteDirectory(tTempDir);
  }

  DetectorData detectorData;
  // Method 1	
  detectorData.type = ::cvac::FILE;
  detectorData.file.directory.relativePath = tDirectoryDetectorData;
  detectorData.file.filename = tFilenameDetectorData;  

  // Method 2
  // 	std::vector<std::string> strSeq;
  // 	strSeq.push_back(dirpath);
  // 	strSeq.push_back(logfile_BowTrainResult);
  // 	Ice::OutputStreamPtr out = Ice::createOutputStream(current.adapter->getCommunicator());	//communicator()
  // 	out->write(strSeq);
  // 	ByteSeq seq;
  // 	out->finished(seq);
  // 	detectorData.data = seq;	

  _callback->createdDetector(detectorData);

  localAndClientMsg(VLogger::INFO, _callback, "Training procedure completed.\n");
}
