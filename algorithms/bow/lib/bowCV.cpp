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

#include "bowCV.h"

bowCV* pLib = NULL;

/*
extern "C"
{
	int bowInitialize(string _detectorName,string _extractorName,string _matcherName,int _nCluster)
	{
		if(pLib)
		{
			//_tprintf(_T("already initialized\n"));	fflush(stdout);
			return false;	//false
		}
		pLib = new bowCV();
		return pLib->initialize(_detectorName,_extractorName,_matcherName,_nCluster);
	}
	int	bowRunTrain(string _filepathTrain,string _filenameTrainList,string _filenameTrainResult)
	{			
		return pLib->runTrain(_filepathTrain,_filenameTrainList,_filenameTrainResult);
	}	

	int	bowLoadTrainResult(string _filepath,string _filename)
	{
		return pLib->readTrainResult(_filepath,_filename);
	}

	int	bowRunTest(string _fullfilename, float _thresholdScore, int& _bestClass, float& _bestScore)
	{
		return pLib->runTest(_fullfilename,_thresholdScore,_bestClass,_bestScore);
	}
}
*/

bowCV::bowCV()
{	
	flagTrain = false;
	flagName = false;
	cntCluster = -1;
	cv::initModule_nonfree();	//it should be for using SIFT or SURF. 	

	filenameVocabulary = "logTrain_Vocabulary.xml.gz";
}

bowCV::~bowCV()
{	
	vFilenameTrain.clear();
	vClassIDTrain.clear();
	vBoundX.clear();	
	vBoundY.clear();
	vBoundWidth.clear();
	vBoundHeight.clear();
}


bool bowCV::train_initialize(const string& _detectorName,const string& _extractorName,const string& _matcherName,int _nCluster)
{
	//_detectorName;	//SURF, SIFT, FAST, STAR, MSER, GFTT, HARRIS
	//_extractorName;	//SURF, SIFT, OpponentSIFT, OpponentSURF
	//_matcherName;	//BruteForce-L1, BruteForce, FlannBased  

	flagName = false;

	fDetector = FeatureDetector::create(_detectorName);
	dExtractor = DescriptorExtractor::create(_extractorName);
	if( fDetector.empty() || dExtractor.empty() )
	{
		cout << "Error - featureDetector or descExtractor was not created" << endl;	fflush(stdout);
		return false;
	}
	else
	{
		nameDetector = _detectorName;
		nameExtractor = _extractorName;
	}

	cntCluster = _nCluster;

	dMatcher = DescriptorMatcher::create(_matcherName);
	if (dMatcher.empty())
	{
		cout << "Error - descMatcher was not created" << endl;	fflush(stdout);
		return false;
	}
	else
		nameMatcher = _matcherName;

	bowExtractor = new BOWImgDescriptorExtractor(dExtractor,dMatcher);

	vFilenameTrain.clear();
	vClassIDTrain.clear();
	vBoundX.clear();	
	vBoundY.clear();
	vBoundWidth.clear();
	vBoundHeight.clear();

	flagName = true;

	return true;
}

bool bowCV::detect_initialize(const string& _filepath,const string& _filename)
{
	return detect_readTrainResult(_filepath,_filename);
}

bool bowCV::detect_setParameter(const string& _detectorName,const string& _extractorName,const string& _matcherName)
{
	flagName = false;

	fDetector = FeatureDetector::create(_detectorName);
	dExtractor = DescriptorExtractor::create(_extractorName);
	if( fDetector.empty() || dExtractor.empty() )
	{
		cout << "Error - featureDetector or descExtractor was not created" << endl;	fflush(stdout);
		return false;
	}	
	else
	{
		nameDetector = _detectorName;
		nameExtractor = _extractorName;
	}

	dMatcher = DescriptorMatcher::create(_matcherName);
	if (dMatcher.empty())
	{
		cout << "Error - descMatcher was not created" << endl;	fflush(stdout);
		return false;
	}
	else
		nameMatcher = _matcherName;

	bowExtractor = new BOWImgDescriptorExtractor(dExtractor,dMatcher);

	vFilenameTrain.clear();
	vClassIDTrain.clear();
	vBoundX.clear();	
	vBoundY.clear();
	vBoundWidth.clear();
	vBoundHeight.clear();

	flagName = true;

	return true;
}

bool bowCV::train_parseTrainList(const string& _filepathTrain,const string& _filenameTrainList)
{
	_fullFilePathList = _filepathTrain + "/" + _filenameTrainList;
	ifstream ifList(_fullFilePathList.c_str());
	if(!ifList.is_open())
	{
		cout << "Error - no file: " << _fullFilePathList <<endl;	fflush(stdout);
		return false;
	}

	cout << "Checking training files..." << '\xd'; fflush(stdout);
	do
	{
		ifList.getline(_buf, 255);
		string line(_buf);
		istringstream iss(line);

		iss >> _tfileName;	
		if (_tfileName == "#")
			continue;	

		_fullFilePathImg = _filepathTrain + "/" + _tfileName;

		string classStr;
		iss >> classStr;
		if(classStr.size()==0)
			continue;	

		int _classID(atoi(classStr.c_str()));

		string boxStr_x;
		iss >> boxStr_x;
		Rect _rect;
		if(boxStr_x.size()==0)
			_rect = Rect(0,0,0,0);	
		else
		{
			string boxStr_y;
			iss >> boxStr_y;
			string boxStr_width;
			iss >> boxStr_width;
			string boxStr_height;
			iss >> boxStr_height;
			_rect = Rect(atoi(boxStr_x.c_str()),
				atoi(boxStr_y.c_str()),
				atoi(boxStr_width.c_str()),
				atoi(boxStr_height.c_str()));
		}


		_img = imread(_fullFilePathImg);
		if(_img.empty())
		{
			ifList.close();
			cout<<"Error - no file: " << _fullFilePathImg << endl;	fflush(stdout);
			return false;
		}	

		train_stackTrainImage(_fullFilePathImg,_classID,_rect.x,_rect.y,_rect.width,_rect.height);

	} while (!ifList.eof());
	ifList.close();	

	cout<< "Total number of images for training: " << vFilenameTrain.size() << endl;	fflush(stdout);

	return true;
}

void bowCV::train_stackTrainImage(const string& _fullpath,const int& _classID)
{
	train_stackTrainImage(_fullpath,_classID,0,0,0,0);
}

void bowCV::train_stackTrainImage(const string& _fullpath,const int& _classID,const int& _x,const int& _y,const int& _width,const int& _height)
{
	vFilenameTrain.push_back(_fullpath);
	vClassIDTrain.push_back(_classID);
	vBoundX.push_back(_x);	
	vBoundY.push_back(_y);	
	vBoundWidth.push_back(_width);	
	vBoundHeight.push_back(_height);	
}


bool bowCV::train_run(const string& _filepathForSavingResult,const string& _filenameForSavingResult, cvac::ServiceManager *sman)
{
	if(!flagName)
	{
		cout << "Need to initialize detector, extractor, matcher, and so on with the function train_initialize()." << endl;	fflush(stdout);
		return false;
	}

	if(vFilenameTrain.size() < 1)
	{
		cout << "There is no training images." << endl;	fflush(stdout);
		return false;
	}
	else
	{
		cout<< "Training is started. " << endl; fflush(stdout);
	}
	//////////////////////////////////////////////////////////////////////////
	// START - Clustering (Most time-consuming step)
	//////////////////////////////////////////////////////////////////////////

	Mat _descriptorRepository;		
	Rect _rect;
	for(int k=0;k<vFilenameTrain.size();k++)
	{
		_fullFilePathImg = vFilenameTrain[k];

		_img = imread(_fullFilePathImg);
		if(_img.empty())
		{			
			cout<<"Error - no file: " << _fullFilePathImg << endl;	fflush(stdout);
			return false;
		}
        if (sman->stopRequested())
        {
            sman->stopCompleted();
            return false;
        }
		
		_rect = Rect(vBoundX[k],vBoundY[k],vBoundWidth[k],vBoundHeight[k]);
		if((_rect.width != 0) && (_rect.height != 0))
			_img = _img(_rect);

// 		imshow("Crop",_img);
// 		waitKey();

		fDetector->detect(_img, _keypoints);
		dExtractor->compute(_img, _keypoints, _descriptors);

		_descriptorRepository.push_back(_descriptors);

		cout<< "Progress of Feature Extraction: " << k+1 << "/" << vFilenameTrain.size() << '\xd';	fflush(stdout);
	}	
	cout << endl;	fflush(stdout);

	cout << "Total number of descriptors: " << _descriptorRepository.rows << endl;	fflush(stdout);

	cout << "Clustering ... It will take a time.." << std::endl;	fflush(stdout);
	BOWKMeansTrainer bowTrainer(cntCluster); 
	bowTrainer.add(_descriptorRepository);	
	mVocabulary = bowTrainer.cluster();	

	if(!train_writeVocabulary(_filepathForSavingResult + "/" + filenameVocabulary,mVocabulary))
		return false;
	//////////////////////////////////////////////////////////////////////////
	// END - Clustering (Most time-consuming step)
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// START - Setup Vocabulary
	//////////////////////////////////////////////////////////////////////////
	cout << "Setting Vocabulary ... " << endl;	fflush(stdout);			
	bowExtractor->setVocabulary(mVocabulary);
	//////////////////////////////////////////////////////////////////////////
	// START - End Vocabulary
	//////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	// START - Train Classifier (SVM)
	//////////////////////////////////////////////////////////////////////////
	cout << "Training Classifier ... " << endl;	fflush(stdout);
	
	Mat trainDescriptors;	trainDescriptors.create(0,bowExtractor->descriptorSize(),bowExtractor->descriptorType());
	Mat trainClass;	trainClass.create(0,1,CV_32FC1);
	int _classID;
	

	for(int k=0;k<vFilenameTrain.size();k++)
	{
		_fullFilePathImg = vFilenameTrain[k];
		_classID = vClassIDTrain[k];
		
		if(_classID==-1)
			continue;
        
        if (sman->stopRequested())
        {
            sman->stopCompleted();
            return false;
        }
		_img = imread(_fullFilePathImg);
		
		_rect = Rect(vBoundX[k],vBoundY[k],vBoundWidth[k],vBoundHeight[k]);
		if((_rect.width != 0) && (_rect.height != 0))
			_img = _img(_rect);
		
		fDetector->detect(_img, _keypoints);
		bowExtractor->compute(_img, _keypoints, _descriptors);
		trainDescriptors.push_back(_descriptors);
		trainClass.push_back(_classID);
	}

	//SVMTrainParamsExt svmParamsExt;
	//CvSVMParams svmParams;
	//CvMat class_wts_cv;
	//setSVMParams( svmParams, class_wts_cv, labels, true );
	//CvParamGrid c_grid, gamma_grid, p_grid, nu_grid, coef_grid, degree_grid;
	//setSVMTrainAutoParams( c_grid, gamma_grid,  p_grid, nu_grid, coef_grid, degree_grid );
	//classifierSVM[class_].train_auto( samples_32f, labels, Mat(), Mat(), svmParams, 10, c_grid, gamma_grid, p_grid, nu_grid, coef_grid, degree_grid );
	classifierSVM.train(trainDescriptors,trainClass);

	

	_fullFilePathList = _filepathForSavingResult + "/" + _filenameForSavingResult;
	ofstream ofile(_fullFilePathList.c_str());
	ofile << "# This file should includes the followings:" << std::endl;
	ofile << "# The name of Detector" << std::endl;
	ofile << "# The name of Extractor" << std::endl;
	ofile << "# The name of Matcher" << std::endl;
	ofile << "# The filename of vocabulary" << std::endl;
	ofile << "# Filenames of svm results" << std::endl;
	ofile << nameDetector << std::endl;
	ofile << nameExtractor << std::endl;
	ofile << nameMatcher << std::endl;
	ofile << filenameVocabulary << std::endl;	
	
	string _svmName = "logTrain_svm.xml.gz";
	ofile << _svmName << endl;
	_fullFilePathList = _filepathForSavingResult + "/" + _svmName;
	classifierSVM.save(_fullFilePathList.c_str());
	
	ofile.close();

	flagTrain = true;

	return true;
}


bool bowCV::detect_run(const string& _fullfilename, int& _bestClass)
{	
	_bestClass = -1;	

	if(!flagName)
	{
		cout << "Need to initialize detector, extractor, and matcher with the function detect_initialize()." << endl;	fflush(stdout);
		return false;
	}

	if(!flagTrain)
	{
		cout << "Error - No training flag found.  Before testing, training is necessary .. " << endl;	fflush(stdout);
		return false;
	}

	_img = imread(_fullfilename);
	if(_img.empty())
	{
		cout << "Error - no file found matching RunSet entry: " << _fullfilename <<endl;	fflush(stdout);
		return false;
	}

	fDetector->detect(_img, _keypoints);
	
	bowExtractor->compute(_img, _keypoints, _descriptors);

	_bestClass = classifierSVM.predict(_descriptors);

	if (-1 == _bestClass) // no class found
		return false;

	else
		return true;
}


bool bowCV::detect_readTrainResult(const string& _filepath,const string& _filename)
{	
	string _fullpath = _filepath + "/" + _filename;
	string _inputString;
	
	
	ifstream infile(_fullpath.c_str());	
	if(!infile.is_open())
	{
		cout << "Error - no file: " << _fullpath <<endl;	fflush(stdout);
		return false;
	}
	
	istringstream iss;
	while(true)
	{
		infile.getline(_buf, 255);	
		iss.clear();	iss.str(_buf);	iss >> _inputString;	
		if (_inputString == "#")
			continue;
		else
			break;
	}	
	string _nameDetector(_inputString);
	
	infile.getline(_buf, 255);	
	iss.clear();	iss.str(_buf);	iss >> _inputString;	
	string _nameExtractor(_inputString);

	infile.getline(_buf, 255);	
	iss.clear();	iss.str(_buf);	iss >> _inputString;
	string _nameMatcher(_inputString);

	if(!detect_setParameter(_nameDetector,_nameExtractor,_nameMatcher))
	{
		cout << "Need to names of detector, extractor, and matcher with the function detect_initialize()" << endl; 
		return false;
	}

	infile.getline(_buf, 255);	
	iss.clear();	iss.str(_buf);	iss >> _inputString;
	_fullpath = _filepath + "/" + _inputString;	

	if(detect_readVocabulary(_fullpath,mVocabulary))
	{
		bowExtractor->setVocabulary(mVocabulary);
	}
	else
		return false;
	
	classifierSVM.clear();
	infile.getline(_buf, 255);	
	iss.clear();	iss.str(_buf);	iss >> _inputString;

	_fullpath = _filepath + "/" + _inputString;		
	classifierSVM.load(_fullpath.c_str());
	infile.close();	

	flagTrain = true;

	return true;
}

bool bowCV::train_writeVocabulary(const string& _filename,const Mat& _vocabulary)
{
	cout << "Saving vocabulary..." << endl;	fflush(stdout);
	FileStorage fs( _filename, FileStorage::WRITE );
	if( fs.isOpened() )
	{
		fs << "vocabulary" << _vocabulary;
		fs.release();
		return true;
	}
	fs.release();
	cout << "Error - in Saving vocabulary...";	fflush(stdout);
	return false;
}

bool bowCV::detect_readVocabulary( const string& _filename, Mat& _vocabulary )
{
	cout << "Reading vocabulary...";	fflush(stdout);
	FileStorage fs( _filename, FileStorage::READ );
	if( fs.isOpened() )
	{
		fs["vocabulary"] >> _vocabulary;
		cout << "done" << endl;
		fs.release();
		return true;
	}
	fs.release();
	cout << "Error - in Reading vocabulary...";	fflush(stdout);
	return false;
}


/*
bool bowCV::runTrainFull(const string& _filepathTrain,const string& _filenameTrainList,const string& _filepathForSavingResult,const string& _filenameForSavingResult)
{
	//////////////////////////////////////////////////////////////////////////
	// START - Clustering (Most time-consuming step)
	//////////////////////////////////////////////////////////////////////////
	Mat _descriptorRepository;	
	_fullFilePathList = _filepathTrain + "/" + _filenameTrainList;
	ifstream ifList(_fullFilePathList.c_str());
	if(!ifList.is_open())
	{
		cout << "Error - no file: " << _fullFilePathList <<endl;	fflush(stdout);
		return false;
	}

	do
	{
		ifList.getline(_buf, 255);
		string line(_buf);
		istringstream iss(line);

		iss >> _tfileName;	
		if (_tfileName == "#")
			continue;	

		_fullFilePathImg = _filepathTrain + "/" + _tfileName;

		string classStr;	//garbage
		iss >> classStr;	//garbage	

		_img = imread(_fullFilePathImg);
		if(_img.empty())
		{
			ifList.close();
			cout<<"Error - no file: " << _fullFilePathImg << endl;	fflush(stdout);
			return false;
		}
		fDetector->detect(_img, _keypoints);
		dExtractor->compute(_img, _keypoints, _descriptors);

		_descriptorRepository.push_back(_descriptors);		

	} while (!ifList.eof());
	ifList.close();	
	cout << "Total number of descriptors: " << _descriptorRepository.rows << endl;	fflush(stdout);

	cout << "Clustering .. " << std::endl;	fflush(stdout);
	BOWKMeansTrainer bowTrainer(cntCluster); 
	bowTrainer.add(_descriptorRepository);	
	mVocabulary = bowTrainer.cluster();	

	if(!train_writeVocabulary(_filepathForSavingResult + "/" + filenameVocabulary,mVocabulary))
		return false;
	//////////////////////////////////////////////////////////////////////////
	// END - Clustering (Most time-consuming step)
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// START - Setup Vocabulary
	//////////////////////////////////////////////////////////////////////////
	cout << "Setting Vocabulary .." << endl;	fflush(stdout);			
	bowExtractor->setVocabulary(mVocabulary);
	//////////////////////////////////////////////////////////////////////////
	// START - End Vocabulary
	//////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	// START - Train Classifier (SVM)
	//////////////////////////////////////////////////////////////////////////
	cout << "Training Classifier .. " << endl;	fflush(stdout);
	map<string,Mat> trainingDescriptorsClass;	trainingDescriptorsClass.clear();
	ifList.open(_fullFilePathList.c_str());	
	ifList.seekg(0,ios::beg);
	if(!ifList.is_open())
	{
		cout << "Error - no file: " << _fullFilePathList <<endl;	fflush(stdout);
		return false;
	}

	do
	{
		ifList.getline(_buf, 255);
		string line(_buf);
		istringstream iss(line);		

		iss >> _tfileName;	
		if (_tfileName == "#")
			continue;	

		_fullFilePathImg = _filepathTrain + "/" + _tfileName;

		string classStr;
		iss >> classStr;

		//cout<<_tfileName<<'\t'<<classStr<<endl;	fflush(stdout);

		if(!classStr.compare("-1"))
			continue;

		string classID(classStr.c_str());

		_img = imread(_fullFilePathImg);
		if(_img.empty())
		{
			ifList.close();
			cout<<"Error - no file: " << _fullFilePathImg << endl;	fflush(stdout);
			return false;
		}

		fDetector->detect(_img, _keypoints);
		bowExtractor->compute(_img, _keypoints, _descriptors);

		if(trainingDescriptorsClass.count(classID) == 0)//not yet created...
		{				
			trainingDescriptorsClass[classID].create(0,_descriptors.cols,_descriptors.type());
		}
		trainingDescriptorsClass[classID].push_back(_descriptors);		

	} while (!ifList.eof());
	ifList.close();	

	// Train 1-vs-all SVMs	
	for (map<string,Mat>::iterator it = trainingDescriptorsClass.begin(); it != trainingDescriptorsClass.end(); ++it) 
	{
		string class_ = (*it).first;
		cout << "Training class: " << class_ << ".." << endl;	fflush(stdout);

		Mat samples(0,_descriptors.cols,_descriptors.type());
		Mat labels(0,1,CV_32FC1);

		//copy class samples and label
		samples.push_back(trainingDescriptorsClass[class_]);
		Mat class_label = Mat::ones(trainingDescriptorsClass[class_].rows, 1, CV_32FC1);
		labels.push_back(class_label);

		//copy rest samples and label
		for (map<string,Mat>::iterator it1 = trainingDescriptorsClass.begin(); it1 != trainingDescriptorsClass.end(); ++it1)
		{
			string not_class_ = (*it1).first;
			if(not_class_[0] == class_[0]) 
				continue;

			samples.push_back(trainingDescriptorsClass[not_class_]);
			class_label = Mat::zeros(trainingDescriptorsClass[not_class_].rows, 1, CV_32FC1);
			labels.push_back(class_label);
		}

		Mat samples_32f; samples.convertTo(samples_32f, CV_32F);


		//SVMTrainParamsExt svmParamsExt;
		//CvSVMParams svmParams;
		//CvMat class_wts_cv;
		//setSVMParams( svmParams, class_wts_cv, labels, true );
		//CvParamGrid c_grid, gamma_grid, p_grid, nu_grid, coef_grid, degree_grid;
		//setSVMTrainAutoParams( c_grid, gamma_grid,  p_grid, nu_grid, coef_grid, degree_grid );
		//classifierSVM[class_].train_auto( samples_32f, labels, Mat(), Mat(), svmParams, 10, c_grid, gamma_grid, p_grid, nu_grid, coef_grid, degree_grid );


		classifierSVM[class_].train(samples_32f,labels);
	}

	_fullFilePathList = _filepathForSavingResult + "/" + _filenameForSavingResult;
	ofstream ofile(_fullFilePathList.c_str());
	ofile << filenameVocabulary << endl;
	for (map<string,Mat>::iterator it = trainingDescriptorsClass.begin(); it != trainingDescriptorsClass.end(); ++it) 
	{
		string class_ = (*it).first;
		string _svmName =  "logTrain_svm_" + class_ + ".xml.gz";
		ofile << _svmName <<'\t' << class_ << endl;
		_fullFilePathList = _filepathForSavingResult + "/" + _svmName;
		classifierSVM[class_].save(_fullFilePathList.c_str());
	}
	ofile.close();

	flagTrain = true;

	return true;
}
*/


//void bowCV::setSVMParams( CvSVMParams& svmParams, CvMat& class_wts_cv, const Mat& responses, bool balanceClasses )
//{
//	int pos_ex = countNonZero(responses == 1);
//	int neg_ex = countNonZero(responses == -1);
//	//cout << pos_ex << " positive training samples; " << neg_ex << " negative training samples" << endl;
//
//	svmParams.svm_type = CvSVM::C_SVC;
//	svmParams.kernel_type = CvSVM::RBF;
//	if( balanceClasses )
//	{
//		Mat class_wts( 2, 1, CV_32FC1 );
//		// The first training sample determines the '+1' class internally, even if it is negative,
//		// so store whether this is the case so that the class weights can be reversed accordingly.
//		bool reversed_classes = (responses.at<float>(0) < 0.f);
//		if( reversed_classes == false )
//		{
//			class_wts.at<float>(0) = static_cast<float>(pos_ex)/static_cast<float>(pos_ex+neg_ex); // weighting for costs of positive class + 1 (i.e. cost of false positive - larger gives greater cost)
//			class_wts.at<float>(1) = static_cast<float>(neg_ex)/static_cast<float>(pos_ex+neg_ex); // weighting for costs of negative class - 1 (i.e. cost of false negative)
//		}
//		else
//		{
//			class_wts.at<float>(0) = static_cast<float>(neg_ex)/static_cast<float>(pos_ex+neg_ex);
//			class_wts.at<float>(1) = static_cast<float>(pos_ex)/static_cast<float>(pos_ex+neg_ex);
//		}
//		class_wts_cv = class_wts;
//		svmParams.class_weights = &class_wts_cv;
//	}
//}
//
//void bowCV::setSVMTrainAutoParams( CvParamGrid& c_grid, CvParamGrid& gamma_grid,
//						   CvParamGrid& p_grid, CvParamGrid& nu_grid,
//						   CvParamGrid& coef_grid, CvParamGrid& degree_grid )
//{
//	c_grid = CvSVM::get_default_grid(CvSVM::C);
//
//	gamma_grid = CvSVM::get_default_grid(CvSVM::GAMMA);
//
//	p_grid = CvSVM::get_default_grid(CvSVM::P);
//	p_grid.step = 0;
//
//	nu_grid = CvSVM::get_default_grid(CvSVM::NU);
//	nu_grid.step = 0;
//
//	coef_grid = CvSVM::get_default_grid(CvSVM::COEF);
//	coef_grid.step = 0;
//
//	degree_grid = CvSVM::get_default_grid(CvSVM::DEGREE);
//	degree_grid.step = 0;
//}
