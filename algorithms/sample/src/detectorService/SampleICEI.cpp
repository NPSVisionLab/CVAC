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
#include "BowICEI.h"
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
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
        BowICEI *bow = new BowICEI(sMan);
        sMan->setService(bow, "bowTest");
        return (::IceBox::Service*) sMan->getIceService();
	}
}


///////////////////////////////////////////////////////////////////////////////

BowICEI::BowICEI(ServiceManager *sman)
: pBowCV(NULL),fInitialized(false)
{
    mServiceMan = sman;
}

BowICEI::~BowICEI()
{
	delete pBowCV;
	pBowCV = NULL;
}
                          // Client verbosity
void BowICEI::initialize(::Ice::Int verbosity, const ::DetectorData& data, const ::Ice::Current& current)
{
	// Since constructor only called on service start and destroy can be called.  We need to make sure we have it
	if (pBowCV == NULL)
		pBowCV = new bowCV();
	
    // Get the default CVAC data directory as defined in the config file
    m_CVAC_DataDir = mServiceMan->getDataDir();	
	std::string _extFile = data.file.filename.substr(data.file.filename.rfind(".")+1,data.file.filename.length());
	if (_extFile.compare("txt") == 0)
	{
		localAndClientMsg(VLogger::DEBUG, NULL, "Initializing bag_of_words.\n");
		localAndClientMsg(VLogger::DEBUG_1, NULL, "Initializing bag_of_words with %s/%s\n", data.file.directory.relativePath.c_str(), data.file.filename.c_str());
		
		
		if(pBowCV->detect_initialize(data.file.directory.relativePath,data.file.filename))
			fInitialized = true;
		else
			fInitialized = false;
	}
	else	//for a zip file	//if (cvac::FILE == data.type && size == 0)
    { 
         // Use utils un-compression to get zip file names
         // Filepath is relative to 'CVAC_DataDir'
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
         std::string directory = std::string(getCurrentWorkingDirectory().c_str());
         std::string name = getName(current);
         std::string dpath;
         dpath.reserve(directory.length() + name.length() + 3);
         dpath += directory;
         dpath += std::string("/");
         dpath += ".";
         dpath += name;

         if(pBowCV->detect_initialize(dpath,logfile_BowTrainResult))
			 fInitialized = true;
		 else
		 {
			 localAndClientMsg(VLogger::WARN, NULL, "Failed to run CV detect_initialize\n");
			 fInitialized = false;
         }
    }
}



bool BowICEI::isInitialized(const ::Ice::Current& current)
{
	return fInitialized;
}
 
void BowICEI::destroy(const ::Ice::Current& current)
{
	if(pBowCV != NULL)
		delete pBowCV;
	pBowCV = NULL;

	fInitialized = false;
}
std::string BowICEI::getName(const ::Ice::Current& current)
{
	return "bowTest";
}
std::string BowICEI::getDescription(const ::Ice::Current& current)
{
	return "bowTest - Empty Description";
}

void BowICEI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{

}

DetectorData BowICEI::createCopyOfDetectorData(const ::Ice::Current& current)
{	
	DetectorData data;
	return data;
}

DetectorPropertiesPrx BowICEI::getDetectorProperties(const ::Ice::Current& current)
{	
	return NULL;
}

ResultSetV2 BowICEI::processSingleImg(DetectorPtr detector,const char* fullfilename)
{	
	ResultSetV2 _resSet;	
	int _bestClass;	

	// Detail the current file being processed (DEBUG_1)
	std::string _ffullname = std::string(fullfilename);
	localAndClientMsg(VLogger::DEBUG_1, NULL, "%s is processing.\n", _ffullname.c_str());
	BowICEI* _bowCV = static_cast<BowICEI*>(detector.get());
    bool result = _bowCV->pBowCV->detect_run(fullfilename, _bestClass);

    if(true == result) {
        localAndClientMsg(VLogger::DEBUG_1, NULL, "Detection, %s as Class: %d\n", _ffullname.c_str(), _bestClass);
    }

    Result _tResult;
    _tResult.original = new Labelable();
    _tResult.original->sub.path.filename = _ffullname;

    // The original field is for the original file name.  Results need
    // to be returned in foundLabels.
    Labelable *labelable = new Labelable();
    char buff[32];
    sprintf(buff, "%d", _bestClass);
    labelable->lab.name = buff;   
    _tResult.foundLabels.push_back(labelable);
    _resSet.results.push_back(_tResult);
	
	return _resSet;
}

//void BowICEI::process(const ::DetectorCallbackHandlerPrx& callbackHandler,const ::RunSet& runset,const ::Ice::Current& current)
void BowICEI::process(const Ice::Identity &client,const ::RunSet& runset,const ::Ice::Current& current)
{
	DetectorCallbackHandlerPrx _callback = DetectorCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());
  DoDetectFunc func = BowICEI::processSingleImg;

  try {
    processRunSet(this, _callback, func, runset, m_CVAC_DataDir, mServiceMan);
  }
  catch (exception e) {
    localAndClientMsg(VLogger::ERROR_V, NULL, "$$$ Detector could not process given file-path.");
  }
}
