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

#include "SBDCV.h"
#include <util/ServiceMan.h>

SBDCV::SBDCV()
{
	mFeatureDim = 6;	//Intensity, EdgeChangeRatio, HSV_Correlation, HSV_Bhattacharyya, RGB_Correlation //RGB_Bhattacharyya
	mFeature_Boundary.create(0,mFeatureDim,CV_32F);
	mFeature_NotBoundary.create(0,mFeatureDim,CV_32F);
	mMarginForLocalMax = 3;

	//////////////////////////////////////////////////////////////////////////
	// SVM Parameter 
	mSVM_Param.gamma = 10;
	//mSVM_Param.kernel_type = CvSVM::LINEAR;
	fSVM = false;
}

SBDCV::~SBDCV()
{
	if(mVideoFile.isOpened())
		mVideoFile.release();
}

bool SBDCV::loadVideo(string _fullfileNameVideo,long& _nFrame)
{
	if(mVideoFile.isOpened())
		mVideoFile.release();

	mVideoFile.open(_fullfileNameVideo);

	if(mVideoFile.isOpened())
	{
		cout << "Video: " << _fullfileNameVideo << " is loaded." << endl;	fflush(stdout);
		
		_nFrame = (long)mVideoFile.get(CV_CAP_PROP_FRAME_COUNT);	//approximation, not Accurate

		if(_nFrame < 3)
		{
			cout << "Error - Not enough the number of frames" << endl;	 fflush(stdout);
			return false;
		}
		else
		{
			cout << _nFrame << " frames will be processed, approximately." << endl;
			return true;		
		}		
	}
	else
	{
		_nFrame = -1;
		cout << "Error - not proper file: " << _fullfileNameVideo << endl;	fflush(stdout);
		return false;		
	}
}

bool SBDCV::loadFramelist(string _fullfileNameFrameList,std::vector<long>& _outframeList)
{
	_outframeList.clear();

	ifstream _fileLog(_fullfileNameFrameList.c_str());
	if(!_fileLog.is_open())
	{
		cout << "Error - no frameList file: " << _fullfileNameFrameList <<endl;	fflush(stdout);
		return false;
	}

	char			_buf[255];
	string			_bufString;
	istringstream	_bufStream;	

	do
	{
		_fileLog.getline(_buf, 255);
		_bufStream.clear();	
		_bufStream.str(_buf);
		_bufStream >> _bufString;

		if (_bufString == "#")
			continue;
		else if(_bufString == "\0")	
			continue;
		
		_outframeList.push_back(atoi(_bufString.c_str()));		

	} while (!_fileLog.eof());
	_fileLog.close();

	cout << "FrameList :" << _fullfileNameFrameList << " is loaded." << endl;	fflush(stdout);
	return true;
}



bool SBDCV::loadTrainlist(string _commonPath,string _trainListFilename,std::vector<string>& _outListVideo,std::vector<string>& _outListFrame)
{
	_outListVideo.clear();
	_outListFrame.clear();

	string _fullFileNameLog = _commonPath+ "/" + _trainListFilename;
	ifstream _fileLog(_fullFileNameLog.c_str());
	if(!_fileLog.is_open())
	{
		cout << "Error - no trainList file: " << _fullFileNameLog <<endl;	fflush(stdout);
		return false;
	}
	else
		cout << "TrainList: " << _fullFileNameLog << " is loaded." << std::endl;

	char			_buf[255];
	string			_bufString;
	istringstream	_bufStream;
	string			_fullFileNameVideo;
	string			_fullFileNameFrameList;
	
	do
	{
		_fileLog.getline(_buf, 255);
		_bufStream.clear();	
		_bufStream.str(_buf);
		_bufStream >> _bufString;

		if (_bufString == "#")
			continue;
		else if(_bufString == "\0")	
			continue;

		// read the name of video file
		_fullFileNameVideo = _commonPath + "/" + _bufString;
		
		// read the name of frameList file
		_bufStream >> _bufString;
		_fullFileNameFrameList = _commonPath + "/" + _bufString;	

		_outListVideo.push_back(_fullFileNameVideo);
		_outListFrame.push_back(_fullFileNameFrameList);

	} while (!_fileLog.eof());
	_fileLog.close();	

	if(_outListVideo.size()<1)
	{
		cout << "Error - no video file or frameList file " << endl;	fflush(stdout);
		return false;
	}

	return true;
}

bool SBDCV::extractFeature(Mat& _outFeatureAll,std::vector<float>& _outFeatureNorm)
{
	_outFeatureAll.create(0,mFeatureDim,CV_32FC1);
	Mat _featureSingle;	_featureSingle.create(1,mFeatureDim,CV_32FC1);	

	Mat srcMatCur,srcMatNext;
	mVideoFile.read(srcMatNext);
	
	long _cntTotalFrame = 0;	
	std::vector<float> _vFeature;
	_outFeatureNorm.clear();
	
	while(true)
	{
		srcMatCur = srcMatNext.clone();
		if(!mVideoFile.read(srcMatNext))
			break;
		else
		{
			++_cntTotalFrame;


			/* 
			// DEBUG_lekomin
			if(_cntTotalFrame==44)
				int xx = 100;
			//*/

			_outFeatureNorm.push_back(computeFeature(srcMatCur,srcMatNext,_vFeature));	

			for(int k=0;k<mFeatureDim;k++)
				_featureSingle.at<float>(0,k) = _vFeature[k];

			_outFeatureAll.push_back(_featureSingle);
			cout<< "The frame " << _cntTotalFrame << " is processed" << '\xd';	fflush(stdout);			
		}		
	}

	return true;
}

bool SBDCV::separateFeature(Mat& _inFeature,std::vector<float>& _inFeatureNorm,std::vector<long>& _inFrame,Mat& _outFeatureBoundary,Mat& _outFeatureNotBoundary)
{
	if((_inFeature.rows < 1) || (_inFeature.rows != _inFeatureNorm.size()))
	{
		std::cout << "Error - empty feature set or frame list" << std::endl;
		return false;
	}

	_outFeatureBoundary.create(0,mFeatureDim,CV_32FC1);
	_outFeatureNotBoundary.create(0,mFeatureDim,CV_32FC1);	
	Mat _matBnd;	_matBnd.create(1,mFeatureDim,CV_32FC1);

	//////////////////////////////////////////////////////////////////////////
	// Separate the feature to Boundary and Not-Boundary
	int _cntTotalFrame = _inFeature.rows;
	std::vector<long>::const_iterator _it;
	int k;
	int marginSize = mMarginForLocalMax;
	int marginMin,marginMax;
	float _valueMax;
	int	_valudeMaxIdx;
	for(_it=_inFrame.begin();_it!=_inFrame.end();++_it)
	{
		k = (*_it)-1;

		if(k>=_cntTotalFrame)
			break;

		marginMin = k - marginSize;
		marginMax = k + marginSize;
		marginMin = (marginMin>=0)?marginMin:0;
		marginMax = (marginMax<_cntTotalFrame)?marginMax:(_cntTotalFrame-1);

		_valueMax = 0.0;
		_valudeMaxIdx = marginMin;
		for(k=marginMin;k<=marginMax;k++)
		{
			if(_inFeatureNorm[k]>_valueMax)
			{
				_valueMax = _inFeatureNorm[k];
				_valudeMaxIdx = k;
			}	
		}
		
		// -1.0 : Boundary
		// -2.0 : Extra (Undecidable)
		_inFeatureNorm[_valudeMaxIdx] = -1.0;
		for(k=marginMin;k<=marginMax;k++)
		{
			if(_inFeatureNorm[k]>=0.0)
				_inFeatureNorm[k] = -2.0;
		}
	}

	std::vector<float>::const_iterator _itNorm;
	int _cnt;
	for(_itNorm=_inFeatureNorm.begin(),_cnt=0;_itNorm!=_inFeatureNorm.end();++_itNorm,_cnt++)
	{
		for(k=0;k<mFeatureDim;k++)
			_matBnd.at<float>(0,k) = _inFeature.at<float>(_cnt,k);

		if((*_itNorm)>=0.0)
			_outFeatureNotBoundary.push_back(_matBnd);
		else if((*_itNorm)==-1.0)
			_outFeatureBoundary.push_back(_matBnd);
	}
	
	long _nFrameActual = _cntTotalFrame+1;
	cout << _nFrameActual << " frames were processed, actually." << endl;

	return true;
}

bool SBDCV::setFeature(string _fullfileNameVideo,std::vector<long>& _inFrameList,Mat& _outFeatureBoundary,Mat& _outFeatureNotBoundary)
{
	//////////////////////////////////////////////////////////////////////////
	// Check Video File
	long _nFrameApproximate;
	if(!loadVideo(_fullfileNameVideo,_nFrameApproximate))
		return false;		

	//////////////////////////////////////////////////////////////////////////
	// Check FrameList File
	if(_inFrameList.size()<1)
	{
		cout << "Error - no frames in the frameList file" << endl;	fflush(stdout);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Extract Features
	Mat _featureAll;
	std::vector<float> _featureNorm;
	if(extractFeature(_featureAll,_featureNorm))
	{
		/*
		//	DEBUG_lekomin
// 		ofstream out;
// 		std::string _featureFileName = _fullfileNameVideo.substr(0,_fullfileNameVideo.rfind(".")) + "_featureFull.txt";				
// 		out.open(_featureFileName.c_str());
// 		for(int r=0; r<_featureAll.rows;r++)
// 		{
// 			for(int c=0;c<_featureAll.cols;c++)
// 			{
// 				out << _featureAll.at<float>(r,c) << '\t';
// 			}				
// 			out << std::endl;
// 		}
// 		out.close();
		std::string _featureFileName = _fullfileNameVideo.substr(0,_fullfileNameVideo.rfind(".")) + "_featureFull.txt";				
		FileStorage _fs(_featureFileName,FileStorage::WRITE);
		_fs << "FeatureFull" << _featureAll;		
		_fs.release();		
		//*/


		if(separateFeature(_featureAll,_featureNorm,_inFrameList,_outFeatureBoundary,_outFeatureNotBoundary))
		{
			/*
			//	DEBUG_lekomin			
			std::string _featureFileName2 = _fullfileNameVideo.substr(0,_fullfileNameVideo.rfind(".")) + "_feature.txt";				
			FileStorage _fs2(_featureFileName2,FileStorage::WRITE);
			_fs2 << "Feature_Boundary" << _outFeatureBoundary;
			_fs2 << "Feature_NotBoundary" << _outFeatureNotBoundary;
			_fs2.release();	
			//*/
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool SBDCV::train_extractFeature(string _path,string _videoFileName,std::vector<long>& _vFrameList,bool _doInitialize)
{
	//////////////////////////////////////////////////////////////////////////
	// Generate new file name
	string _fullVideoFileName = _path+ "/" + _videoFileName;	

	//////////////////////////////////////////////////////////////////////////
	// Extract Features
	if(_doInitialize)	//For selecting whether info. would be stacked or not
	{
		mFeature_Boundary.create(0,mFeatureDim,CV_32F);
		mFeature_NotBoundary.create(0,mFeatureDim,CV_32F);
	}

	Mat _featureBoundary,_featureNotBoundary;
	if(setFeature(_fullVideoFileName,_vFrameList,_featureBoundary,_featureNotBoundary))
	{
		mFeature_Boundary.push_back(_featureBoundary);
		mFeature_NotBoundary.push_back(_featureNotBoundary);
		return true;
	}
	else
	{
		return false;		
	}	
}

bool SBDCV::setFeatureFromTrainList(string _commonPath,string _trainListFileName,bool _doInitialize)
{
	std::vector<string> _listVideoName;
	std::vector<string> _listFrameLogName;

	//////////////////////////////////////////////////////////////////////////
	// Parse TrainList file
	if(!loadTrainlist(_commonPath,_trainListFileName,_listVideoName,_listFrameLogName))
	{
		cout << "Error - read the file trainList" << endl;	fflush(stdout);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Extract Features
	if(_doInitialize)
	{
		mFeature_Boundary.create(0,mFeatureDim,CV_32F);
		mFeature_NotBoundary.create(0,mFeatureDim,CV_32F);
	}
	
	std::vector<long> _frameList;
	Mat _featureBoundary,_featureNotBoundary;
	std::vector<string>::const_iterator itr_video;
	std::vector<string>::const_iterator itr_frameLog;
	for(itr_video=_listVideoName.begin(),itr_frameLog=_listFrameLogName.begin(); itr_video != _listVideoName.end(); ++itr_video, ++itr_frameLog)
	{ 		
		if(!loadFramelist((*itr_frameLog),_frameList))
			return false;

		if(!setFeature((*itr_video),_frameList,_featureBoundary,_featureNotBoundary))			
			return false;
		else
		{
			mFeature_Boundary.push_back(_featureBoundary);
			mFeature_NotBoundary.push_back(_featureNotBoundary);
		}		
	}		 	

	return true;
}

bool SBDCV::train_trainSVM()
{
	return trainSVM();
}

bool SBDCV::trainSVM()
{
	//////////////////////////////////////////////////////////////////////////
	// Check exception
	if(mFeature_Boundary.rows==0 || mFeature_NotBoundary.rows==0)
	{
		cout << "Error - no feature data " << endl;	fflush(stdout);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Initialize Train Data
	Mat _svmFeature(0,mFeature_Boundary.cols,mFeature_Boundary.type());
	_svmFeature.push_back(mFeature_Boundary);
	_svmFeature.push_back(mFeature_NotBoundary);

	Mat	_svmClass(0,1,CV_32FC1);	
	Mat _matClassBoundary = Mat::ones(mFeature_Boundary.rows,1,CV_32FC1);
	Mat _matClassNotBoundary =  Mat::zeros(mFeature_NotBoundary.rows,1,CV_32FC1);
	_matClassNotBoundary.setTo(-1.0);

	_svmClass.push_back(_matClassBoundary);
	_svmClass.push_back(_matClassNotBoundary);

	//////////////////////////////////////////////////////////////////////////
	// SVM
	mSVM.clear();
	cout << "Training Process is started. According to data size, it may take long time ...."	<< endl;	fflush(stdout);	
	if(mSVM.train(_svmFeature, _svmClass, Mat(), Mat(), mSVM_Param))
	{
		cout << "Training Process is done"	<< endl;	fflush(stdout);
		fSVM = true;
		return true;
	}
	else
	{
		cout << "Error - SVM Training Process "	<< endl;	fflush(stdout);
		fSVM = false;
		return false;
	}		
}

bool SBDCV::train_saveSVM(string _commonPath,string _filename)
{
	return saveSVM(_commonPath,_filename);
}

bool SBDCV::saveSVM(string _commonPath,string _filename)
{
	string _fullname = _commonPath + "/" + _filename;
	if(fSVM)
	{
		mSVM.save(_fullname.c_str());
		cout << "SVM result is saved: " << _fullname << endl;	fflush(stdout);
		return true;
	}
	else
	{
		cout << "Error - No SVM result to be saved " << endl;	fflush(stdout);
		return false;
	}
}

bool SBDCV::test_loadSVM(string _commonPath,string _SVMfilename)
{
	//restore after fixing osx problem
	cout << "SBD checking file parms: \n";
	cout << "SBD  video  file: " << _commonPath.c_str() << "/" << _SVMfilename.c_str() << "\n";

	return loadSVM(_commonPath,_SVMfilename);
}

bool SBDCV::loadSVM(string _commonPath, string _SVMfilename)
{	
	if(fSVM)
		mSVM.clear();

	fSVM = false;
	string _fullname = _commonPath + "/" + _SVMfilename;
  cout << "Support Vector Machine loading from : " << _fullname << endl;

/*	//restore after fixing osx problem
	FileStorage _fs;
	_fs.open(_fullname,FileStorage::READ);

	if (!_fs.isOpened())
  {
    cout << "Error-loadSVM: Cannot detect without a Support Vector Machine from: " << 
         _commonPath << "\n" << _SVMfilename << endl;
	  return false;
	}
	_fs.release();
*/
	mSVM.load(_fullname.c_str());
	fSVM = true;
	cout << "SVM loaded from: " << _fullname << endl;
	return true;
}

bool SBDCV::train_saveFeature(string _commonPath,string _filename)
{
	return saveFeature(_commonPath,_filename);
}

bool SBDCV::saveFeature(string _commonPath,string _filename)
{
	string _saveName = _commonPath + "/" + _filename;
	FileStorage _fs(_saveName,FileStorage::WRITE);
	_fs << "Feature_Boundary" << mFeature_Boundary;
	_fs << "Feature_NotBoundary" << mFeature_NotBoundary;
	_fs.release();

	cout << "Feature data is saved: " << _saveName << endl;	fflush(stdout);

	return true;
}

bool SBDCV::loadFeature(string _commonPath,string _filename,bool _flagClear)
{
	if(_flagClear)
	{
		mFeature_Boundary.create(0,mFeatureDim,CV_32F);
		mFeature_NotBoundary.create(0,mFeatureDim,CV_32F);
	}

	string _readName = _commonPath + "/" + _filename;
	FileStorage _fs;
	_fs.open(_readName,FileStorage::READ);

	if (!_fs.isOpened())
	{
		cout << "Error - fail to open : " << _readName << endl;	fflush(stdout);
		return false;
	}
	else
	{
		cout << "Succeeded to open : " << _readName << endl;	fflush(stdout);

		Mat _matbd,_matnbd;
		_fs["Feature_Boundary"] >> _matbd;	//mFeature_Boundary;
		_fs["Feature_NotBoundary"] >> _matnbd;	//mFeature_NotBoundary;
		_fs.release();

		mFeature_Boundary.push_back(_matbd);
		mFeature_NotBoundary.push_back(_matnbd);

		return true;
	}
}

bool SBDCV::test_run(string _fullfileNameVideo,std::vector<long>& _outFrameList,bool _flagTxtSave,std::string& _outFrameFileName,bool _flagImgSave)            
{
	//restore after fixing osx problem
	cout << "SBD checking file parms: \n";
	cout << "SBD  video  file: " << _fullfileNameVideo.c_str() << "\n";

	std::string _path;
	std::string _videoFileName;
	int _idx = _fullfileNameVideo.rfind("/");
	int _lenght = _fullfileNameVideo.length();
	if(_idx<=0)
	{
		_path = "";
		_videoFileName = _fullfileNameVideo;
	}
	else
	{
		_path = _fullfileNameVideo.substr(0,_idx);
		_videoFileName = _fullfileNameVideo.substr(_idx+1,_lenght-_idx);
	}

	_outFrameFileName = _videoFileName.substr(0,_videoFileName.rfind('.')) + "_result.txt";	

	return runTest(_path,_videoFileName,_outFrameFileName,_outFrameList,_flagTxtSave,_flagImgSave);
}

bool SBDCV::runTest(string _commonPath,string _fileNameVideo,string& _fileNameResult,std::vector<long>& _outFrameList,bool _flagTxtSave,bool _flagImgSave)
{
	_outFrameList.clear();
	//////////////////////////////////////////////////////////////////////////
	// Check Availability of SVM 
	if(!fSVM)
	{
    cout << "Error-runTest: Cannot detect without an initialized Support Vector Machine." << endl;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Load Video File
	string _fullfileNameVideo = _commonPath + "/" + _fileNameVideo;
	long _nFrame;
	if(!loadVideo(_fullfileNameVideo,_nFrame))
		return false;

	//////////////////////////////////////////////////////////////////////////
	// Compute features & Test them
	Mat _matFeature;	_matFeature.create(1,mFeatureDim,CV_32FC1);
	Mat srcMatCur,srcMatNext;
	mVideoFile >> srcMatNext;	
	long _cnt = 0;	
	std::vector<float> _vFeature;
	while(true)
	{	
		srcMatCur = srcMatNext.clone();
		if(!mVideoFile.read(srcMatNext))
			break;
		else
		{
			++_cnt;

			computeFeature(srcMatCur,srcMatNext,_vFeature);		

			for(int k=0;k<mFeatureDim;k++)
				_matFeature.at<float>(0,k) = _vFeature[k];
			
			if(mSVM.predict(_matFeature) == 1.0)
			{
				_outFrameList.push_back(_cnt);

				if(_flagImgSave)
				{
					stringstream _strmInput;
					_strmInput << _commonPath + "/" + _fileNameVideo << "_" << _cnt << "_input" << ".jpg";
					imwrite(_strmInput.str(),srcMatCur);

					stringstream _strmOutput;
					_strmOutput << _commonPath + "/" + _fileNameVideo << "_" << _cnt << "_output" << ".jpg";
					imwrite(_strmOutput.str(),srcMatNext);
				}
			}

			cout<< "The frame " << _cnt << " is processed" << '\xd';	fflush(stdout);
            if (cvac::stopRequested(cvac::getServiceName()))
            {
                cvac::stopCompleted(cvac::getServiceName());
                break;
            }
		}
	}
	cout << std::endl;

	if(_flagTxtSave)
	{
		string _fullfileNameClass = _commonPath + "/" + _fileNameResult;
		ofstream _outClass(_fullfileNameClass.c_str());
		for(unsigned int k=0;k<_outFrameList.size();k++)
			_outClass << _outFrameList[k] << '\n';
		_outClass.close();
		_fileNameResult = _fullfileNameClass;
		cout << "Frame list is saved: " << _fileNameResult << std::endl;
	}

	return true;	
}


float SBDCV::computeFeature(Mat& srcMatCur,Mat& srcMatNext,std::vector<float>& _vFeature)
{
	_vFeature.clear();

	//////////////////////////////////////////////////////////////////////////
	// Convert Sources
	Mat _grayCur,_grayNext;
	cvtColor(srcMatCur,_grayCur,CV_BGR2GRAY);
	cvtColor(srcMatNext,_grayNext,CV_BGR2GRAY);

	Mat _HSVCur,_HSVNext;	
	cvtColor(srcMatCur,_HSVCur,CV_RGB2HSV);
	cvtColor(srcMatNext,_HSVNext,CV_RGB2HSV);

	vector<Mat> _bgrCur,_bgrNext;
	split( srcMatCur, _bgrCur );
	split( srcMatNext, _bgrNext );


	//////////////////////////////////////////////////////////////////////////
	// Extracting Features
	double _sum = 0.0;
	double _valueA,_valueB;		
	computeDifferenceIntensity_Gray(_grayCur,_grayNext,_valueA);		
	_vFeature.push_back((float)_valueA);	_sum+=(_valueA*_valueA);

	computeEdgeChangeRatio_Gray(_grayCur,_grayNext,_valueA);		
	_vFeature.push_back((float)_valueA);	_sum+=(_valueA*_valueA);

	computeDifferenceColorHistogram_HSV(_HSVCur,_HSVNext,_valueA,_valueB);
	_vFeature.push_back((float)_valueA);	_sum+=(_valueA*_valueA);
	//_vFeature.push_back((float)_valueB);	//It's not critical

	computeDifferenceColorHistogram_RGB(_bgrCur,_bgrNext,_valueA,_valueB);	
	_vFeature.push_back((float)_valueA);	_sum+=(_valueA*_valueA);
	//_vFeature.push_back((float)_valueB);	//It's not critical	//There are invaild values. Check it. 

	computeDifferenceColorPixelWise_RGB(_bgrCur,_bgrNext,_valueA);
	_vFeature.push_back((float)_valueA);	_sum+=(_valueA*_valueA);

	computeDCT_Gray(_grayCur,_grayNext,_valueA);
	_vFeature.push_back((float)_valueA);	_sum+=(_valueA*_valueA);

	return (float)sqrt(_sum/(double)mFeatureDim);
}

void SBDCV::computeDCT_Gray(const Mat& _greyCur,const Mat& _greyNext,double& _diffValue)
{
	Mat _grayCur_f,_grayNext_f;
	_greyCur.convertTo(_grayCur_f,CV_32FC1);
	_greyNext.convertTo(_grayNext_f,CV_32FC1);

	Mat _dctCur,_dctNext;
	dct(_grayCur_f,_dctCur);
	dct(_grayNext_f,_dctNext);

	double _cosDist = 0;
	double _sqSumCur = 0;
	double _sqSumNext = 0;
	
	double _a,_b;
	int r,c;
	for(r=0;r<4;r++)
	{
		for(c=0;c<4-r;c++)
		{
			_a = fabs(_dctCur.at<float>(r,c));
			_b = fabs(_dctNext.at<float>(r,c));

			_cosDist += _a*_b;
			_sqSumCur += _a*_a;
			_sqSumNext += _b*_b;
		}
	}

	_cosDist /=sqrt(_sqSumCur*_sqSumNext);
	_cosDist = acos(_cosDist);

	_diffValue = _cosDist / 1.57079632679489661923;
}

void SBDCV::computeDifferenceIntensity_Gray(const Mat& _greyCur,const Mat& _greyNext,double& _featureIntensity)
{
	//matNext = imread("F:/test Mov/new.jpg");	

	Mat _matDiff;	
	cv::absdiff(_greyCur,_greyNext,_matDiff);

	_featureIntensity = cv::sum(_matDiff)[0];
	_featureIntensity /= (double)(255*_matDiff.rows*_matDiff.cols);	//normalize [0,1]
}

void SBDCV::computeEdgeChangeRatio_Gray(const Mat& _greyCur,const Mat& _greyNext,double& _featureEdgeChangeRatio)
{
	//matNext = imread("F:/test Mov/new.jpg");

	int _nRow,_nCol;
	_nRow = _greyCur.rows;
	_nCol = _greyCur.cols;

	Mat _resCur,_resNext;
	blur(_greyCur,_resCur,Size(3,3));
	blur(_greyNext,_resNext,Size(3,3));

	int _sizeKernel = 3;
	int _lowThreshold = 50;
	int _highThreshold = 200;
	Canny(_resCur,_resCur,_lowThreshold,_highThreshold,_sizeKernel);	//_resCur = [0,255]
	Canny(_resNext,_resNext,_lowThreshold,_highThreshold,_sizeKernel);
	int _nCur,_nNext;	_nCur = _nNext = 0;
	for(int r=0;r<_nRow;r++)
	{
		for(int c=0;c<_nCol;c++)
		{
			if(_resCur.at<uchar>(r,c) > uchar(200))	
				_nCur++;

			if(_resNext.at<uchar>(r,c) > uchar(200))	
				_nNext++;
		}
	}
	_nCur = max(_nCur,1);
	_nNext = max(_nNext,1);

	Mat _resInvErodeCur,_resInvErodeNext;
	Mat _elementDilate;
	_elementDilate = getStructuringElement(MORPH_RECT,Size(5,5),Point(2,2));

	bitwise_not(_resCur,_resInvErodeCur);	//cvNot
	erode(_resInvErodeCur,_resInvErodeCur,_elementDilate);	
	bitwise_not(_resNext,_resInvErodeNext);	//cvNot
	erode(_resInvErodeNext,_resInvErodeNext,_elementDilate);	

	bitwise_and(_resCur,_resInvErodeNext,_resCur);
	bitwise_and(_resNext,_resInvErodeCur,_resNext);
	int _nOut,_nIn;	_nOut = _nIn = 0;
	for(int r=0;r<_nRow;r++)
	{
		for(int c=0;c<_nCol;c++)
		{
			if(_resCur.at<uchar>(r,c) > uchar(200))	
				_nOut++;

			if(_resNext.at<uchar>(r,c) > uchar(200))	
				_nIn++;
		}
	}

	double _ecrOut,_ecrIn;
	_ecrOut = (double)_nOut/(double)_nCur;
	_ecrIn = (double)_nIn/(double)_nNext;

	_featureEdgeChangeRatio = max(_ecrOut,_ecrIn);
	_featureEdgeChangeRatio = (_featureEdgeChangeRatio>1.0)?1.0:_featureEdgeChangeRatio;	//[0,1]
}


void SBDCV::computeDifferenceColorHistogram_HSV(const Mat& _HSVCur,const Mat& _HSVNext,double& _featureColorHistogramHSVCorrelation,double& _featureColorHistogramHSVBhattacharyya)
{
	//matNext = imread("F:/test Mov/new.jpg");
	int h_bins = 50; int s_bins = 60;
	int histSize[] = { h_bins, s_bins };

	float h_ranges[] = { 0, 256 };
	float s_ranges[] = { 0, 180 };

	const float* ranges[] = { h_ranges, s_ranges };

	int channels[] = { 0, 1 };


	MatND _histCur,_histNext;
	calcHist( &_HSVCur, 1, channels, Mat(), _histCur, 2, histSize, ranges, true, false );
	//normalize( _histCur, _histCur, 0, 1, NORM_MINMAX, -1, Mat() );

	calcHist( &_HSVNext, 1, channels, Mat(), _histNext, 2, histSize, ranges, true, false );
	//normalize( _histNext, _histNext, 0, 1, NORM_MINMAX, -1, Mat() );

	_featureColorHistogramHSVCorrelation = 1.0 - fabs(compareHist( _histCur, _histNext, CV_COMP_CORREL ));	
	_featureColorHistogramHSVBhattacharyya = compareHist( _histCur, _histNext, CV_COMP_BHATTACHARYYA );
}

void SBDCV::computeDifferenceColorHistogram_RGB(const vector<Mat>& bgrCur,const vector<Mat>& bgrNext,double& _featureColorHistogramRGBCorrelation,double& _featureColorHistogramRGBBhattacharyya)
{
	//matNext = imread("F:/test Mov/new.jpg");

	int histSize = 50;
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	Mat histCurB,histCurG,histCurR;
	Mat histNextB,histNextG,histNextR;

	calcHist( &bgrCur[0], 1, 0, Mat(), histCurB, 1, &histSize, &histRange, true, false );	
	calcHist( &bgrCur[1], 1, 0, Mat(), histCurG, 1, &histSize, &histRange, true, false );
	calcHist( &bgrCur[2], 1, 0, Mat(), histCurR, 1, &histSize, &histRange, true, false );

	//double _vmax,_vmin;
	//minMaxLoc(histCurB,&_vmax,&_vmin);

	//normalize(histCurB, histCurB, 0, 1, NORM_MINMAX, -1, Mat() );
	//normalize(histCurG, histCurG, 0, 1, NORM_MINMAX, -1, Mat() );
	//normalize(histCurR, histCurR, 0, 1, NORM_MINMAX, -1, Mat() );

	//minMaxLoc(histCurB,&_vmax,&_vmin);

	calcHist( &bgrNext[0], 1, 0, Mat(), histNextB, 1, &histSize, &histRange, true, false );
	calcHist( &bgrNext[1], 1, 0, Mat(), histNextG, 1, &histSize, &histRange, true, false );
	calcHist( &bgrNext[2], 1, 0, Mat(), histNextR, 1, &histSize, &histRange, true, false );
	//normalize(histNextB, histNextB, 0, 1, NORM_MINMAX, -1, Mat() );
	//normalize(histNextG, histNextG, 0, 1, NORM_MINMAX, -1, Mat() );
	//normalize(histNextR, histNextR, 0, 1, NORM_MINMAX, -1, Mat() );

	double _bCorrelation = 1.0 - fabs(compareHist( histCurB, histNextB, CV_COMP_CORREL ));	
	double _gCorrelation = 1.0 - fabs(compareHist( histCurG, histNextG, CV_COMP_CORREL ));	
	double _rCorrelation = 1.0 - fabs(compareHist( histCurR, histNextR, CV_COMP_CORREL ));
	_featureColorHistogramRGBCorrelation = (_bCorrelation + _gCorrelation + _rCorrelation)/3.0;

	// 	double _bBhattacharyya = compareHist( histCurB, histNextB, CV_COMP_BHATTACHARYYA );	
	// 	double _gBhattacharyya = compareHist( histCurG, histNextG, CV_COMP_BHATTACHARYYA );	
	// 	double _rBhattacharyya = compareHist( histCurR, histNextR, CV_COMP_BHATTACHARYYA );
	// 	_featureColorHistogramRGBBhattacharyya = (_bBhattacharyya + _gBhattacharyya + _rBhattacharyya)/3.0;

	//float _res(0.0f);
	//float _difR,_difG,_difB;
	//for(int i=1;i<histSize;i++)
	//{
	//	_difR = fabs(histCurR.at<float>(i) - histNextR.at<float>(i));
	//	_difG = fabs(histCurG.at<float>(i) - histNextG.at<float>(i));
	//	_difB = fabs(histCurB.at<float>(i) - histNextB.at<float>(i));
	//	//cvQueryHistValue_1D(hist_red,i)
	//	_res += (_difR + _difG + _difB);
	//}
	//_res /= (float)(histSize*3);
	//_featureColorHistogramRGBBhattacharyya = _res;
	_featureColorHistogramRGBBhattacharyya = 0;
}




void SBDCV::computeDifferenceColorPixelWise_RGB(const vector<Mat>& bgrCur,const vector<Mat>& bgrNext,double& _featureColorRGBPixelWise)
{
	_featureColorRGBPixelWise = 0;

	Mat _diff;
	absdiff(bgrCur[0],bgrNext[0],_diff);	
	_featureColorRGBPixelWise += cv::sum(_diff)[0];

	absdiff(bgrCur[1],bgrNext[1],_diff);	
	_featureColorRGBPixelWise += cv::sum(_diff)[0];

	absdiff(bgrCur[2],bgrNext[2],_diff);	
	_featureColorRGBPixelWise += cv::sum(_diff)[0];

	_featureColorRGBPixelWise /= 3.0;
	_featureColorRGBPixelWise /= (double)(255*_diff.rows*_diff.cols);	//normalize [0,1]
}

bool SBDCV::runTestFromFeature(string _commonPath,string _fileNameVideo,bool _flagTxtSave)
{
	string _fileNameResult = _fileNameVideo.substr(0,_fileNameVideo.rfind('.')) + "_result.txt";
	//////////////////////////////////////////////////////////////////////////
	// Check Availability of SVM 
	if(!fSVM)
	{
		cout << "Error - There is No SVM result " << endl;	fflush(stdout);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Load Video File
	string _fullfileNameVideo = _commonPath + "/" + _fileNameVideo;
	long _nFrame;
	if(!loadVideo(_fullfileNameVideo,_nFrame))
		return false;

	//////////////////////////////////////////////////////////////////////////
	// Compute features & Test them
	Mat _matFeatureTotal;
	string _readName = _fullfileNameVideo.substr(0,_fullfileNameVideo.rfind('.')) + "_featureFull.txt";
	FileStorage _fs;
	_fs.open(_readName,FileStorage::READ);

	if (!_fs.isOpened())
	{
		cout << "Error - fail to open : " << _readName << endl;	fflush(stdout);
		return false;
	}
	else
	{
		cout << "Succeeded to open : " << _readName << endl;	fflush(stdout);

		_fs["FeatureFull"] >> _matFeatureTotal;	//mFeature_Boundary;
		_fs.release();
	}

	Mat _matFeatureTemp;	_matFeatureTemp.create(1,mFeatureDim,CV_32FC1);
	long _cnt = 0;	
	std::vector<long>	_vFrameList;
	std::vector<float> _vFeature;
	for(int r=0;r<_matFeatureTotal.rows;r++)
	{	
		for(int c=0;c<mFeatureDim;c++)
				_matFeatureTemp.at<float>(0,c) = _matFeatureTotal.at<float>(r,c);

		if(mSVM.predict(_matFeatureTemp) == 1.0)
			_vFrameList.push_back(r+1);

		cout<< "The frame " << r+1 << " is processed" << '\xd';	fflush(stdout);
	}
	cout << std::endl;

	if(_flagTxtSave)
	{
		string _fullfileNameClass = _commonPath + "/" + _fileNameResult;
		ofstream _outClass(_fullfileNameClass.c_str());
		for(unsigned int k=0;k<_vFrameList.size();k++)
			_outClass << _vFrameList[k] << '\n';
		_outClass.close();
		_fileNameResult = _fullfileNameClass;
	}	

	return true;
}

//restore after fixing osx problem
void SBDCV::osxTest()
{
	string _xmlName = "data/CTest/svm_Result.xml";
	string _videoName = "data/CTest/ShortTest.mpg";

	cv::FileStorage _fs;
	_fs.open(_xmlName,cv::FileStorage::READ);

	if (!_fs.isOpened())
		cout << "Error - it is not opened: "<< _xmlName <<"\n";
	else
		cout << "Success - it is opened: "<< _xmlName <<"\n";

	_fs.release();


	cv::VideoCapture	mVideoFile;

	if(mVideoFile.isOpened())
		mVideoFile.release();

	mVideoFile.open(_videoName);

	if(mVideoFile.isOpened())
		cout << "Success - it is opened: "<< _videoName <<"\n";
	else
		cout << "Error - it is not opened: "<< _videoName <<"\n";
		
	mVideoFile.release();
}

