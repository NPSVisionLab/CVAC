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
#include <string>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/DetectorDataArchive.h>
#include <util/ServiceManI.h>
#include <util/OutputResults.h>

#include <highgui.h>
#include <stdlib.h>

#include "CascadeDetectI.h"

using namespace cvac;
using namespace Ice;


///////////////////////////////////////////////////////////////////////////////
// This is called by IceBox to get the service to communicate with.
extern "C"
{
  /**
   * Create the detector service via a ServiceManager.  The 
   * ServiceManager handles all the icebox interactions.  Pass the constructed
   * detector instance to the ServiceManager.  The ServiceManager obtains the
   * service name from the config.icebox file as follows. Given this
   * entry:
   * IceBox.Service.BOW_Detector=bowICEServer:create --Ice.Config=config.service
   * ... the name of the service is BOW_Detector.
   */
  ICE_DECLSPEC_EXPORT IceBox::Service* create(CommunicatorPtr communicator)
  {
    CascadeDetectI *detector = new CascadeDetectI();
    ServiceManagerI *sMan = new ServiceManagerI( detector, detector );
    detector->setServiceManager( sMan );
    return sMan;
  }
}

// callback function for processRunSet
// TODO: remove once RunSetWrapper works, see NEW_RUNSETWRAPPER
ResultSet detectFunc(DetectorPtr detector, const char *fname);


///////////////////////////////////////////////////////////////////////////////

CascadeDetectI::CascadeDetectI()
  : callback(NULL)
  , cascade(NULL)
  , mServiceMan(NULL)
  , gotModel(false)
{
}

CascadeDetectI::~CascadeDetectI()
{
  delete cascade;
}

void CascadeDetectI::setServiceManager(ServiceManagerI *sman)
{
  mServiceMan = sman;
}

void CascadeDetectI::starting()
{
  m_CVAC_DataDir = mServiceMan->getDataDir();	

  // check if the config.service file contains a trained model; if so, read it.
  string modelfile = mServiceMan->getModelFileFromConfig();

  if (modelfile.empty())
  {
    localAndClientMsg(VLogger::DEBUG, NULL, "No trained model file specified in service config.\n" );
  }
  else
  {
    localAndClientMsg(VLogger::DEBUG, NULL, "Will read trained model file as specified in service config: %s\n",
                      modelfile.c_str());
    if (pathAbsolute(modelfile) == false)
        modelfile =  m_CVAC_DataDir + "/" + modelfile;
        
    gotModel = readModelFile( modelfile, Ice::Current() );
    if (!gotModel)
    {
      localAndClientMsg(VLogger::WARN, NULL, "Failed to read pre-configured trained model "
                        "from: %s; will continue but now require client to send trained model\n",
                        modelfile.c_str());
    }
  }
}  

bool CascadeDetectI::readModelFile( string model, const ::Ice::Current& current)
{
    std::string _extFile = model.substr(model.rfind(".")+1,
                                        model.length());
    if (_extFile.compare("xml") != 0)
    { 
      //for a zip file
      std::string zipfilename = model;
      std::string connectName = getClientConnectionName(current);
      std::string clientName = mServiceMan->getSandbox()->createClientName(mServiceMan->getServiceName(),
                                                                           connectName);
      
      std::string clientDir = mServiceMan->getSandbox()->createClientDir(clientName);
      DetectorDataArchive dda;
      dda.unarchive(zipfilename, clientDir);
      // This detector only needs an XML file
      model = dda.getFile(XMLID);
      if (model.empty())
        {
          localAndClientMsg( VLogger::WARN, NULL,
                             "unable to load classifier from archive file %s\n", zipfilename.c_str());
          return false;
        }
    }
  // load cascade from XML file
  cascade = new cv::CascadeClassifier;
  if (cascade->load(model.c_str()) == false)
  {
    localAndClientMsg( VLogger::WARN, NULL,
                       "unable to load classifier from %s\n", model.c_str());
    return false;
  }
  cascade_name = model;
  return true;
}

// Client verbosity
bool CascadeDetectI::initialize( const DetectorProperties& detprops,
                                 const FilePath& model,
                                 const Current& current)
{
  // Create DetectorPropertiesI class to allow the user to modify detection
  // parameters
  mDetectorProps = new DetectorPropertiesI();

  // Get the default CVAC data directory as defined in the config file
  localAndClientMsg(VLogger::DEBUG, NULL, "Initializing CascadeDetector...\n");
  Ice::PropertiesPtr iceprops = (current.adapter->getCommunicator()->getProperties());
  string verbStr = iceprops->getProperty("CVAC.ServicesVerbosity");
  if (!verbStr.empty())
  {
    getVLogger().setLocalVerbosityLevel( verbStr );
  }

  if(model.filename.empty())
  {
    if (!gotModel)
    {
        localAndClientMsg(VLogger::ERROR, callback, "No trained model available, aborting.\n" );
        return false;
    }
    // ok, go on with pre-configured model
  }
  else
  {
    if (gotModel)
    {
        localAndClientMsg(VLogger::WARN , callback, "Detector Preconfigured with a model file so ignoring passed in model %s.\n",
                          model.filename.c_str() );
    }else
    {
        string modelfile = getFSPath( model, m_CVAC_DataDir );
        localAndClientMsg( VLogger::DEBUG_1, NULL, "initializing with %s\n", modelfile.c_str());
        bool res = readModelFile( modelfile, current );
        if (!res)
        {
          localAndClientMsg(VLogger::ERROR, callback,
                        "Failed to initialize because explicitly specified trained model "
                        "cannot be found or loaded: %s\n", modelfile.c_str());
          return false;
        }
    }
  }

  localAndClientMsg(VLogger::INFO, NULL, "CascadeDetector initialized.\n");
  // Set the default window size to what the cascade was trained as.
  // User can override this by passing in properties in the process call.
  cv::Size orig = cascade->getOriginalWindowSize(); // Default to size trained with
  mDetectorProps->nativeWindowSize.width = orig.width;
  mDetectorProps->nativeWindowSize.height = orig.height;
  return true;
}

std::string CascadeDetectI::getName(const ::Ice::Current& current)
{
  return mServiceMan->getServiceName();
}

std::string CascadeDetectI::getDescription(const ::Ice::Current& current)
{
  return "OpenCV Cascade Detector (boost)";
}

void CascadeDetectI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{
}

DetectorProperties CascadeDetectI::getDetectorProperties(const ::Ice::Current& current)
{
  return DetectorPropertiesI();
}

/** Scans the detection cascade across each image in the RunSet
 * and returns the results to the client
 */
void CascadeDetectI::process( const Identity &client,
                              const RunSet& runset,
                              const FilePath& model,
                              const DetectorProperties& detprops,
                              const Current& current)
{
  callback = DetectorCallbackHandlerPrx::uncheckedCast(
            current.con->createProxy(client)->ice_oneway());

  bool initRes = initialize( detprops, model, current );
  if (initRes == false)
      return;
  mDetectorProps->load(detprops);
  //////////////////////////////////////////////////////////////////////////
  // Setup - RunsetConstraints
  cvac::RunSetConstraint mRunsetConstraint;  
  mRunsetConstraint.addType("png");
  mRunsetConstraint.addType("tif");
  mRunsetConstraint.addType("jpg");
  mRunsetConstraint.addType("jpeg");
  mRunsetConstraint.addType("PNG");
  mRunsetConstraint.addType("TIF");
  mRunsetConstraint.addType("JPG");
  mRunsetConstraint.addType("JPEG");
  // End - RunsetConstraints

  //////////////////////////////////////////////////////////////////////////
  // Start - RunsetWrapper
  mServiceMan->setStoppable();  
  cvac::RunSetWrapper mRunsetWrapper(&runset,m_CVAC_DataDir,mServiceMan);
  mServiceMan->clearStop();
  if(!mRunsetWrapper.isInitialized())
  {
    localAndClientMsg(VLogger::ERROR, callback,
      "RunsetWrapper is not initialized, aborting.\n");    
    return;
  }
  // End - RunsetWrapper

  OutputResults outputres(callback, mDetectorProps->callbackFreq);

  //////////////////////////////////////////////////////////////////////////
  // Start - RunsetIterator
  int nSkipFrames = 150;  //the number of skip frames
  mServiceMan->setStoppable();
  cvac::RunSetIterator mRunsetIterator(&mRunsetWrapper,mRunsetConstraint,
                                       mServiceMan,callback,nSkipFrames);
  mServiceMan->clearStop();
  if(!mRunsetIterator.isInitialized())
  {
    localAndClientMsg(VLogger::ERROR, callback,
      "RunSetIterator is not initialized, aborting.\n");
    return;
  } 
  // End - RunsetIterator
 
  mServiceMan->setStoppable();
  while(mRunsetIterator.hasNext())
  {
    if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
    {        
      mServiceMan->stopCompleted();
      break;
    }
    
    cvac::Labelable& labelable = *(mRunsetIterator.getNext());
    Result &curres = mRunsetIterator.getCurrentResult();
   
    std::vector<cv::Rect> objects = detectObjects( callback, labelable );
    // return the label name with result
    string resultName;
    if (objects.size() > 0)
        resultName = "positive";
    else
        resultName = "negative";
    // returning the cascade file name as label name: outputres.addResult(curres,labelable,objects, cascadeName, 1.0f); 
    outputres.addResult(curres,labelable,objects, resultName, 1.0f);      
    
  }  
  // We are done so send any final results
  outputres.finishedResults(mRunsetIterator);
  mServiceMan->clearStop();

  //////////////////////////////////////////////////////////////////////////
  // Example to show results
  cvac::ResultSet& tResSet = mRunsetIterator.getResultSet();
  unsigned int kres;
  for(kres=0;kres<tResSet.results.size();kres++)
  {
    localAndClientMsg( VLogger::DEBUG, NULL, "Original= %s, Found= %i labels\n", 
      tResSet.results[kres].original->sub.path.filename.c_str(),
      tResSet.results[kres].foundLabels.size());
    unsigned int kfnd;
    for(kfnd=0;kfnd<tResSet.results[kres].foundLabels.size();kfnd++)
    {
      //LabeledTrackPtr _tPtr = static_cast<LabeledTrack*>(tResSet.results[kres].foundLabels[kfnd].get());     
      LabeledTrackPtr _tPtr = dynamic_cast<LabeledTrack*>(tResSet.results[kres].foundLabels[kfnd].get());
      //if(_tPtr->lab.hasLabel)
      if (_tPtr && _tPtr->lab.hasLabel)
      {
        if(_tPtr->keyframesLocations[0].frame.framecnt != -1)
          localAndClientMsg( VLogger::DEBUG, NULL, "at Frame=%i\n",_tPtr->keyframesLocations[0].frame.framecnt);
      }     
    }    
  }  
  //////////////////////////////////////////////////////////////////////////
}

/** run the cascade on the image described in lbl,
 *  return the objects (rectangles) that were found
 */
std::vector<cv::Rect> CascadeDetectI::detectObjects( const CallbackHandlerPrx& callback, const cvac::Labelable& lbl )
{
  localAndClientMsg(VLogger::DEBUG_2, callback, "in detectObjects\n");
  CvSeq* objects = NULL;
  string fullname = getFSPath( lbl.sub.path, m_CVAC_DataDir );
  return detectObjects( callback, fullname );
}

/** run the cascade on the image described in lbl,
 *  return the objects (rectangles) that were found
 */
std::vector<cv::Rect> CascadeDetectI::detectObjects( const CallbackHandlerPrx& callback, const std::string& fullname )
{
  // TODO: since the debug messages below print out the full file path,
  // they should be local messages only, not send back to the callback.
  std::vector<cv::Rect> results;
  cv::Mat src_img, gray_img, eq_img;
  localAndClientMsg(VLogger::DEBUG_3, callback, "About to process: %s\n", fullname.c_str());
  src_img = cv::imread( fullname.c_str(), CV_LOAD_IMAGE_COLOR );
  if( src_img.data == NULL )
  {
    localAndClientMsg(VLogger::WARN, callback, "cannot open %s\n", fullname.c_str());
    return results;
  }
  localAndClientMsg(VLogger::DEBUG_2, callback,
                    "Loaded file, will process: %s\n", fullname.c_str());
  cv::cvtColor(src_img, gray_img, CV_RGB2GRAY);
  cv::equalizeHist(gray_img, eq_img);
  int flags = 0;       // TODO: make this a parameter
  cv::Size minwinSize;
  cv::Size maxwinSize;
  maxwinSize.width = 0;
  maxwinSize.height = 0;
  if (mDetectorProps->slideStartSize.width == 0 || mDetectorProps->slideStartSize.width == 0 || 
      mDetectorProps->slideStopSize.width == 0 || mDetectorProps->slideStopSize.width == 0)
  {  // no min max windows specified so use nativeWindowSize
      minwinSize.width = mDetectorProps->nativeWindowSize.width;
      minwinSize.height = mDetectorProps->nativeWindowSize.height;
  }else
  { // min max specified
      maxwinSize.width = mDetectorProps->slideStopSize.width;
      maxwinSize.height = mDetectorProps->slideStopSize.height;
      minwinSize.width = mDetectorProps->slideStartSize.width;
      minwinSize.height = mDetectorProps->slideStartSize.height;
  }
  
  cascade->detectMultiScale(eq_img, results, mDetectorProps->slideScaleFactor, 
                  mDetectorProps->minNeighbors, flags, minwinSize, maxwinSize);
  
  if (results.size() > mDetectorProps->maxRectangles)
  { // Only return maxRectangles
      unsigned long len = results.size();
      results.erase(results.begin()+mDetectorProps->maxRectangles, results.end());
      localAndClientMsg(VLogger::WARN, callback, "reducing result rectangles from %d to %d\n", len, mDetectorProps->maxRectangles);
  }
  return results;
}


/** convert from OpenCV result to CVAC ResultSet
 */

ResultSet CascadeDetectI::convertResults( const Labelable& original, std::vector<cv::Rect> rects)
{	
  int detcount = rects.size();
  localAndClientMsg(VLogger::DEBUG, NULL, "detections: %d\n", detcount);
  
  Result tResult;
  // The original field is for the original label and file name.  Results need
  // to be returned in foundLabels.
  tResult.original = new Labelable( original );
  int cnt = 0; // Only return a max number of rectangles
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
    newLocation->confidence = 1.0f;
    newLocation->loc = box;
    
    tResult.foundLabels.push_back( newLocation );
  }
  
  ResultSet resSet;	
  resSet.results.push_back( tResult );
  
  return resSet;
}


// callback for processRunSet
ResultSet detectFunc(DetectorPtr detector, const char *fname)
{
  localAndClientMsg(VLogger::DEBUG, NULL, "Filename for Detection: %s\n", fname);
  CascadeDetectI *detI = dynamic_cast<CascadeDetectI*>(detector.get());
  if (!detI)
  {
    return ResultSet();
  }

  // do the actual detection
  std::vector<cv::Rect> results;
  results = detI->detectObjects( (DetectorCallbackHandlerPrx)NULL, fname );

  // convert to ResultSet and return
  LabelablePtr original = new Labelable();
  ResultSet resSet = detI->convertResults( *original, results );
  return resSet;
}

bool CascadeDetectI::cancel(const Identity &client, const Current& current)
{
  localAndClientMsg(VLogger::WARN, NULL, "cancel not implemented.");
  return false;
}

//----------------------------------------------------------------------------
DetectorPropertiesI::DetectorPropertiesI()
{
    verbosity = 0;
    isSlidingWindow = true;
    canSetSensitivity = false;
    canPostProcessNeighbors = true;
    videoFPS = 0;
    nativeWindowSize.width = 0;
    nativeWindowSize.height = 0;
    falseAlarmRate = 0.0;
    recall = 0.0;
    minNeighbors = 3;
    slideScaleFactor = 1.2f;
    slideStartSize.width = 0;
    slideStartSize.height = 0;
    slideStopSize.width = 0;
    slideStopSize.height = 0;
    slideStepX = 0.0f;
    slideStepY = 0.0f;
    maxRectangles = 5000;
}

void DetectorPropertiesI::load(const DetectorProperties &p) 
{
    verbosity = p.verbosity;
    props = p.props;
    if (p.videoFPS > 0)
        videoFPS = p.videoFPS;
    //Only load values that are not zero
    if (p.nativeWindowSize.width > 0 && p.nativeWindowSize.height > 0)
        nativeWindowSize = p.nativeWindowSize;
    if (p.minNeighbors >= 0)
        minNeighbors = p.minNeighbors;
    if (p.slideScaleFactor > 0)
        slideScaleFactor = p.slideScaleFactor;
    if (p.slideStartSize.width > 0 && p.slideStartSize.height > 0)
        slideStartSize = p.slideStartSize;
    if (p.slideStopSize.width > 0 && p.slideStopSize.height > 0)
        slideStopSize = p.slideStopSize;
    readProps();
}

bool DetectorPropertiesI::readProps()
{
    bool res = true;
    cvac::Properties::iterator it;
    for (it = props.begin(); it != props.end(); it++)
    {
        if (it->first.compare("callbackFrequency") == 0)
        {
            callbackFreq = it->second;
            if ((it->second.compare("labelable") != 0) &&
                (it->second.compare("immediate") != 0) &&
                (it->second.compare("final") == 0))
            {
                localAndClientMsg(VLogger::ERROR, NULL, 
                         "callbackFrequency type not supported.\n");
                res = false;
            }
        }
        if (it->first.compare("maxRectangles") == 0)
        {
            long cnt = strtol(it->second.c_str(), NULL, 10);
            if (cnt > 0 && cnt != LONG_MAX && cnt != LONG_MIN)
                maxRectangles = cnt;
            else
            {
                localAndClientMsg(VLogger::ERROR, NULL, 
                         "Invalid maxRectangles property.\n");
                res = false;
            }
        }
    }   
    return res;
}
 
bool DetectorPropertiesI::writeProps()
{
    std::stringstream stream;
    stream << maxRectangles;
    bool res = true;
    props.insert(std::pair<string, string>("callbackFrequency", callbackFreq));
    props.insert(std::pair<string, string>("maxRectangles", stream.str()));
    return res;
}

