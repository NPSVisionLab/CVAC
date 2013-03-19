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
#include <MultiBoostIceI.h>

#include <opencv/cv.h>
#include <opencv/highgui.h> //for opencv image
#include <MultiBoost.h> // depends on opencv

#include <Ice/Communicator.h>
#include <Ice/Initialize.h> //for createInputStream(?)
#include <Ice/ObjectAdapter.h>
#include <CVACUtil/processRunSet.h>
#include <CVACUtil/FileUtils.h>
#include "CVACUtil/DetectorDataArchive.h"

//temp
#include <iostream>
#include <assert.h>
#include <string>
#include <fstream>

///////////////////////////////////////////////////////////////////////////////
MultiBoostIceI::MultiBoostIceI()
   : mMultiBoostThread(NULL)
   , mInitialized(false)
{
   mMultiBoost = MultiBoost::CreateMultiBoost();
}

///////////////////////////////////////////////////////////////////////////////
MultiBoostIceI::~MultiBoostIceI()
{
   destroy();
}

///////////////////////////////////////////////////////////////////////////////
/* Ordering of arguments in 'MultiBoost Init(..' comes from the vector ordering,
   specified in file 'usageOrder.txt' by the caller adding it to the zip file.

   First argument is Dictionary XML, second argument is Detector Data XML
*/
void MultiBoostIceI::initialize(::Ice::Int verbosity,
                                const ::cvac::DetectorData& data,
                                const ::Ice::Current& current)
{
    // Datatypes:  {BYTES, FILE, PROVIDER}
  if(::cvac::FILE == data.type)
  {
      // Use utils un-compression to get file names
    std::string zipFileName = data.file.filename;
std::cout << "File: " << zipFileName << std::endl;

    std::vector<std::string> fileNameStrings = expandSeq_fromFile(zipFileName);


      // Expecting 2 args for 'Init('
    int numFilesInVector = (int)fileNameStrings.size();
    if(2 != numFilesInVector) {
      std::cerr << "Expected 2 file-name strings from zip, (and the ordering txt-file), instead got: " << numFilesInVector << std::endl;
      std::cerr << "Not inititializing. " << std::endl;
    }

      // See argument-ordering comment for 'initialize('
    std::string arg1 = fileNameStrings.back();
    fileNameStrings.pop_back();  // vector: args are stored back to front
    std::string arg2 = fileNameStrings.back();

    mMultiBoost->Init(arg1, arg2);
    mInitialized = true;
  }

  else if(::cvac::BYTES == data.type)
  {  // Warn about BYTES
    std::cerr << "Stub: 'MultiBoostIceI::initialize' needs implementation for 'data.type==BYTES'" << std::endl;
    std::cerr << "Not inititializing. " << std::endl;
  }

  else {  // No other transfer types supported
    std::cerr << "Error, 'MultiBoostIceI::initialize' supports only 'FILE' as data.type" << std::endl;
  }
}

///////////////////////////////////////////////////////////////////////////////
bool MultiBoostIceI::isInitialized(const ::Ice::Current& current)
{
   return mInitialized;
}

////////////////////////////////////////////////////////////////////////////////
void MultiBoostIceI::OnProcessingComplete()
{
   // do this when thread finishes
   mInitialized = false;
   mMultiBoost->UnInit();
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostIceI::destroy(const ::Ice::Current& current)
{
   destroy();
}

////////////////////////////////////////////////////////////////////////////////
void MultiBoostIceI::destroy()
{
   delete mMultiBoost;
   mMultiBoost = NULL;

   mInitialized = false;
}

///////////////////////////////////////////////////////////////////////////////
std::string MultiBoostIceI::getName(const ::Ice::Current& current)
{
   return "MultiBoost";
}

///////////////////////////////////////////////////////////////////////////////
std::string MultiBoostIceI::getDescription(const ::Ice::Current& current)
{
   return "no description";
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostIceI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{

}

///////////////////////////////////////////////////////////////////////////////
cvac::ResultSetV2 MultiBoostIceI::ProcessOneImage(cvac::DetectorPtr detector,
                                                  const char* filename)
{
   cvac::ResultSetV2 resultSet;

   MultiBoostIceI* multiBoostDetector = static_cast<MultiBoostIceI*>(detector.get());

   IplImage *img = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
   if (img)
   {
      IplImage *out_img = cvCloneImage(img);
      std::vector<MultiBoostMatch> matches;

      //look for matches
      multiBoostDetector->mMultiBoost->Detect(img, out_img, matches);

      //we have some matches
      if (!matches.empty())
      {
         cvac::Result newResult;

         // "original" is intended to represent the ground truth so no need to set it here
         newResult.original = new cvac::Labelable();
         newResult.original->sub.isImage = true;
         newResult.original->sub.path.filename = cvac::getFileName(filename);
         newResult.original->sub.path.directory.relativePath = cvac::getFilePath(filename);

         //copy the results
         multiBoostDetector->FillOutResult(newResult, matches);

         resultSet.results.push_back(newResult);

         //save the marked up image locally (TODO?)
         multiBoostDetector->SaveMarkedUpImage(filename, *out_img);
      }


      cvReleaseImage(&out_img);
   }

   return resultSet;
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostIceI::process(const ::cvac::DetectorCallbackHandlerPrx& callbackHandler,
                             const ::cvac::RunSet& runset,
                             const ::Ice::Current& current)
{
   cvac::DoDetectFunc func = MultiBoostIceI::ProcessOneImage;
   cvac::processRunSet(this, callbackHandler, func, runset);

   mMultiBoost->UnInit();
   mInitialized = false;
}

///////////////////////////////////////////////////////////////////////////////
cvac::DetectorPropertiesPrx MultiBoostIceI::getDetectorProperties(const ::Ice::Current& current)
{
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostIceI::SaveMarkedUpImage(const std::string& filename,
                                       const IplImage& out_img) const
{
   const std::string outputPath = cvac::getFilePath(filename)+"/out";

   //make an "out" subdirectory below the file's current directory
   if (cvac::directoryExists(outputPath) == false)
   {
      cvac::makeDirectory(outputPath);
   }

   //change the path to include the "out" subfolder and tack on "_out" to the
   //image filename (e.g. c:/temp/myimage.jpg -> c:/temp/out/myimage_out.jpg)
   std::string outFilename = outputPath + "/" +
                             cvac::getBaseFileName(filename) +
                             "_out.jpg";

   cvSaveImage(outFilename.c_str(), &out_img);
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostIceI::FillOutResult(cvac::Result& outResult, const std::vector<MultiBoostMatch>& matches) const
{

   std::vector<MultiBoostMatch>::const_iterator itr = matches.begin();
   while (itr != matches.end())
   {
      //cvac::ResultRect rect;
      //rect.x = itr->m_x;
      //rect.y = itr->m_y;
      //rect.width = itr->m_w;
      //rect.height = itr->m_h;
      //rect.confidence = itr->m_score;

      //resultSet.detections.push_back(rect);

      cvac::BBox* box = new cvac::BBox();
      box->x = itr->m_x;
      box->y = itr->m_y;
      box->width = itr->m_w;
      box->height = itr->m_h;

      cvac::LabeledLocation* newLocation = new cvac::LabeledLocation();
      newLocation->loc = box;

      outResult.foundLabels.push_back(newLocation);

      ++itr;
   }
}

///////////////////////////////////////////////////////////////////////////////
cvac::DetectorData MultiBoostIceI::createCopyOfDetectorData(const ::Ice::Current& current)
{
   //TODO store and return the used DetectorData, not this hard coded stuff
   Ice::OutputStreamPtr out = Ice::createOutputStream(current.adapter->getCommunicator());
   out->write("marine_dictionary.xml");
   out->write("marine_detector.xml");
   cvac::ByteSeq seq;
   out->finished(seq);

   cvac::DetectorData data;
   data.data = seq;
   return data;
}

///////////////////////////////////////////////////////////////////////////////
namespace nps {
   int g_verbose           = 4;
   FILE* g_ostream         = stderr;
   const char* leveltxt[]  = {"silent", "error", "warn ",
      "info ", "info1", "info2",
      "info3", "info4", "info5"};
   void printv(int level, const char* fmt, ...)
   {
      if (nps::g_verbose >= level)
      {
         va_list args;
         va_start(args, fmt);
         fprintf(nps::g_ostream, "%s: ", leveltxt[level]);
         vfprintf(nps::g_ostream, fmt, args);
         va_end(args);
         fflush(nps::g_ostream);
      }
   }
}; // namespace
