#ifndef _VIDEOVALIDATORFOROPENCV_H__
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
#define _VIDEOVALIDATORFOROPENCV_H__

#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include <opencv2/opencv.hpp>
// #include <opencv2/opencv.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/imgproc/imgproc.hpp>

// Necessary Libs
//opencv_core242.lib
//opencv_highgui242.lib
//opencv_imgproc242.lib

using namespace cv;
using namespace std;

#if defined( WIN32 ) || defined( WIN64 )
#ifdef DLL_EXPORT
#define VIDEOVALIDATORFOROPENCV_DLL	__declspec(dllexport)
#else
#define VIDEOVALIDATORFOROPENCV_DLL	__declspec(dllimport)
#endif
#else // windows
#define VIDEOVALIDATORFOROPENCV_DLL
#define BOOL bool
#define TRUE 1
#define FALSE 0
#endif // windows

//////////////////////////////////////////////////////////////////////////
// Video Information (Reference)
// NASAKSN-NorthernLights.mpg
// nFrames: 1805
// Sampled Frames: 100,500,1000,1300,1700,1800
// Saved: NASAKSN-NorthernLights.txt.gz
//////////////////////////////////////////////////////////////////////////

// #define VIDEO_TESTER_FOR_OPENCV_ERROR_1		137		//Error while loading a video file
// #define VIDEO_TESTER_FOR_OPENCV_ERROR_2		287		//error for different size of frames
// #define VIDEO_TESTER_FOR_OPENCV_ERROR_3		1387	//error for different image of frames
// #define VIDEO_TESTER_FOR_OPENCV_ERROR_4		6387	//error for different count of frames

#define VIDEO_VALIDATOR_FOR_OPENCV_ERROR_1_MSG	"Failure(Error while loading a video file)."
#define VIDEO_VALIDATOR_FOR_OPENCV_ERROR_2_MSG "Failure(Different Video Resolution to the reference video)."
#define VIDEO_VALIDATOR_FOR_OPENCV_ERROR_3_MSG "Failure(Different Image of Sampled Frames to the reference video)."
#define VIDEO_VALIDATOR_FOR_OPENCV_ERROR_4_MSG "Failure(Different Total Number of Frames to the reference video)."


class VIDEOVALIDATORFOROPENCV_DLL VideoValidatorForOpenCV {
public:
	VideoValidatorForOpenCV();
	~VideoValidatorForOpenCV();
	
	bool			runTest(std::string _commonPath,std::string _fileName,std::string& _returnMsg);	//For comparing reference data with the target video file
	bool			generateSrc(std::string _commonPath,std::string _fileName);	//For generating reference data		

private:	
	bool			runCompare(std::string _commonPath,std::string _fileName,std::string& _returnMsg);
	bool			loadVideo(std::string _fullFileNameVideo,long& _nFrame);	
	long			extractFrames(std::vector<Mat>& _srcFrm);

private:	
	VideoCapture		mVideoFile;	
	long				mSampleCount;
	long				mFrameSrcFullCount;
	std::vector<long>	mFrameSampledIdx;	
	//std::string			mFilenameVideoSource;
	std::string			mFilenamePrefixCapturedSource;
	std::string			mFilenamePrefixCapturedTarget;
};
#endif	//_VIDEOVALIDATORFOROPENCV_H__
