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
/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/*
 * performance.cpp
 *
 * Measure performance of classifier
 */
#include <Ice/Ice.h>
#include <IceBox/IceBox.h>
#include <IceUtil/UUID.h>
#include "DetectorI.h"
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/DetectorDataArchive.h>
#include <util/ServiceMan.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <cstdio>
#include <cmath>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#ifdef _WIN32
/* use clock() function instead of time() */
#define time( arg ) (((double) clock()) / CLOCKS_PER_SEC)
#endif /* _WIN32 */

#ifndef PATH_MAX
#define PATH_MAX 512
#endif /* PATH_MAX */

using namespace Ice;
using namespace cvac;

typedef struct HidCascade
{
    int size;
    int count;
} HidCascade;

typedef struct ObjectPos
{
    float x;
    float y;
    float width;
    int found;    /* for reference */
    int neighbors;
} ObjectPos;


//===========================================================================

static const char *configFile = "config.server";

//===========================================================================

// For Use with IceBox service
class CVAC_OpenCV_Detector : public ::IceBox::Service
{
private:
    Ice::ObjectAdapterPtr _adapter;
    std::string mName;

public:
    CVAC_OpenCV_Detector()
    {
    }
    virtual ~CVAC_OpenCV_Detector()
    {
    }
    virtual void start(const std::string& name,
        const Ice::CommunicatorPtr &communicator,
        const Ice::StringSeq &args)
    {
        localAndClientMsg(VLogger::DEBUG_3, NULL, "CVAC_OpenCV_Detector: starting\n"); // Echo immediately
   
        mName = name;
        _adapter = communicator->createObjectAdapter(name);
        setServiceName(name);
        clearStop(getServiceName());
        DetectorI* detect = new DetectorI("opencv",
                                          "OpenCV HAAR detector",
                                          "xml");
        _adapter->add(detect, communicator->stringToIdentity("CVAC_OpenCV_Detector"));
        _adapter->activate();

        localAndClientMsg(VLogger::INFO, NULL, "Service started: %s \n", mName.c_str());
    }

    virtual void stop()
    {
        _adapter->deactivate();
        localAndClientMsg(VLogger::INFO, NULL, "Stopping Service: %s \n", mName.c_str());
        waitForStopService(getServiceName());
        localAndClientMsg(VLogger::INFO, NULL, "Service stopped: %s \n", mName.c_str());
    }
};
// factory function for IceBox server
extern "C"
{
    ICE_DECLSPEC_EXPORT IceBox::Service* create(Ice::CommunicatorPtr communicator)
    {
        return new CVAC_OpenCV_Detector();
    }
}
//===========================================================================



class OpenCVPerformanceServer : public Ice::Application
{
public:
    virtual int run(int, char*[]);
};

int main( int argc, char* argv[] )
{
    OpenCVPerformanceServer app;
    return app.main(argc, argv, configFile);
}

int OpenCVPerformanceServer::run( int argc, char* [] )
{
    if(argc > 1)
    {
        localAndClientMsg(VLogger::WARN, NULL, "%s: too many arguments\n", appName());
        return EXIT_FAILURE;
    }
    
    Ice::ObjectAdapterPtr adapter =
           communicator()->createObjectAdapter("CVAC_OpenCV_Detector.Server");


  /*Ice::Identity alID = communicator()->stringToIdentity(IceUtil::generateUUID());
    CVAlgorithmPrx alProx = CVAlgorithmPrx::uncheckedCast(adapter->createProxy(alID));
    adapter->add(algor, alID);*/

    DetectorI* detect = new DetectorI("cvPerformance",
                              "OpenCV HAAR detector",
                               "xml");
    adapter->add(detect,
                 communicator()->stringToIdentity("CVAC_OpenCV_Detector"));

    adapter->activate();

    communicator()->waitForShutdown();
    localAndClientMsg(VLogger::INFO, NULL, "CVAC_OpenCV_Detector: stopped\n");  // Local echo on shutdown
    return EXIT_SUCCESS;
}

//===========================================================================

ResultSetV2 detectFile(DetectorPtr detector, const char *fname)
{
   ResultSetV2 resultSet;

   localAndClientMsg(VLogger::DEBUG, NULL, "Filename for Detection: %s\n", fname);

   IplImage* img;
   double scale_factor = 1.2;
   int i;
   DetectorI *detectI = dynamic_cast<DetectorI*>(detector.get());

   img = cvLoadImage( fname );
   if( !img )
   {
      return resultSet;
   }

   //localAndClientMsg(VLogger::DEBUG, NULL, "Loaded image w, h: %d, %d\n", img->width, img->height);

   CvMemStorage *storage = detectI->getStorage();
   CvHaarClassifierCascade *cascade = detectI->getCascade();
   cvClearMemStorage(storage);
   CvSeq *objects;
   //localAndClientMsg(VLogger::DEBUG, NULL, "calling cvHaarDetectObjects, cascade cnt %d, scale fac %g\n", cascade->count, scale_factor);
   objects = cvHaarDetectObjects(img, cascade, storage, scale_factor, 1 );
   //localAndClientMsg(VLogger::DEBUG, NULL, "returned from cvHaarDetectObjects");
   cascade->count = detectI->getNOS();

   int detcount = (objects ? objects->total : 0);
   localAndClientMsg(VLogger::DEBUG, NULL, "detections: %d\n", detcount);

   Result newResult;
   newResult.original = new Labelable();
   newResult.original->sub.isImage = true;
   newResult.original->sub.path.filename = std::string(fname);
   newResult.original->sub.path.directory.relativePath = std::string(getCurrentWorkingDirectory().c_str());


   //res.filename = std::string(fname);

   //ResultRect rect;
   //std::vector<ResultRect> *detections =
      //new std::vector<ResultRect>(detcount);
   for( i = 0; i < detcount; i++ )
   {
      CvAvgComp r = *((CvAvgComp*) cvGetSeqElem( objects, i ));

      BBox* box = new BBox();
      box->x = r.rect.x;
      box->y = r.rect.y;
      box->width = r.rect.width;
      box->height = r.rect.height;

      LabeledLocation* newLocation = new LabeledLocation();
      newLocation->loc = box;

      newResult.foundLabels.push_back(newLocation);
      //detections->push_back(rect);
   }

   resultSet.results.push_back(newResult);
   //localAndClientMsg(VLogger::DEBUG, NULL, "Added to result set\n");

   //res.filename = std::string(fname);
   //res.detections = *detections;

   cvReleaseImage( &img );

   return resultSet;
}

/*
ResultSetV1 detectFile(DetectorPtr detector, const char *fname)
{
    ResultSetV1 res;
    IplImage* img;
    double scale_factor = 1.2;
    int i;
    DetectorI *detectI = dynamic_cast<DetectorI*>(detector.get());

    img = cvLoadImage( fname );
    if( !img )
    {
        return res;
    }
    CvMemStorage *storage = detectI->getStorage();
    CvHaarClassifierCascade *cascade = detectI->getCascade();
    cvClearMemStorage(storage);
    CvSeq *objects;
    objects = cvHaarDetectObjects(img, cascade, storage, scale_factor, 1 );
    cascade->count = detectI->getNOS();

    int detcount = ( objects ? objects->total : 0);

    res.filename = std::string(fname);
    ResultRect rect;
     std::vector<ResultRect> *detections =
                         new std::vector<ResultRect>(detcount);
    for( i = 0; i < detcount; i++ )
    {
        CvAvgComp r = *((CvAvgComp*) cvGetSeqElem( objects, i ));
        rect.x = r.rect.x;
        rect.y = r.rect.y;
        rect.width = r.rect.width;
        rect.height = r.rect.height;
        rect.confidence = 0;
        detections->push_back(rect);
    }

    res.filename = std::string(fname);
    res.detections = *detections;

    cvReleaseImage( &img );

    return res;
}
*/

//===========================================================================
DetectorI::DetectorI(std::string name, std::string desc, std::string cascade)
{

    _cascadeString = cascade;
    _name = name;
    _description = desc;
    _is_initialized = false;
}

const char *tempSavedCascade = "mycascade";  // temp binary file name
#ifdef WIN32
const char *openStr = "w+b";
#else
const char *openStr = "w+";
#endif

void DetectorI::initialize(int verbosity,
                           const ::DetectorData& ddata,
                           const Ice::Current &current)
{
    localAndClientMsg(VLogger::DEBUG, NULL, "OpenCVPerformance initializing...\n");
    bool newCascade = false;
    _verbosity = verbosity;
    _ddata = ddata;
    _storage = cvCreateMemStorage();
    int size = ddata.data.size();
    std::string arg1;
    ::FILE *xml = NULL;
    std::string archiveFilePath;

    Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
    vLogger.setLocalVerbosityLevel(props->getProperty("CVAC.ServicesVerbosity"));

    // Load the CVAC property: 'CVAC.DataDir'  Used for the xml filename path, and to correct Runset paths
    m_CVAC_DataDir = props->getProperty("CVAC.DataDir");
    if(m_CVAC_DataDir.empty()) {
      localAndClientMsg(VLogger::WARN, NULL, "Unable to locate CVAC Data directory, specified: 'CVAC.DataDir = path/to/dataDir' in </CVAC_Services/config.service>\n");
    }
    localAndClientMsg(VLogger::DEBUG, NULL, "CVAC Data directory configured as: %s \n", m_CVAC_DataDir.c_str());
    

    // File name and path object was received
    if (size > 0 || ddata.type == cvac::FILE)
    {
      if(cvac::FILE == ddata.type && size == 0) 
      {
        // Use utils un-compression to get file names
        std::string directory = std::string(getCurrentWorkingDirectory().c_str());  // Unzip to subfolder of CWD, not CVAC_DataDir
        std::string name = getName(current);
        std::string dpath;
        dpath.reserve(directory.length() + name.length() + 3);
        dpath += directory;
        dpath += std::string("/");
        dpath += ".";
        dpath += name;  // Filepath is relative to 'CVAC_DataDir'
        if ((ddata.file.directory.relativePath.length() > 1 && ddata.file.directory.relativePath[1] == ':' )||
            ddata.file.directory.relativePath[0] == '/' ||
            ddata.file.directory.relativePath[0] == '\\')
        {  // absolute path
            archiveFilePath = ddata.file.directory.relativePath + "/" + ddata.file.filename;
        } else { // prepend our prefix
            archiveFilePath = (m_CVAC_DataDir + "/" + ddata.file.directory.relativePath + "/" + ddata.file.filename);
        }
        localAndClientMsg(VLogger::DEBUG, NULL, 
				"CvPerformance will use DetectorData file %s\n", archiveFilePath.c_str());
        std::vector<std::string> fileNameStrings =  expandSeq_fromFile(archiveFilePath, name);

        // Expecting 1 argument
        int numFilesInVector = (int)fileNameStrings.size();
        if(1 != numFilesInVector) {
          localAndClientMsg(VLogger::WARN, NULL, "Expected 1 xml file from zip, (plus one txt file), got: %d.  \nNot inititializing.\n", numFilesInVector);
        }
        arg1 = dpath;
        arg1 += std::string("/");
        arg1 += std::string(fileNameStrings.back());
        
        if(verbosity >=4) {  // 'info' message
          localAndClientMsg(VLogger::DEBUG, NULL, 
                "CvPerformance extracted XML file to %s\n", arg1.c_str());
        }
        _cascadeString = arg1;
      }
      else if(cvac::BYTES == ddata.type)
      {
        xml = fopen(tempSavedCascade, openStr);

        if (NULL == xml)
        {
          localAndClientMsg(VLogger::ERROR_V, NULL, "Error in 'DetectorI::initialize(..', failed to create xml cascade on disk from ddata.\n");
          return;
        }
      }
      else if(cvac::FILE == ddata.type)
      {
        xml = fopen(arg1.c_str(), openStr);

        if (NULL == xml)
        {
          localAndClientMsg(VLogger::ERROR_V, NULL, "Error in 'DetectorI::initialize(..', failed to open xml cascade from zip.\n");
          return;
        }
        else {
          if(verbosity >=3) {
            localAndClientMsg(VLogger::DEBUG, NULL, "CvPerformance Detector loaded XML file.\n");
          }
        }
      }
      if (xml != NULL)
      {
         // Proceed to access XML
         char *buff = new char[size];
         int i;
         for (i = 0; i < size; i++)
           buff[i] = ddata.data[i];
         int res = fwrite(buff, 1, size, xml);
         delete buff;
         if (res != size)
         {
           localAndClientMsg(VLogger::ERROR_V, NULL, "Error in 'DetectorI::initialize(..',  '(res != size)', 'Could not write temporary cascade file'.\n");
           return;
         }
         fflush(xml);
         fclose(xml);
         newCascade = true;
       }
    }
    else {
      localAndClientMsg(VLogger::ERROR_V, NULL, "Error in 'DetectorI::initialize(..', Received 'ddata' stream with size of 0.\n");
      return;
    }

    if (_cascadeString.size() > 0 && newCascade == false)
    {
      _cascade = cvLoadHaarClassifierCascade(
        _cascadeString.c_str(), cvSize( DetectorI::SCAN_WIDTH,
        DetectorI::SCAN_HEIGHT ) );
    }else
    {
      _cascade = cvLoadHaarClassifierCascade(
        tempSavedCascade, cvSize( DetectorI::SCAN_WIDTH,
        DetectorI::SCAN_HEIGHT ) );
    }
    if (NULL != _cascade)
    {
      _nos = _cascade->count;
    }

    localAndClientMsg(VLogger::DEBUG, NULL, "OpenCVPerformance is initialized.\n");
    _is_initialized = true;
}

///////////////////////////////////////////////////////////////////////////////
DetectorData DetectorI::createCopyOfDetectorData(const ::Ice::Current& current)
{
   return _ddata;
}

DetectorI::~DetectorI()
{
}


void DetectorI::destroy(const Ice::Current &current)
{
    _is_initialized = false;
    if (NULL != _storage)
        cvReleaseMemStorage( &_storage );
    if (NULL != _cascade)
        cvReleaseHaarClassifierCascade( &_cascade );
}

void DetectorI::process(const Ice::Identity &client,
                        const RunSet &run,
                        const Ice::Current &curr)
{
//    CallbackReceiverPrx client = CallbackReceiverPrx::uncheckedCast(curr.con->createProxy(ident));
  DetectorCallbackHandlerPrx callback = DetectorCallbackHandlerPrx::uncheckedCast(curr.con->createProxy(client)->ice_oneway());

  callback->message(2, "this is a callback!!");
  localAndClientMsg(VLogger::DEBUG_2, callback, "'OpenCVPerformance::process' called...\n");

  DoDetectFunc func = detectFile;
  processRunSet(this, callback, func, run, m_CVAC_DataDir); // Runs 'func' with modified runset-paths based on data dir
  localAndClientMsg(VLogger::DEBUG_2, callback, "'OpenCVPerformance::process' done.\n");
}

DetectorPropertiesPrx DetectorI::getDetectorProperties(const Ice::Current &)
{
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::getDetectorProperties' called...\n");
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::getDetectorProperties' done.\n");
  return NULL;
}

bool DetectorI::isInitialized(const Ice::Current &)
{
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::isInitialized' called...\n");
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::isInitialized' done.\n");
  return _is_initialized;
}

std::string DetectorI::getName( const Ice::Current &)
{
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::getName' called...\n");
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::getName' done.\n");
  return _name;
}
std::string DetectorI::getDescription(const Ice::Current &)
{
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::getDescription' called...\n");
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::getDescription' done.\n");
  return _description;
}
void DetectorI::setVerbosity(int verbosity, const Ice::Current &)
{
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::setVerbosity' called...\n");
  localAndClientMsg(VLogger::DEBUG_2, NULL, "'OpenCVPerformance::setVerbosity' done.\n");
  _verbosity = verbosity;
}
