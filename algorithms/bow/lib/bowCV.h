#ifndef _BOWCV_H__
/****
 *CVAC Software Disclaimer
 *
 *This software was developed at the Naval Postgraduate School, Monterey, CA,
 *by employees of the Federal Government in the course of their official duties.
 *Pursuant to title 17 Section 105 of the United States Code this software
 *is not subject to copyright protection and is in the public domain. It is 
 *an experimental system.  The Naval Postgraduate School assumes no
 *responsibility whatsoever for its use by other parties, and makes
 *no guarantees, expressed or implied, about its quality, reliability, 
 *or any other characteristic.
 *We would appreciate acknowledgement and a brief notification if the software
 *is used.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above notice,
 *      this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the Naval Postgraduate School, nor the name of
 *      the U.S. Government, nor the names of its contributors may be used
 *      to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****/
#define _BOWCV_H__

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <util/ServiceMan.h>
#include <util/DetectorDataArchive.h>

#include <opencv2/opencv.hpp>
// #include <opencv2/opencv.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>	//this should be included for using SIFT or SURF
// #include <opencv2/ml/ml.hpp>

// #ifdef _DEBUG
// #pragma comment(lib,"opencv_core242d.lib")
// #pragma comment(lib,"opencv_highgui242d.lib")
// #pragma comment(lib,"opencv_ml242d.lib")
// #pragma comment(lib,"opencv_nonfree242d.lib")
// #pragma comment(lib,"opencv_features2d242d.lib")
// #else
// #pragma comment(lib,"opencv_core242.lib")
// #pragma comment(lib,"opencv_highgui242.lib")
// #pragma comment(lib,"opencv_ml242.lib")
// #pragma comment(lib,"opencv_nonfree242.lib")
// #pragma comment(lib,"opencv_features2d242.lib")
// #endif


using namespace cv;

class bowCV {
public:

  static const std::string BOW_VOC_FILE, BOW_SVM_FILE, BOW_DETECTOR_NAME,
    BOW_EXTRACTOR_NAME, BOW_MATCHER_NAME, BOW_OPENCV_VERSION, BOW_ONECLASS_ID;
  
public:
  bowCV();
  ~bowCV();	

  bool  isInitialized();
  bool  train_initialize(const std::string& _detectorName,
                         const std::string& _extractorName,
                         const std::string& _matcherName,int _nCluster,
                         cvac::DetectorDataArchive* dda);
  void  train_stackTrainImage(const std::string& _fullpath,const int& _classID);	
  void  train_stackTrainImage(const std::string& _fullpath,const int& _classID,
                              const int& _x,const int& _y,
                              const int& _width,const int& _height);
  bool  train_run(const std::string& _filepathForSavingResult,
                  cvac::ServiceManager *,
                  float _oneclassNu = 0.1);  

  bool  detect_initialize( const cvac::DetectorDataArchive* dda);	
  bool  detect_run(const std::string& _fullfilename, int& _bestClass,
                   int _boxX=0,int _boxY=0,int _boxWidth=0,int _boxHeight=0);


private:
  bool  isCompatibleOpenCV(const std::string& _version);
  bool  train_parseTrainList(const std::string& _filepathTrain,
                             const std::string& _filenameTrainList);
  bool    train_writeVocabulary(const std::string& _filename,const Mat& _vocabulary);

  bool    detect_readVocabulary( const std::string& _filename, Mat& _vocabulary );
  bool  detect_setParameter(const std::string& _detectorName,
                            const std::string& _extractorName,
                            const std::string& _matcherName);	
  bool  detect_readTrainResult();	

  //bool  runTrainFull(const std::string& _filepathTrain,const std::string& _filenameTrainList,const std::string& _filepathForSavingResult,const std::string& _filenameForSavingResult);	//This function is not good to the ICE project.
  //void  setSVMParams( CvSVMParams& svmParams, CvMat& class_wts_cv, const Mat& responses, bool balanceClasses );
  //void  setSVMTrainAutoParams( CvParamGrid& c_grid, CvParamGrid& gamma_grid,CvParamGrid& p_grid, CvParamGrid& nu_grid,CvParamGrid& coef_grid, CvParamGrid& degree_grid );	

public:
  std::string  filenameVocabulary;	
  std::string  filenameSVM;
  cvac::DetectorDataArchive* dda;

protected:
  Mat                    _img;
  Mat                    _descriptors;
  vector<KeyPoint>       _keypoints;
  std::string            _tfileName;
  char                   _buf[255];
  std::string            _fullFilePathImg;
  std::string            _fullFilePathList;
  Mat                    mVocabulary;
  std::string            filenameTrainResult;
  
private:
  int   cntCluster;
  int   mInclassIDforOneClass;
  int   mOutclassIDforOneClass;
  bool  flagOneClass;
  bool  flagName;	
  bool  flagTrain;  
  Ptr<FeatureDetector>      fDetector;	
  Ptr<DescriptorExtractor>  dExtractor;
  Ptr<DescriptorMatcher>    dMatcher;  
  Ptr<BOWImgDescriptorExtractor>  bowExtractor;
  CvSVM  classifierSVM;
  std::vector<std::string>  vFilenameTrain;
  std::vector<int>          vClassIDTrain;
  std::vector<int>          vBoundX,vBoundY;
  std::vector<int>          vBoundWidth,vBoundHeight;
};

#endif	//_BOWCV_H__
