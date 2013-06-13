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
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/DetectorDataArchive.h>

#include <highgui.h>

#include "CascadeDetectI.h"

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
    CascadeDetectI *detector = new CascadeDetectI(sMan);
    sMan->setService(detector, "CascadeDetect");
    return (::IceBox::Service*) sMan->getIceService();
  }
}


///////////////////////////////////////////////////////////////////////////////

CascadeDetectI::CascadeDetectI(ServiceManager *sman)
  : fInitialized(false)
  , cascade(NULL)
{
  mServiceMan = sman;
}

CascadeDetectI::~CascadeDetectI()
{
  cvReleaseHaarClassifierCascade( &cascade );
  cvReleaseMemStorage( &storage );
}

// TODO create C++ util function; see easy.py for an example
std::string getFSPath( cvac::FilePath fp, const std::string& rootDir="" )
{
  return "TODO";
}

// Client verbosity
void CascadeDetectI::initialize( ::Ice::Int verbosity,
                                 const ::DetectorData& data,
                                 const ::Ice::Current& current)
{
  // Get the default CVAC data directory as defined in the config file
  localAndClientMsg(VLogger::DEBUG, NULL, "Initializing CascadeDetector...\n");
  m_CVAC_DataDir = mServiceMan->getDataDir();	
  std::string _extFile = data.file.filename.substr(data.file.filename.rfind(".")+1,
                                                   data.file.filename.length());
  std::string dpath = "";
  if (_extFile.compare("xml") == 0)
  {
    dpath = getFSPath( data.file, m_CVAC_DataDir );
  }
  else	//for a zip file
  { 
    // Use utils un-compression to get zip file names
    // Filepath is relative to 'CVAC_DataDir'
    std::string archiveFilePath; 
    archiveFilePath = (m_CVAC_DataDir + "/" + data.file.directory.relativePath + "/" + data.file.filename);
  
    std::vector<std::string> fileNameStrings =  expandSeq_fromFile(archiveFilePath, getName(current));
    
    // Need to strip off extra zeros
    std::string directory = std::string(getCurrentWorkingDirectory().c_str());
    std::string name = getName(current);
    dpath.reserve(directory.length() + name.length() + 3);
    dpath += directory;
    dpath += std::string("/");
    dpath += ".";
    dpath += name;
  }
  localAndClientMsg( VLogger::DEBUG_1, NULL, "initializing with %s\n", dpath.c_str());

  // set parameters
  CvSize detsize = cascade->orig_window_size;
  // TODO: get size from the detector parameters
  
  // load cascade from XML file
  cascade = cvLoadHaarClassifierCascade( dpath.c_str(), detsize );
  if( cascade == NULL )
  {
    localAndClientMsg( VLogger::WARN, NULL,
                       "unable to load classifier from %s\n", dpath.c_str());
    fInitialized = false;
    return;
  }

  storage = cvCreateMemStorage();
  cascade_name = data.file.filename;

  localAndClientMsg(VLogger::INFO, NULL, "CascadeDetector initialized.\n");
  fInitialized = true;
}



bool CascadeDetectI::isInitialized(const ::Ice::Current& current)
{
  return fInitialized;
}
 
void CascadeDetectI::destroy(const ::Ice::Current& current)
{
  cvReleaseHaarClassifierCascade( &cascade );
  cvReleaseMemStorage( &storage );
  fInitialized = false;
}
std::string CascadeDetectI::getName(const ::Ice::Current& current)
{
  return "CascadeDetector";
}
std::string CascadeDetectI::getDescription(const ::Ice::Current& current)
{
  return "OpenCV Cascade Detector (boost)";
}

void CascadeDetectI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{
}

DetectorData CascadeDetectI::createCopyOfDetectorData(const ::Ice::Current& current)
{	
  DetectorData data;
  return data;
}

DetectorPropertiesPrx CascadeDetectI::getDetectorProperties(const ::Ice::Current& current)
{	
  return NULL;
}

void CascadeDetectI::process( const Ice::Identity &client,
                              const ::RunSet& runset,
                              const ::Ice::Current& current)
{
  DetectorCallbackHandlerPrx _callback = DetectorCallbackHandlerPrx::uncheckedCast(
            current.con->createProxy(client)->ice_oneway());

  // loop over Labelable in runset
  // TODO: for each labelable...
  Labelable* ll = new Labelable( 0.2f, Label(), Substrate() );
  const cvac::Labelable& labelable = *ll; // TODO: should come from iterator
  {
    CvSeq* objects;
    objects = detectObjects( labelable );
    reportResults( labelable, objects );
  }
}

// TODO: make this a utility function
std::string getSubstrateFileName( const cvac::Labelable& lbl )
{
  return getFSPath( lbl.sub.path );
}

CvSeq* CascadeDetectI::detectObjects( const cvac::Labelable& lbl )
{
  CvSeq* objects = NULL;
  string fullname = getSubstrateFileName( lbl );
  IplImage *img = cvLoadImage( fullname.c_str() );
  if( !img )
  {
    localAndClientMsg(VLogger::WARN, NULL, "cannot open %s\n", fullname.c_str());
    return NULL;
  }
  localAndClientMsg(VLogger::DEBUG_1, NULL, "About to process %s\n", fullname.c_str());
  cvClearMemStorage( storage );
  float scale_factor = 1.2; // TODO: make this part of detector parameters
  objects = cvHaarDetectObjects( img, cascade, storage, scale_factor, 1 );

  return objects;
}

// TODO: create utility function in processRunSet; functionality should already
// be in processRunSet.cpp
void sendResultsToClient( const ResultSetV2& results )
{
  // TODO
}

void CascadeDetectI::reportResults( const Labelable& original, CvSeq* foundObjects )
{	
  ResultSetV2 resSet;	
  int bestClass;	
  
  Result tResult;
  // The original field is for the original label and file name.  Results need
  // to be returned in foundLabels.
  tResult.original = NULL; // TODO: figure out how to assign original;
  Labelable *labelable = new Labelable();
  labelable->lab.name = cascade_name;
  tResult.foundLabels.push_back( labelable );
  resSet.results.push_back( tResult );

  sendResultsToClient( resSet );
}

