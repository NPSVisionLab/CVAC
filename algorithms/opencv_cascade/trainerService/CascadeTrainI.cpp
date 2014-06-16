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
#include <stdio.h>
#include <stdlib.h>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/ServiceMan.h>
#include <util/processLabels.h>
#include <util/DetectorDataArchive.h>

#include "opencv2/core/core.hpp"
#include "opencv2/core/internal.hpp"
#include "cv.h"
#include "cascadeclassifier.h"
#include "cvhaartraining.h"

#include "CascadeTrainI.h"

using namespace std;
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
    CascadeTrainI *cascade = new CascadeTrainI();
    ServiceManagerI *sMan = new ServiceManagerI( cascade, cascade );
    cascade->setServiceManager( sMan );
    return sMan;
  }
      
}
extern void icvMergeVecVec( const char* invecname, const char* outvecname, int posCnt, int showsamples, int width, int height );

CascadeTrainI::CascadeTrainI()
  : fInitialized(false)
  , mServiceMan(NULL)
{
  initialize();
}

CascadeTrainI::~CascadeTrainI()
{
  delete mTrainProps;
}

void CascadeTrainI::setServiceManager(ServiceManagerI *serv)
{
  mServiceMan = serv;
}

void CascadeTrainI::initialize()
{
  // Create TrainerPropertiesI class to allow the user to modify training 
  // parameters
  mTrainProps = new TrainerPropertiesI();

  // Fill in the trainProps with default values;
  mTrainProps->numStages = 20;
  //mTrainProps->featureType = CvFeatureParams::HAAR; // HAAR, LBP, HOG;
  mTrainProps->featureType = CvFeatureParams::LBP; // HAAR, LBP, HOG;
  mTrainProps->boost_type = CvBoost::GENTLE;  // CvBoost::DISCRETE, REAL, LOGIT
  mTrainProps->minHitRate = 0.995F;
  mTrainProps->maxFalseAlarm = 0.5F;
  mTrainProps->weight_trim_rate = 0.95F; // From opencv/modules/ml/src/boost.cpp
  mTrainProps->max_depth = 1;
  mTrainProps->weak_count = 100;
  mTrainProps->windowSize.width = 25;
  mTrainProps->windowSize.height = 25;
  mTrainProps->sampleSize.width = 25;
  mTrainProps->sampleSize.height = 25;
  
  fInitialized = true;
  
  fInitialized = true;
}

std::string CascadeTrainI::getName(const Current& current)
{
  return mServiceMan->getServiceName();
}
std::string CascadeTrainI::getDescription(const Current& current)
{
  return "OpenCVCascadeTrainer: OpenCV Cascade trainer";
}

TrainerProperties CascadeTrainI::getTrainerProperties(const Current &current)
{
  mTrainProps->writeProps();
  return *mTrainProps;
}

/**
 * Prepend the CVAC_DataDir to the runset's filepath directories.  
 * @param The runset to modify
 * @param The directory path to prepend.
 **/
void CascadeTrainI::addDataPath(RunSet runset, const std::string &CVAC_DataDir)
{
    unsigned int i;
    for (i = 0; i < runset.purposedLists.size(); i++)
    {
        cvac::PurposedLabelableSeq* lab = static_cast<cvac::PurposedLabelableSeq*>(runset.purposedLists[i].get());
        // expand the file names
        LabelableList artifacts = lab->labeledArtifacts;
        std::vector<LabelablePtr>::iterator it;
        for (it = artifacts.begin(); it < artifacts.end(); it++)
        {
            LabelablePtr lptr = (*it);
            Substrate sub = lptr->sub;
            FilePath  filePath = sub.path;
            std::string fname;
            fname = cvac::getFSPath(filePath, CVAC_DataDir);
            filePath.directory.relativePath = getFileDirectory(fname);
            filePath.filename = getFileName(fname);
            sub.path = filePath;
            lptr->sub = sub;
        }
    }
}


void CascadeTrainI::writeBgFile(cvac::RunSetWrapper& rsw, 
                                const string& bgFilename, int* pNumNeg, string CVAC_DataDir,
                                const CallbackHandlerPrx &callback
                                )
{
  // TODO: iterate over NEGATIVE purposes only, count numNeg, write 
  // file names to bgFilename as we did for the old OpenCV Performance training;
  // something like:
  // Constraints con; // has a bunch of defaults
  // con.compatiblePurpose = NEGATIVE;
  // con.substrateType = IMAGES;
  // con.mimeTypes = { "jpg", "png", "bmp" };
  // con.spacesInFilenamesPermitted = false;
  // LabelableIterator it = rsw.iterator( con );
  // for labelable in it ...
  // Save stored data from RunSet to OpenCv negative samples file


  std::vector<RectangleLabels> negRectlabels;
  int imgCnt = 0;
// Fetch all the neg samples
  RunSetConstraint constraint;
  constraint.compatiblePurpose.ptype = NEGATIVE;
  constraint.spacesInFilenamesPermitted = false;
  constraint.excludeLostFrames = true;
  constraint.excludeOccludedFrames = true;
  constraint.addType("png");
  constraint.addType("tif");
  constraint.addType("jpg");


  imgCnt = cvac::processLabelArtifactsToRects(rsw, constraint, 
                                       CVAC_DataDir, mServiceMan,
                                       callback, NULL, 1,
                                       &negRectlabels, false);
  
  ofstream backgroundFile;
  backgroundFile.open(bgFilename.c_str());
  int cnt = 0;
  std::vector<cvac::RectangleLabels>::iterator it;
  for (it = negRectlabels.begin(); it < negRectlabels.end(); it++)
  {
    // fileName, # of objects, x, y, width, height
    cvac::RectangleLabels recLabel = *it;
    backgroundFile << recLabel.filename;
    cnt++;
    // NO EXTRA BLANK LINE after the last sample, or cvhaartraining.cpp can fail on: "CV_Assert(elements_read == 1);"
    if ((cnt) < imgCnt)
        backgroundFile << endl;
  }
  backgroundFile.flush();
  backgroundFile.close();
  // Clean up any memory 
  cvac::cleanupRectangleLabels(&negRectlabels);
  *pNumNeg = cnt;
}

/**
 * Function called by ProcessLabelArtifactsToRects that returns the
 * size of an image.
 */
bool static getImageWidthHeight(std::string filename, int &width, int &height)
{
   IplImage* img = cvLoadImage(filename.c_str());
   bool res;
   if( !img )
   {
       width = 0;
       height = 0;
       res = false;
   } else
   {
       width = img->width;
       height = img->height;
       res = true;
   }
   cvReleaseImage( &img );
   return res;
}

int CascadeTrainI::addRotatedSamples(string tempVec, string vecFilename, string filename, const char *bgInfoFile, int numPos, int showSamples, int w, int h)
{
	 // Rotate max of 15 degrees on the z axis and 5 degrees on x, y.  
	 
     cvCreateTrainingSamples(tempVec.c_str(), filename.c_str(), 0, 0, bgInfoFile, mTrainProps->rotate_count, 0, 0,
                                    0.0872, 0.0872, 0.2618, showSamples, w, h);
	 // now merge new vec file info main one.
     icvMergeVecVec(tempVec.c_str(), vecFilename.c_str(), numPos, showSamples, w, h );
     return mTrainProps->rotate_count;
}

bool CascadeTrainI::createSamples( RunSetWrapper& rsw, 
                                   const SamplesParams& params,
                    const string& infoFilename,
                    const string& vecFilename, int* pNumPos, string CVAC_DataDir,
                    const CallbackHandlerPrx &callback, const string &bgInfo, int bgCnt
                    )
{
 
  bool showsamples = false;

  std::vector<RectangleLabels> posRectlabels;
  int imgCnt = 0;
  // Fetch all the positive samples
  RunSetConstraint constraint;
  constraint.compatiblePurpose.ptype = POSITIVE;
  constraint.spacesInFilenamesPermitted = false;
  constraint.excludeLostFrames = true;
  constraint.excludeOccludedFrames = true;
  constraint.addType("png");
  constraint.addType("tif");
  constraint.addType("jpg");

  imgCnt = cvac::processLabelArtifactsToRects(rsw, constraint, 
                                       CVAC_DataDir, mServiceMan,
                                       callback, getImageWidthHeight, 1,
                                       &posRectlabels, false);

  ofstream infoFile;
 
  infoFile.open(infoFilename.c_str());

  // Save stored data from RunSet to OpenCv positive samples .dat file
  int cnt = 0;
  std::vector<cvac::RectangleLabels>::iterator it;
  for (it = posRectlabels.begin(); it < posRectlabels.end(); it++)
  {
    cvac::RectangleLabels recLabel = *it;
    bool skipFile = false;
    if (recLabel.rects.size() <= 0)
    { // No rectangle so use the whole image
        int w, h;
        getImageWidthHeight(recLabel.filename, w, h);
        infoFile << recLabel.filename << " 1 0 0 " << w << " " << h;
    }else 
    {
       int rectCnt = 0;  // Only add labels that are as large as the window size we are using!
       std::vector<cvac::BBoxPtr>::iterator rit;
       // Get count of valid size rectangles.
       for (rit = recLabel.rects.begin(); rit < recLabel.rects.end(); rit++)
       {
           cvac::BBoxPtr rect = *rit;
           if (rect->width < params.width || rect->height < params.height)
               continue;  // dont' count this rect.
           rectCnt++;
       }
       if (rectCnt == 0)
       {
           int w, h;
           getImageWidthHeight(recLabel.filename, w, h);
           if (w >= params.width && h >= params.height)
               infoFile << recLabel.filename << " 1 0 0 " << w << " " << h;
           else
               skipFile = true;
       } else
       {
          // fileName, # of objects, x, y, width, height
          infoFile << recLabel.filename << " " <<
                      rectCnt << " ";

          for (rit = recLabel.rects.begin(); rit < recLabel.rects.end(); rit++)
          {
              cvac::BBoxPtr rect = *rit;
              if (rect->width >= params.width && rect->height >= params.height)
                  infoFile << rect->x << " " << rect->y << " " << rect->width <<
                          " " << rect->height  << " ";
          }
       }   
    }
    if (skipFile == false)
    {
        cnt++;
    }
    // NO EXTRA BLANK LINE after the last sample, or cvhaartraining.cpp can fail on: "CV_Assert(elements_read == 1);"
    if ((cnt < imgCnt) && skipFile == false)
        infoFile << endl;
  }
  infoFile.flush();
  infoFile.close();
  
  // Save stored data from RunSet to OpenCv negative samples file
  int numPos;
  numPos = cvCreateTrainingSamplesFromInfo( infoFilename.c_str(), 
                                              vecFilename.c_str(), 
                                              cnt, showsamples,
                                              params.width, params.height
                                             );
  // Add geneated samples to vec file if properties say so
  if (mTrainProps->rotate_count > 0)
  {
      std::vector<cvac::RectangleLabels>::iterator it;
      printf("Adding %d rotated images per sample\n", mTrainProps->rotate_count);
      for (it = posRectlabels.begin(); it < posRectlabels.end(); it++)
      {
          cvac::RectangleLabels recLabel = *it;
          string tempVec = "tempVec";
		  string tempImg = "tempImage.jpg";
          // Use background images as background if we have enough
          const char *bgInfoFile = NULL;
          if (bgCnt >= mTrainProps->rotate_count)
          {
              bgInfoFile = bgInfo.c_str();
          }
		  if (recLabel.rects.size() <= 0)
          { // No rectangle so rotate the whole image
              int w, h;
              getImageWidthHeight(recLabel.filename, w, h);
			  if (w < params.width || h < params.height)
                  continue;
			  if (w < params.width*3 && h < params.height *3)
			  {
				  numPos += addRotatedSamples(tempVec, vecFilename, recLabel.filename, bgInfoFile, numPos, showsamples, params.width, params.height);
			  }else
			  { // shrink the image down before rotating so we don't crash
				  cv::Mat mat = cv::imread(recLabel.filename.c_str(), 0);
				  cv::Size mysize(params.width*2, params.height*2);
				  cv::Mat sizeMat;
				  cv::resize(mat, sizeMat, mysize, CV_INTER_AREA);
				  if (cv::imwrite(tempImg.c_str(), sizeMat))
				  {
					  numPos += addRotatedSamples(tempVec, vecFilename, tempImg, bgInfoFile, numPos, showsamples, params.width, params.height);
				  }
			  }
        
          }else 
          {
           
              std::vector<cvac::BBoxPtr>::iterator rit;
              // Create a temp image with just the cropped part and rotate that and add to the vec file
              for (rit = recLabel.rects.begin(); rit < recLabel.rects.end(); rit++)
              {
                  cvac::BBoxPtr rect = *rit;
                  if (rect->width < params.width || rect->height < params.height)
                     continue;  // Skip.
                  // Create a temp image of just the part we want
				  cv::Mat mat = cv::imread(recLabel.filename.c_str(), 0);
				  cv::Rect myrect(rect->x, rect->y, rect->width, rect->height);
				  cv::Mat rMat;
				  cv::Mat(mat, myrect).copyTo(rMat);
				  if (rect->width > params.width*3 || rect->height > params.height *3)
				  {
					   cv::Size mysize(params.width*2, params.height*2);
				       cv::Mat sizeMat;
				       cv::resize(mat, sizeMat, mysize, CV_INTER_AREA);
					   sizeMat.copyTo(rMat);
				  }
				  if (cv::imwrite(tempImg.c_str(), rMat))
				  {
					  numPos += addRotatedSamples(tempVec, vecFilename, tempImg, bgInfoFile, numPos, showsamples, params.width, params.height);
				  }
			  }
		  }
       }
       
       printf("Adding rotated images complete\n");
  }
  // Clean up any memory 
  //debug
  //printf("Using vec file %s\n", vecFilename.c_str());
  cvac::cleanupRectangleLabels(&posRectlabels);
  *pNumPos = numPos;
  return true;
}


bool CascadeTrainI::createClassifier( const string& tempDir, 
                       const string& vecFname, const string& bgName,
                       int numPos, int numNeg, 
                       const TrainerPropertiesI *trainProps)
{
  CvCascadeClassifier classifier;
  int precalcValBufSize = 256,
      precalcIdxBufSize = 256;
  bool baseFormatSave = false;
  CvCascadeParams cascadeParams;
  cascadeParams.winSize.width = trainProps->windowSize.width;
  cascadeParams.winSize.height = trainProps->windowSize.height;
  CvCascadeBoostParams stageParams;
  Ptr<CvFeatureParams> featureParams[] = 
  { Ptr<CvFeatureParams>(new CvHaarFeatureParams),
    Ptr<CvFeatureParams>(new CvLBPFeatureParams),
    Ptr<CvFeatureParams>(new CvHOGFeatureParams)
  };
  cascadeParams.featureType = trainProps->featureType;
  stageParams.boost_type = trainProps->boost_type;

  bool res = classifier.train( tempDir,
                    vecFname,
                    bgName,
                    numPos, numNeg,
                    precalcValBufSize, precalcIdxBufSize,
                    trainProps->numStages,
                    cascadeParams,
                    *featureParams[cascadeParams.featureType],
                    stageParams,
                    baseFormatSave );  
  return res;
}

/** argument error checking: any data? consistent multiclass or pos/neg purpose?
 *  having Negative purpose samples and multiclass is fine, but
 *  having Positive and multiclass is probably incorrect.  WARN.
 */
const static int MIN_SAMPLE_SIZE = 8;

bool CascadeTrainI::checkPurposedLists(
    const PurposedListSequence& purposedLists,
    TrainerCallbackHandlerPrx& _callback )
{
  if(purposedLists.size() == 0)
  {
    localAndClientMsg(VLogger::WARN, _callback, 
                      "Error: no data (runset) for processing\n");
    return false;
  }

  bool havemul = false;
  bool havepos = false;
  bool haveneg = false;
  bool tooSmall = false;

  for (size_t listidx = 0; listidx < purposedLists.size(); listidx++)
  {
    Purpose &pur = purposedLists[listidx]->pur;
    PurposedLabelableSeq * purSeq = NULL;
    purSeq = static_cast<PurposedLabelableSeq *>(purposedLists[listidx].get());
    LabelableList artifacts = purSeq->labeledArtifacts;
    
    switch(pur.ptype)
    {
      case cvac::POSITIVE:
      {
        havepos = true;
        break;
      }
      case cvac::NEGATIVE:
      {
        haveneg = true;
        break;
      }
      case cvac::MULTICLASS:
      {
        havemul = true;
        break;
      }
    }
    if ((artifacts.size() < MIN_SAMPLE_SIZE))
    {
        bool hasVideo = false;
        LabelableList::iterator it;
        for (it = artifacts.begin(); it < artifacts.end(); it++)
        {
            LabelablePtr lab = *it;
            if (lab->sub.isVideo)
            {
                hasVideo = true;
                break;
            }
        }
        if (hasVideo == false)
            tooSmall = true;
    }
 
  }
  
  if (havepos == false)
  {
    localAndClientMsg(VLogger::ERROR, _callback, "Your runset does not contain a pos purpose\n");  
    return false;
  }
  if (haveneg == false)
  {
    localAndClientMsg(VLogger::ERROR, _callback, "Your runset does not contain a neg purpose\n");       
    return false;
  }
  if (tooSmall == true)
  {
    localAndClientMsg(VLogger::ERROR, _callback,
                      "Your runset must contain at least %d+%d (pos+neg) samples\n",
                      MIN_SAMPLE_SIZE, MIN_SAMPLE_SIZE);       
    return false;
  }
  return true;
}

void CascadeTrainI::process(const Identity &client, const RunSet& runset,
                            const TrainerProperties& trainProps,
                            const Current& current)
{	
  mTrainProps->load(trainProps);
  // Obtain CVAC verbosity - TODO: this should happen earlier
  PropertiesPtr svcprops = current.adapter->getCommunicator()->getProperties();
  string verbStr = svcprops->getProperty("CVAC.ServicesVerbosity");
  if (!verbStr.empty())
  {
    vLogger.setLocalVerbosityLevel( verbStr );
  }
  
  TrainerCallbackHandlerPrx callback =
    TrainerCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());

  // check the validity of the runset.
  if (!checkPurposedLists( runset.purposedLists, callback ))
    return;
  // Get the remote client name to use to save cascade file 
  std::string connectName = cvac::getClientConnectionName(current);
  const std::string CVAC_DataDir = svcprops->getProperty("CVAC.DataDir");

  if(runset.purposedLists.size() == 0)
  {
    string _resStr = "Error: no data (runset) for processing\n";
    localAndClientMsg(VLogger::WARN, callback, _resStr.c_str());
    return;
  }
  // Since createSamples fails if there is a space in a file name we will create a temporary runset
  // and provide symbolic links to files that name spaces in there names.
  //cvac::RunSet tempRunSet = runset;
  // Add the cvac data dir to the directories in the runset
  //addDataPath(tempRunSet, CVAC_DataDir);
  // The symbolic links are created in a tempdir so lets remember it so we can delete it at the end
  //std::string tempRSDir = fixupRunSet(tempRunSet, CVAC_DataDir);
  // Iterate over runset, inserting each POSITIVE Labelable into
  // the input file to "createsamples".  Add each NEGATIVE into
  // the bgFile.  Put both created files into a tempdir.
  std::string clientName = mServiceMan->getSandbox()->createClientName(mServiceMan->getServiceName(),
                                                             connectName);
  std::string tempDir = mServiceMan->getSandbox()->createTrainingDir(clientName);
  //RunSetWrapper rsw( &tempRunSet, CVAC_DataDir, mServiceMan );
  RunSetWrapper rsw( &runset, CVAC_DataDir, mServiceMan );
  // We can't put the bgName and infoName in the tempdir without
  // changing cvSamples since it assumes that this files location is the root
  // directory for the data.
  //string bgName = tempDir + "/cascade_negatives.txt";
  //string infoName = tempDir + "/cascade_positives.txt";
  string bgName = "cascade_negatives.txt";
  string infoName = "cascade_positives.txt";
  int numNeg = 0;
  
  writeBgFile( rsw, bgName, &numNeg, CVAC_DataDir, callback );


  // set parameters to createsamples
  SamplesParams samplesParams;
  samplesParams.numSamples = 1000;
  if (mTrainProps->sampleSize.width != 0)
      samplesParams.width = mTrainProps->sampleSize.width;
  else
      samplesParams.width = mTrainProps->windowSize.width;
  if (mTrainProps->sampleSize.height != 0)
      samplesParams.height = mTrainProps->sampleSize.height;
  else
      samplesParams.height = mTrainProps->windowSize.height;

  // run createsamples
  std::string vecFname = tempDir + "/cascade_positives.vec";
  int numPos = 0;
  
  createSamples( rsw, samplesParams, infoName, vecFname, &numPos, CVAC_DataDir, callback,
                 bgName, numNeg);

  // Tell ServiceManager that we will listen for stop
  mServiceMan->setStoppable();

  bool created = createClassifier( tempDir, vecFname, bgName,
                    numPos, numNeg, mTrainProps );

  // Tell ServiceManager that we are done listening for stop
  mServiceMan->clearStop();  
  if (created)
  {
      
      DetectorDataArchive dda;
      std::string clientDir = mServiceMan->getSandbox()->createClientDir(clientName);
      std::string archiveFilename = getDateFilename(clientDir,  "cascade")+ ".zip";
      dda.setArchiveFilename(archiveFilename);
      dda.addFile(XMLID, tempDir + "/cascade.xml");
      dda.createArchive(tempDir);
      mServiceMan->getSandbox()->deleteTrainingDir(clientName);
      FilePath detectorData;
      detectorData.filename = getFileName(archiveFilename);
      std::string relDir;
      int idx = clientDir.find(CVAC_DataDir.c_str(), 0, CVAC_DataDir.length());
      if (idx == 0)
      {
          relDir = clientDir.substr(CVAC_DataDir.length() + 1);
      }else
      {
          relDir = clientDir;
      }
      detectorData.directory.relativePath = relDir; 
      callback->createdDetector(detectorData);

      localAndClientMsg(VLogger::INFO, callback, "Cascade training done.\n");
      
  }else
  {
      localAndClientMsg(VLogger::INFO, callback, "Cascade training failed.\n");
  }
  //deleteDirectory(tempRSDir);
}

//----------------------------------------------------------------------------
TrainerPropertiesI::TrainerPropertiesI()
{
    verbosity = 0;
    canSetWindowSize = true;
    canSetSensitivity = true;
    videoFPS = 0.0;
    windowSize.width = 0;
    windowSize.height = 0;
    sampleSize.width = 0;
    sampleSize.height = 0;
    falseAlarmRate = 0.0;
    recall = 0.0;
    rotate_count = 0;
}

void TrainerPropertiesI::load(const TrainerProperties &p) 
{
    canSetWindowSize = true;
    canSetSensitivity = true;
    verbosity = p.verbosity;
    props = p.props;
    if (p.videoFPS > 0.0)
        videoFPS = p.videoFPS;
    //Only load values that are not zero
    if (p.windowSize.width > 0 && p.windowSize.height > 0)
        windowSize = p.windowSize;
    if (p.falseAlarmRate > 0.0)
        falseAlarmRate = p.falseAlarmRate;
    if (p.recall > 0.0)
        recall = p.recall;
    readProps();
}

bool TrainerPropertiesI::readProps()
{
    bool res = true;
    cvac::Properties::iterator it;
    for (it = props.begin(); it != props.end(); it++)
    {
        if (it->first.compare("numStages") == 0)
        {
            numStages = atoi(it->second.c_str());
        }else if (it->first.compare("featureType") == 0)
        {
            if (it->second.compare("HAAR") == 0)
                featureType = CvFeatureParams::HAAR;
            else if (it->second.compare("LBP") == 0)
                featureType = CvFeatureParams::LBP;
            else if (it->second.compare("HOG") == 0)
                featureType = CvFeatureParams::HOG;
            else 
            {
                localAndClientMsg(VLogger::ERROR, NULL, 
                         "featureType not supported for Cascade training.\n");
                res = false;
            }
        }else if (it->first.compare("boostType") == 0)
        {
            if (it->second.compare("DISCRETE") == 0)
                boost_type = CvBoost::DISCRETE;
            else if (it->second.compare("REAL") == 0)
                boost_type = CvBoost::REAL;
            else if (it->second.compare("LOGIT") == 0)
                boost_type = CvBoost::LOGIT;
            if (it->second.compare("GENTLE") == 0)
                boost_type = CvBoost::GENTLE;
            else 
            {
                localAndClientMsg(VLogger::ERROR, NULL, 
                         "boostType not supported for Cascade training.\n");
                res = false;
            }
        }else if (it->first.compare("weightTrimRate") == 0)
        {
            weight_trim_rate = (float)atof(it->second.c_str());
        }else if (it->first.compare("maxDepth") == 0)
        {
            max_depth = atoi(it->second.c_str());
        }else if (it->first.compare("weakCount") == 0)
        {
            weak_count = atoi(it->second.c_str());
        }else if (it->first.compare("rotateSamples") == 0)
        {
            rotate_count = atoi(it->second.c_str());
        }else if (it->first.compare("sampleImageWidth") == 0)
        {
            sampleSize.width = atoi(it->second.c_str());
        }else if (it->first.compare("sampleImageHeight") == 0)
        {
            sampleSize.height = atoi(it->second.c_str());
        }
    }
   
    return res;
}

bool TrainerPropertiesI::writeProps()
{
    bool res = true;
    char buff[128];
    sprintf(buff, "%d", numStages);
    props.insert(std::pair<string, string>("numStages", buff));
    if (featureType == CvFeatureParams::HAAR)
        props.insert(std::pair<string, string>("featureType", "HAAR"));
    else if (featureType == CvFeatureParams::LBP)
        props.insert(std::pair<string, string>("featureType", "LBP"));
    else if (featureType == CvFeatureParams::HOG)
        props.insert(std::pair<string, string>("featureType", "HOG"));
    else 
    {
        localAndClientMsg(VLogger::ERROR, NULL, 
                         "featureType not supported for Cascade training.\n");
        res = false;
    }
    if (boost_type == CvBoost::DISCRETE)
        props.insert(std::pair<string, string>("boostType", "DISCRETE"));
    else if (boost_type == CvBoost::REAL)
        props.insert(std::pair<string, string>("boostType", "REAL"));
    else if (boost_type == CvBoost::LOGIT)
        props.insert(std::pair<string, string>("boostType", "LOGIT"));
    else if (boost_type == CvBoost::GENTLE)
        props.insert(std::pair<string, string>("boostType", "GENTLE"));
    else 
    {
        localAndClientMsg(VLogger::ERROR, NULL, 
                         "boostType not supported for Cascade training.\n");
        res = false;
    }
    sprintf(buff, "%g", weight_trim_rate);
    props.insert(std::pair<string, string>("weightTrimRate", buff));
    sprintf(buff, "%d", max_depth);
    props.insert(std::pair<string, string>("maxDepth", buff));
    sprintf(buff, "%d", weak_count);
    props.insert(std::pair<string, string>("weakCount", buff));
    sprintf(buff, "%d", rotate_count);
    props.insert(std::pair<string, string>("rotateSamples", buff));
    sprintf(buff, "%d", sampleSize.width);
    props.insert(std::pair<string, string>("sampleImageWidth", buff));
    sprintf(buff, "%d", sampleSize.height);
    props.insert(std::pair<string, string>("sampleImageHeight", buff));

    falseAlarmRate = maxFalseAlarm;
    recall = minHitRate;
    return res;
}

// TODO: this is the old main function; here only for reference.  remove 
// once no longer needed
int nomain( int argc, char* argv[] )
{
    CvCascadeClassifier classifier;
    String cascadeDirName, vecName, bgName;
    int numPos    = 2000;
    int numNeg    = 1000;
    int numStages = 20;
    int precalcValBufSize = 256,
        precalcIdxBufSize = 256;
    bool baseFormatSave = false;

    CvCascadeParams cascadeParams;
    CvCascadeBoostParams stageParams;
    Ptr<CvFeatureParams> featureParams[] = { Ptr<CvFeatureParams>(new CvHaarFeatureParams),
                                             Ptr<CvFeatureParams>(new CvLBPFeatureParams),
                                             Ptr<CvFeatureParams>(new CvHOGFeatureParams)
                                           };
    int fc = sizeof(featureParams)/sizeof(featureParams[0]);
    if( argc == 1 )
    {
        cout << "Usage: " << argv[0] << endl;
        cout << "  -data <cascade_dir_name>" << endl;
        cout << "  -vec <vec_file_name>" << endl;
        cout << "  -bg <background_file_name>" << endl;
        cout << "  [-numPos <number_of_positive_samples = " << numPos << ">]" << endl;
        cout << "  [-numNeg <number_of_negative_samples = " << numNeg << ">]" << endl;
        cout << "  [-numStages <number_of_stages = " << numStages << ">]" << endl;
        cout << "  [-precalcValBufSize <precalculated_vals_buffer_size_in_Mb = " << precalcValBufSize << ">]" << endl;
        cout << "  [-precalcIdxBufSize <precalculated_idxs_buffer_size_in_Mb = " << precalcIdxBufSize << ">]" << endl;
        cout << "  [-baseFormatSave]" << endl;
        cascadeParams.printDefaults();
        stageParams.printDefaults();
        for( int fi = 0; fi < fc; fi++ )
            featureParams[fi]->printDefaults();
        return 0;
    }

    for( int i = 1; i < argc; i++ )
    {
        bool set = false;
        if( !strcmp( argv[i], "-data" ) )
        {
            cascadeDirName = argv[++i];
        }
        else if( !strcmp( argv[i], "-vec" ) )
        {
            vecName = argv[++i];
        }
        else if( !strcmp( argv[i], "-bg" ) )
        {
            bgName = argv[++i];
        }
        else if( !strcmp( argv[i], "-numPos" ) )
        {
            numPos = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-numNeg" ) )
        {
            numNeg = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-numStages" ) )
        {
            numStages = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-precalcValBufSize" ) )
        {
            precalcValBufSize = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-precalcIdxBufSize" ) )
        {
            precalcIdxBufSize = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-baseFormatSave" ) )
        {
            baseFormatSave = true;
        }
        else if ( cascadeParams.scanAttr( argv[i], argv[i+1] ) ) { i++; }
        else if ( stageParams.scanAttr( argv[i], argv[i+1] ) ) { i++; }
        else if ( !set )
        {
            for( int fi = 0; fi < fc; fi++ )
            {
                set = featureParams[fi]->scanAttr(argv[i], argv[i+1]);
                if ( !set )
                {
                    i++;
                    break;
                }
            }
        }
    }

    classifier.train( cascadeDirName,
                      vecName,
                      bgName,
                      numPos, numNeg,
                      precalcValBufSize, precalcIdxBufSize,
                      numStages,
                      cascadeParams,
                      *featureParams[cascadeParams.featureType],
                      stageParams,
                      baseFormatSave );
    return 0;
}

bool CascadeTrainI::cancel(const Identity &client, const Current& current)
{
  localAndClientMsg(VLogger::WARN, NULL, "cancel not implemented.");
  return false;
}
