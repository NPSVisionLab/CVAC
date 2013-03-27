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
#include <util/ServiceMan.h>

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
using namespace std;

class bowCV {
public:
	bowCV();
	~bowCV();	
	
	bool			train_initialize(const string& _detectorName,const string& _extractorName,const string& _matcherName,int _nCluster);
	bool			train_parseTrainList(const string& _filepathTrain,const string& _filenameTrainList);
	void			train_stackTrainImage(const string& _fullpath,const int& _classID);	
	void			train_stackTrainImage(const string& _fullpath,const int& _classID,const int& _x,const int& _y,const int& _width,const int& _height);
	bool			train_run(const string& _filepathForSavingResult,const string& _filenameForSavingResult, 
                              cvac::ServiceManager *);

	bool			detect_initialize(const string& _filepath,const string& _filename);	
	bool			detect_setParameter(const string& _detectorName,const string& _extractorName,const string& _matcherName);	
	bool			detect_readTrainResult(const string& _filepath,const string& _filename);	
	bool			detect_run(const string& _fullfilename, int& _bestClass);
	

private:
	bool			train_writeVocabulary(const string& _filename,const Mat& _vocabulary);
	bool			detect_readVocabulary( const string& _filename, Mat& _vocabulary );
	//bool			runTrainFull(const string& _filepathTrain,const string& _filenameTrainList,const string& _filepathForSavingResult,const string& _filenameForSavingResult);	//This function is not good to the ICE project.
	//void			setSVMParams( CvSVMParams& svmParams, CvMat& class_wts_cv, const Mat& responses, bool balanceClasses );
	//void			setSVMTrainAutoParams( CvParamGrid& c_grid, CvParamGrid& gamma_grid,CvParamGrid& p_grid, CvParamGrid& nu_grid,CvParamGrid& coef_grid, CvParamGrid& degree_grid );	
	
protected:
	Mat					_img;
	Mat					_descriptors;
	vector<KeyPoint>	_keypoints;
	string				_tfileName;
	char				_buf[255];
	string				_fullFilePathImg;
	string				_fullFilePathList;
	Mat					mVocabulary;
	string				filenameTrainResult;
	string				filenameVocabulary;	
	

private:
	int								cntCluster;
	bool							flagName;	
	bool							flagTrain;
	Ptr<FeatureDetector>			fDetector;	
	Ptr<DescriptorExtractor>		dExtractor;
	Ptr<DescriptorMatcher>			dMatcher;
	string							nameDetector;
	string							nameExtractor;
	string							nameMatcher;
	Ptr<BOWImgDescriptorExtractor>	bowExtractor;
	CvSVM							classifierSVM;
	std::vector<string>				vFilenameTrain;
	std::vector<int>				vClassIDTrain;
	std::vector<int>				vBoundX,vBoundY;
	std::vector<int>				vBoundWidth,vBoundHeight;
};
#endif	//_BOWCV_H__
