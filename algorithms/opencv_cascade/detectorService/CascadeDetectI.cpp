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
    sMan->setService(detector, "OpenCVCascadeDetector");
    return (::IceBox::Service*) sMan->getIceService();
  }
}

// callback function for processRunSet
// TODO: remove once RunSetWrapper works, see NEW_RUNSETWRAPPER
ResultSetV2 detectFunc(DetectorPtr detector, const char *fname);


///////////////////////////////////////////////////////////////////////////////

CascadeDetectI::CascadeDetectI(ServiceManager *sman)
  : callback(NULL)
  , fInitialized(false)
  , cascade(NULL)
{
  mServiceMan = sman;
}

CascadeDetectI::~CascadeDetectI()
{
  delete cascade;
}


// TODO: make this a utility function
std::string getSubstrateFileName( const cvac::Labelable& lbl, const std::string& CVAC_DataDir="" )
{
  return getFSPath( lbl.sub.path, CVAC_DataDir );
}

// TODO: move into fileUtils
/** Turn a file system path into a CVAC FilePath
 */
cvac::FilePath* getCvacPath( const std::string& fsPath, const std::string& CVAC_DataDir="" )
{
  // todo: should figure out what CVAC.DataDir is and parse that out, too
  const std::string relPath = getFileDirectory( fsPath );
  const std::string filename = getFileName( fsPath );
  cvac::DirectoryPath directory = DirectoryPath();
  directory.relativePath = relPath;
  FilePath* fp = new FilePath();
  fp->directory = directory;
  fp->filename = filename;
  return fp;
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
    std::string archiveFilePath = getFSPath( data.file );  
    std::vector<std::string> fileNameStrings = 
      expandSeq_fromFile( archiveFilePath, getName( current ));
    
    // Need to strip off extra zeros  (matz: todo: not sure what that means)
    // TODO: don't use cwd here but a temp dir under CVAC.DataDir instead
    std::string directory = std::string(getCurrentWorkingDirectory().c_str());
    std::string name = getName(current);
    dpath.reserve(directory.length() + name.length() + 3);
    dpath = directory + "/." + name;
  }
  localAndClientMsg( VLogger::DEBUG_1, NULL, "initializing with %s\n", dpath.c_str());

  // set parameters
  CvSize detsize = cvSize( 24, 24 );
  // TODO: get size from the detector parameters
  
  // load cascade from XML file
  cascade = new cv::CascadeClassifier;
  if (cascade->load(dpath.c_str()) == false)
  {
    localAndClientMsg( VLogger::WARN, NULL,
                       "unable to load classifier from %s\n", dpath.c_str());
    fInitialized = false;
    return;
  }
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
  delete cascade;
  fInitialized = false;
}
std::string CascadeDetectI::getName(const ::Ice::Current& current)
{
  return "OpenCV CascadeDetector";
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

/** Scans the detection cascade across each image in the RunSet
 * and returns the results to the client
 */
void CascadeDetectI::process( const Ice::Identity &client,
                              const ::RunSet& runset,
                              const ::Ice::Current& current)
{
  callback = DetectorCallbackHandlerPrx::uncheckedCast(
            current.con->createProxy(client)->ice_oneway());

  // While we don't have the new RunSetWrapper and iterator, use the
  // processRunSet callback method
#if NEW_RUNSETWRAPPER
  // loop over Labelable in runset
  // TODO: for each labelable...
  Labelable* ll = new Labelable( 0.2f, Label(), Substrate() );
  const cvac::Labelable& labelable = *ll; // TODO: should come from iterator
  {
    // do the actual detection
    CvSeq* objects;
    objects = detectObjects( callback, labelable );

    // convert to ResultSet and send to ICE client
    ResultSetV2 resSet = convertResults( labelable, objects );
    sendResultsToClient( callback, resSet );
  }
#else
  processRunSet(this, callback, detectFunc, runset, m_CVAC_DataDir, mServiceMan );
#endif

  callback = NULL;
}

/** run the cascade on the image described in lbl,
 *  return the objects (rectangles) that were found
 */
std::vector<cv::Rect> CascadeDetectI::detectObjects( const CallbackHandlerPrx& callback, const cvac::Labelable& lbl )
{
  localAndClientMsg(VLogger::DEBUG, callback, "in detectObjects\n");
  CvSeq* objects = NULL;
  string fullname = getSubstrateFileName( lbl, m_CVAC_DataDir );
  return detectObjects( callback, fullname );
}

/** run the cascade on the image described in lbl,
 *  return the objects (rectangles) that were found
 */
std::vector<cv::Rect> CascadeDetectI::detectObjects( const CallbackHandlerPrx& callback, const std::string& fullname )
{
  std::vector<cv::Rect> results;
  cv::Mat src_img, gray_img, eq_img;
  localAndClientMsg(VLogger::DEBUG, callback, "About to process 1 %s\n", fullname.c_str());
  src_img = cv::imread( fullname.c_str(), CV_LOAD_IMAGE_COLOR );
  if( src_img.data == NULL )
  {
    localAndClientMsg(VLogger::WARN, callback, "cannot open %s\n", fullname.c_str());
    return results;
  }
  localAndClientMsg(VLogger::DEBUG, callback, "About to process 2 %s\n", fullname.c_str());
  cv::cvtColor(src_img, gray_img, CV_RGB2GRAY);
  cv::equalizeHist(gray_img, eq_img);
  float scale_factor = 1.2f; // TODO: make this part of detector parameters
  int min_neighbors = 3; // TODO: make this a parameter
  int flags = 0;       // TODO: make this a parameter
  cv::Size size(24, 24);  // TODO: make this a parameter
  
  cascade->detectMultiScale(eq_img, results, scale_factor, min_neighbors, flags, size); 

  return results;
}

/** convert from OpenCV result to CVAC ResultSet
 */
ResultSetV2 CascadeDetectI::convertResults( const Labelable& original, std::vector<cv::Rect> rects)
{	
  int detcount = rects.size();
  localAndClientMsg(VLogger::DEBUG, NULL, "detections: %d\n", detcount);
  
  Result tResult;
  // The original field is for the original label and file name.  Results need
  // to be returned in foundLabels.
  tResult.original = new Labelable( original );

  for (std::vector<cv::Rect>::iterator it = rects.begin(); it != rects.end(); ++it)
  {
    cv::Rect r = *it;
    
    BBox* box = new BBox();
    box->x = r.x;
    box->y = r.y;
    box->width = r.width;
    box->height = r.height;
    
    LabeledLocation* newLocation = new LabeledLocation();
    newLocation->lab.hasLabel = true;
    newLocation->lab.name = cascade_name;
    newLocation->loc = box;
    
    tResult.foundLabels.push_back( newLocation );
  }
  
  ResultSetV2 resSet;	
  resSet.results.push_back( tResult );
  
  return resSet;
}

// callback for processRunSet
ResultSetV2 detectFunc(DetectorPtr detector, const char *fname)
{
  localAndClientMsg(VLogger::DEBUG, NULL, "Filename for Detection: %s\n", fname);
  CascadeDetectI *detI = dynamic_cast<CascadeDetectI*>(detector.get());
  if (!detI)
  {
    return ResultSetV2();
  }

  // do the actual detection
  std::vector<cv::Rect> results;
  results = detI->detectObjects( (DetectorCallbackHandlerPrx)NULL, fname );

  // convert to ResultSet and return
  LabelablePtr original = new Labelable();
  ResultSetV2 resSet = detI->convertResults( *original, results );
  return resSet;
}
