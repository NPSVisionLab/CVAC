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
#define DLL_EXPORT

#include "VideoValidatorForOpenCV.h"

VideoValidatorForOpenCV::VideoValidatorForOpenCV()
{
	mFrameSrcFullCount = 1805;
	mSampleCount = 6;
	long _idx[]={100,500,1000,1300,1700,1800};
	for(int i=0;i<mSampleCount;i++)
		mFrameSampledIdx.push_back(_idx[i]);

	//mFilenameVideoSource = "NASAKSN-NorthernLights.mpg";
	mFilenamePrefixCapturedSource = "source_frame_";
	mFilenamePrefixCapturedTarget = "target_frame_";
}


VideoValidatorForOpenCV::~VideoValidatorForOpenCV()
{
	if(mVideoFile.isOpened())
		mVideoFile.release();

	mFrameSampledIdx.clear();
}

bool VideoValidatorForOpenCV::loadVideo(std::string _fullFileNameVideo,long& _nFrame)
{
	if(mVideoFile.isOpened())
		mVideoFile.release();

	mVideoFile.open(_fullFileNameVideo);

	if(mVideoFile.isOpened())
	{
		cout << _fullFileNameVideo << " is loaded." << endl;	fflush(stdout);

		_nFrame = (long)mVideoFile.get(CV_CAP_PROP_FRAME_COUNT);	//row	
		return true;
	}
	else
	{
		_nFrame = -1;
		cout << "Error - not proper file: " << _fullFileNameVideo << endl;	fflush(stdout);
		return false;		
	}
}

bool VideoValidatorForOpenCV::runTest(std::string _commonPath,std::string _fileName,std::string& _returnMsg)
{
	if(!runCompare(_commonPath,_fileName,_returnMsg))
	{
		cout << "Your system may show wrong results for running video-processing." <<std::endl;	fflush(stdout);
		cout << "On your system, ffmpeg may not be used. Please, check use of ffmpeg." <<std::endl;	fflush(stdout);
		return false;
	}
	else
	{
		cout << "Your system can show valid results for running video-processing." <<std::endl;	fflush(stdout);
		return true;
	}
}

bool VideoValidatorForOpenCV::runCompare(std::string _commonPath,std::string _fileName,std::string& _returnMsg)
{	
	_returnMsg = "SUCCESS - Your system can show valid results for running video-processing.";

	//mFilenameVideoSource = "BOR10_002.mpg";
	std::string _fullVideoFileName = _commonPath + "/" + _fileName;			
	long _estNumberFrames;
	if(loadVideo(_fullVideoFileName,_estNumberFrames))
	{
		long _idxCurrentCnt = 0;
		long _idxCurrent = mFrameSampledIdx[_idxCurrentCnt];
		if(mVideoFile.isOpened())
		{
			Mat srcMatFrame;		
			Mat targetMatFrame;
			long _cnt = 0;		
			bool _repeatFlag = true;
			do{
				_repeatFlag = mVideoFile.read(targetMatFrame);
				if(_repeatFlag)
				{
					_cnt++;
					if(_cnt == _idxCurrent)
					{			
						std::ostringstream _strStrmSource;
						_strStrmSource << _commonPath << "/" << mFilenamePrefixCapturedSource << _idxCurrent << ".bmp";
						srcMatFrame = imread(_strStrmSource.str());

						if((srcMatFrame.rows != targetMatFrame.rows) || (srcMatFrame.cols != targetMatFrame.cols) || (srcMatFrame.type() != targetMatFrame.type()))
						{
							cout << "Frame: " << _idxCurrent << " is different with the source." << std::endl; fflush(stdout);

							std::ostringstream _strStrmTarget;
							_strStrmTarget << _commonPath << "/" << mFilenamePrefixCapturedTarget << _idxCurrent << ".bmp";
							imwrite(_strStrmTarget.str(),targetMatFrame);

							cout << _strStrmTarget.str() << " is generated." << std::endl;	fflush(stdout);
							_returnMsg = VIDEO_VALIDATOR_FOR_OPENCV_ERROR_2_MSG;
							return false;
						}						

						Mat _diffMatFrame;
						Mat _srcGrey,_targetGrey;
						cvtColor(srcMatFrame,_srcGrey,CV_BGR2GRAY);
						cvtColor(targetMatFrame,_targetGrey,CV_BGR2GRAY);
						compare(_srcGrey,_targetGrey,_diffMatFrame,cv::CMP_NE);
						int _nDiff = countNonZero(_diffMatFrame);
						if(_nDiff>0)
						{
							cout << "Frame: " << _idxCurrent << " is different with the source." << std::endl; fflush(stdout);

							std::ostringstream _strStrmTarget;
							_strStrmTarget << _commonPath << "/" << mFilenamePrefixCapturedTarget << _idxCurrent << ".jpg";
							imwrite(_strStrmTarget.str(),targetMatFrame);

							cout << _strStrmTarget.str() << " is generated." << std::endl;	fflush(stdout);
							_returnMsg = VIDEO_VALIDATOR_FOR_OPENCV_ERROR_3_MSG;
							return false;
						}
						else
						{
							cout << "Frame: " << _idxCurrent << " is the same with the source." << std::endl; fflush(stdout);
						}									

						_idxCurrentCnt++;
						if(_idxCurrentCnt<mFrameSampledIdx.size())
							_idxCurrent = mFrameSampledIdx[_idxCurrentCnt];
						else
							_idxCurrent = -1;
					}
				}

			}while(_repeatFlag);			

			if(_cnt == mFrameSrcFullCount)
			{
				cout << "Comparison of the total number of frames is passed (nFrames=" << _cnt << ")." << std::endl;	fflush(stdout);
				return true;
			}
			else
			{
				cout << "Total number of frames are different: Source = " << mFrameSrcFullCount << " ,Target = " << _cnt << "." << std::endl; fflush(stdout);
				_returnMsg = VIDEO_VALIDATOR_FOR_OPENCV_ERROR_4_MSG;
				return false;
			}			
		}	
		else
		{
			_returnMsg = VIDEO_VALIDATOR_FOR_OPENCV_ERROR_1_MSG;
			return false;
		}
	}
	else
	{
		_returnMsg = VIDEO_VALIDATOR_FOR_OPENCV_ERROR_1_MSG;
		return false;
	}
}


bool VideoValidatorForOpenCV::generateSrc(std::string _commonPath,std::string _fileName)
{
	std::string _fullVideoFileName = _commonPath + "/" + _fileName;			
	long _estNumberFrames;
	if(loadVideo(_fullVideoFileName,_estNumberFrames))
	{
		long _idxCurrentCnt = 0;
		long _idxCurrent = mFrameSampledIdx[_idxCurrentCnt];
		if(mVideoFile.isOpened())
		{
			Mat srcMatFrame;		
			long _cnt = 0;		
			bool _repeatFlag = true;
			do{
				_repeatFlag = mVideoFile.read(srcMatFrame);
				if(_repeatFlag)
				{
					_cnt++;
					if(_cnt == _idxCurrent)
					{			
						std::ostringstream _strStrm;
						_strStrm << _commonPath << "/" << mFilenamePrefixCapturedSource << _idxCurrent << ".bmp";
						if(imwrite(_strStrm.str(),srcMatFrame))
						{
							cout << _strStrm.str() << " is generated." << std::endl;	fflush(stdout);
						}
						else
						{
							cout << _strStrm.str() << " is failed to be saved as an image." << std::endl;	fflush(stdout);
							return false;
						}					

						_idxCurrentCnt++;
						if(_idxCurrentCnt<mFrameSampledIdx.size())
							_idxCurrent = mFrameSampledIdx[_idxCurrentCnt];
						else
							_idxCurrent = -1;
					}
				}

			}while(_repeatFlag);

			cout<< "The number of frames of the source video is " << _cnt << "."<< std::endl;	fflush(stdout); 
			return true;
		}			
	}
	else
		return false;
}
