#ifndef _SBDCV_H__
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
#define _SBDCV_H__

#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include <opencv2/opencv.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/imgproc/imgproc.hpp>
// #include <opencv2/ml/ml.hpp>

// Necessary OpenCV Lib
//opencv_core242.lib
//opencv_highgui242.lib
//opencv_imgproc242.lib
//opencv_ml242.lib

//#define _FULL_LOG_FOR_DEBUG
//#define _SRC_CHECK_FOR_DEBUG

using namespace cv;
using namespace std;

// #ifdef WINDOWS
// #ifdef DLL_EXPORT
// #define SBDCV_DLL	__declspec(dllexport)
// #else
// #define SBDCV_DLL	__declspec(dllimport)
// #endif
// #else // WINDOWS
// #define SBDCV_DLL
// #endif

#if defined( WIN32 ) || defined( WIN64 )
#ifdef DLL_EXPORT
#define SBDLIBRARY_DLL	//__declspec(dllexport)
#else
#define SBDLIBRARY_DLL	//__declspec(dllimport)
#endif
#else // windows
#define SBDLIBRARY_DLL
#define BOOL bool
#define TRUE 1
#define FALSE 0
#endif // windows

class SBDLIBRARY_DLL SBDCV {
public:
	SBDCV();
	~SBDCV();
	//////////////////////////////////////////////////////////////////////////
	// For ICE only
	bool			train_extractFeature(string _path,string _videoFileName,std::vector<long>& _vFrameList,bool _doInitialize = true);	//after it, run train_SVM();			
	bool			test_loadSVM(string _commonPath,string _SVMfilename);		
	bool			test_run(string _fullfileNameVideo,std::vector<long>& _outFrameList,bool _flagTxtSave,std::string& _outFrameFileName,bool _flagImgSave);	
	//////////////////////////////////////////////////////////////////////////
	// For Common Use	
	bool			train_trainSVM();	//SHOULD BE PERFORMED after extracting features
	bool			train_saveSVM(string _commonPath,string _filename);	
	bool			train_saveFeature(string _commonPath,string _filename);			

	//////////////////////////////////////////////////////////////////////////
	// For easy Debugging 
	bool			setFeatureFromTrainList(string _commonPath,string _trainListFileName,bool _doInitialize = true);	//loading a list file and performing SVM. after that, run train_SVM();		
	bool			loadFeature(string _commonPath,string _filename,bool _flagClear = false);	
	bool			runTestFromFeature(string _commonPath,string _fileNameVideo,bool _flagTxtSave);	//for debugging

	void			osxTest();	//restore after fixing osx problem
	
private:	
	bool			trainSVM();
	bool			saveSVM(string _commonPath,string _filename);
	bool			loadSVM(string _commonPath,string _SVMfilename);

	bool			loadVideo(string _fullfileNameVideo,long& _nFrame);
	bool			loadFramelist(string _fileNameFrameList,std::vector<long>& _vFrameList);
	bool			loadTrainlist(string _commonPath,string _trainListFilename,std::vector<string>& _outListVideo,std::vector<string>& _outListFrame);

	bool			setFeature(string _fullfileNameVideo,std::vector<long>& _inFrameList,Mat& _outFeatureBoundary,Mat& _outFeatureNotBoundary);
	bool			extractFeature(Mat& _featureAll,std::vector<float>& _featureNorm);
	bool			separateFeature(Mat& _inFeature,std::vector<float>& _inFeatureNorm,std::vector<long>& _inFrame,Mat& _outFeatureBoundary,Mat& _outFeatureNotBoundary);			
	bool			saveFeature(string _commonPath,string _filename);	

	bool			runTest(string _commonPath,string _fileNameVideo,string& _fileNameResult,std::vector<long>& _outFrameList,bool _flagTxtSave,bool _flagImgSave);	

	float			computeFeature(Mat& srcMatCur,Mat& srcMatNext,std::vector<float>& _vFeature);
	void			computeDifferenceIntensity_Gray(const Mat& _greyCur,const Mat& _greyNext,double& _featureIntensity);
	void			computeEdgeChangeRatio_Gray(const Mat& _greyCur,const Mat& _greyNext,double& _featureEdgeChangeRatio);
	void			computeDifferenceColorHistogram_HSV(const Mat& _HSVCur,const Mat& _HSVNext,double& _featureColorHistogramHSVCorrelation,double& _featureColorHistogramHSVBhattacharyya);
	void			computeDifferenceColorHistogram_RGB(const vector<Mat>& bgrCur,const vector<Mat>& bgrNext,double& _featureColorHistogramRGBCorrelation,double& _featureColorHistogramRGBBhattacharyya);
	void			computeDifferenceColorPixelWise_RGB(const vector<Mat>& bgrCur,const vector<Mat>& bgrNext,double& _featureColorRGBPixelWise);
	void			computeDCT_Gray(const Mat& _greyCur,const Mat& _greyNext,double& _diffValue);

private:	
	VideoCapture	mVideoFile;
	Mat				    mFeature_Boundary,mFeature_NotBoundary;
	int				    mFeatureDim;	
	bool			    fSVM;
	CvSVM			    mSVM;
	CvSVMParams		mSVM_Param;
	int				    mMarginForLocalMax;
};

#endif	//_SBDCV_H__
