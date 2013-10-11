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
#include <util/ServiceManI.h>

#include <highgui.h>

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
    gotModel = readModelFile( modelfile );
    if (!gotModel)
    {
      localAndClientMsg(VLogger::WARN, NULL, "Failed to read pre-configured trained model "
                        "from: %s; will continue but now require client to send trained model\n",
                        modelfile.c_str());
    }
  }
}  

bool CascadeDetectI::readModelFile( string model )
{
    std::string _extFile = model.substr(model.rfind(".")+1,
                                        model.length());
    if (_extFile.compare("xml") != 0)
    { 
      //for a zip file
      std::string zipfilename = model;
      std::string connectName = "TODO_fixme"; //getClientConnectionName(current);
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
  // Get the default CVAC data directory as defined in the config file
  localAndClientMsg(VLogger::DEBUG, NULL, "Initializing CascadeDetector...\n");
  Ice::PropertiesPtr iceprops = (current.adapter->getCommunicator()->getProperties());
  string verbStr = iceprops->getProperty("CVAC.ServicesVerbosity");
  if (!verbStr.empty())
  {
    vLogger.setLocalVerbosityLevel( verbStr );
  }

  if(model.filename.empty())
  {
    if (!gotModel)
    {
        localAndClientMsg(VLogger::ERROR, NULL, "No trained model available, aborting.\n" );
        return false;
    }
    // ok, go on with pre-configured model
  }
  else
  {
    string modelfile = getFSPath( model, m_CVAC_DataDir );
    localAndClientMsg( VLogger::DEBUG_1, NULL, "initializing with %s\n", modelfile.c_str());
    gotModel = readModelFile( modelfile );
    if (!gotModel)
      {
        localAndClientMsg(VLogger::ERROR, NULL,
                          "Failed to initialize because explicitly specified trained model "
                          "cannot be found or loaded: %s\n", modelfile.c_str());
        return false;
      }
  }

  localAndClientMsg(VLogger::INFO, NULL, "CascadeDetector initialized.\n");
  return true;
}

std::string CascadeDetectI::getName(const ::Ice::Current& current)
{
  mServiceMan->getServiceName();
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
  return DetectorProperties();
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

  initialize( detprops, model, current );
  
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
    ResultSet resSet = convertResults( labelable, objects );
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
  string fullname = getFSPath( lbl.sub.path, m_CVAC_DataDir );
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
  cv::Size orig = cascade->getOriginalWindowSize(); // TODO: make this a parameter, for now use same as cascade
  cascade->detectMultiScale(eq_img, results, scale_factor, min_neighbors, flags, orig); 

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
