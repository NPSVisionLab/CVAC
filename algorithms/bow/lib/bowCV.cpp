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

/** Bag of Words style trainer and detector.
 *  K Lee, 2012,  matz 2013
 */


#define DLL_EXPORT

#include "bowCV.h"

using namespace std;
using namespace cvac;

const string bowCV::BOW_VOC_FILE = "VOC_FILE";
const string bowCV::BOW_SVM_FILE = "SVM_FILE";
const string bowCV::BOW_DETECTOR_NAME = "FeatureType";
const string bowCV::BOW_EXTRACTOR_NAME = "DescriptorType";
const string bowCV::BOW_MATCHER_NAME = "MatcherType";
const string bowCV::BOW_NUM_WORDS = "NumWords";
const string bowCV::BOW_OPENCV_VERSION = "OPENCV_VERSION";
const string bowCV::BOW_ONECLASS_ID = "ONE-CLASS_ID";

const string bowCV::BOW_REJECT_CLASS_STRATEGY       = "RejectClassStrategy";
const string bowCV::BOW_REJECT_CLASS_AS_MULTICLASS  = "multiclass";
const string bowCV::BOW_REJECT_CLASS_IGNORE_SAMPLES = "ignore";
const string bowCV::BOW_REJECT_CLASS_AS_FIRST_STAGE = "stages";

bowCV* pLib = NULL;

bowCV::bowCV(MsgLogger* _msgLog)
{	
    flagTrain = false;
    flagName = false;
    flagOneClass = false;
    cntCluster = -1;
    mInclassIDforOneClass = 1;
    mOutclassIDforOneClass = 0;
    dda = NULL;
    cv::initModule_nonfree();	//it should be for using SIFT or SURF. 	

    filenameVocabulary = "logTrain_Vocabulary.xml.gz";
    filenameSVM = "logTrain_svm.xml.gz";

    msgLogger = _msgLog;
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

bool bowCV::isInitialized()
{
    return flagName;
}

bool bowCV::train_initialize(const string& _detectorName,
                             const string& _extractorName,
                             const string& _matcherName,
                             int _nCluster,
                             DetectorDataArchive* _dda)
{
    //_detectorName;	//SURF, SIFT, FAST, STAR, MSER, GFTT, HARRIS
    //_extractorName;	//SURF, SIFT, OpponentSIFT, OpponentSURF
    //_matcherName;	//BruteForce-L1, BruteForce, FlannBased  

    flagName = false;
    dda = _dda;

//debug
    printf ("detect name %s, extractor %s, matcher %s, nWords %d\n",
            _detectorName.c_str(),
            _extractorName.c_str(),
            _matcherName.c_str(),
            _nCluster);
    fDetector = FeatureDetector::create(_detectorName);    
    dExtractor = DescriptorExtractor::create(_extractorName);
    if( fDetector.empty() || dExtractor.empty() )
    {
        cout << "Error - featureDetector or descExtractor was not created" << endl;	fflush(stdout);
        return false;
    }
    else
    {
      dda->setProperty(BOW_DETECTOR_NAME,_detectorName);
      dda->setProperty(BOW_EXTRACTOR_NAME,_extractorName);
    }

    cntCluster = _nCluster;

    dMatcher = DescriptorMatcher::create(_matcherName);
    if (dMatcher.empty())
    {
        cout << "Error - descMatcher was not created" << endl;	fflush(stdout);
        return false;
    }
    else
      dda->setProperty(BOW_MATCHER_NAME,_matcherName);

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

bool bowCV::detect_initialize( const DetectorDataArchive* _dda )
{
  dda = (DetectorDataArchive*)_dda;
  return detect_readTrainResult();
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

    dMatcher = DescriptorMatcher::create(_matcherName);
    if (dMatcher.empty())
    {
	    cout << "Error - descMatcher was not created" << endl;	fflush(stdout);
	    return false;
    }

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
        cout << "Error - can't parse train list from path list file: "
             << _fullFilePathList <<endl; 
        fflush(stdout);
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
	        cout<<"Error - cannot read image from file: "
                << _fullFilePathImg << endl;	
            fflush(stdout);
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


bool bowCV::train_run(const string& _filepathForSavingResult,
                      cvac::ServiceManager *sman,
                      float _oneclassNu)
{
    if(!flagName)
    {
        cout << "Need to initialize detector, extractor, matcher, " 
             << "and so on with the function train_initialize()." << endl;	fflush(stdout);
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

    vector<int> vSkipIndex;
    Mat _descriptorRepository;		
    Rect _rect;
    for(unsigned int k=0;k<vFilenameTrain.size();k++)
    {
        _fullFilePathImg = vFilenameTrain[k];

        _img = imread(_fullFilePathImg);
        if(_img.empty())
        {
          string outMsg;
          outMsg = "There is no file " + _fullFilePathImg + 
                   ". This file will be skipped for the processing.\n";
          msgLogger->message(MsgLogger::WARN,outMsg);          
          continue;
        }

        if ((sman!=NULL) && (sman->stopRequested()))
        {
          sman->stopCompleted();
          return false;
        }  

        if((vBoundX[k]<0) || (vBoundY[k]<0) || 
          ((vBoundX[k]+vBoundWidth[k])>_img.cols) || 
          ((vBoundY[k]+vBoundHeight[k])>_img.rows))
        {
          string outMsg;
          outMsg = "Out of boundary in file " + _fullFilePathImg + 
                   ". This file will be skipped for the processing.\n";
          msgLogger->message(MsgLogger::WARN,outMsg);
          continue;
        }
      	
        _rect = Rect(vBoundX[k],vBoundY[k],vBoundWidth[k],vBoundHeight[k]);
        if((_rect.width != 0) && (_rect.height != 0))
	        _img = _img(_rect);

        fDetector->detect(_img, _keypoints);
        if(_keypoints.size()<1) //According to the version of openCV, it may cause an exceptional error.
            continue;

        dExtractor->compute(_img, _keypoints, _descriptors);

        _descriptorRepository.push_back(_descriptors);

        cout<< "Progress of Feature Extraction: " << k+1 << "/" << vFilenameTrain.size() << '\xd';	fflush(stdout);
    }	
    cout << endl;	fflush(stdout);

    cout << "Total number of descriptors: " << _descriptorRepository.rows << endl;	fflush(stdout);

    cout << "Clustering ... this might take some time..." << std::endl;	fflush(stdout);
    
    if(cntCluster > _descriptorRepository.rows)
    {
      string outMsg;
      outMsg = "Error: the number of clusters is smaller than the number of descriptors\n";
      cout << outMsg;
      msgLogger->message(MsgLogger::WARN,outMsg);
      return false;
    }
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
    // END - Setup Vocabulary
    //////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////
    // START - Train Classifier (SVM)
    //////////////////////////////////////////////////////////////////////////
    cout << "Training Classifier ... " << endl;	fflush(stdout);

    Mat trainDescriptors;	trainDescriptors.create(0,bowExtractor->descriptorSize(),bowExtractor->descriptorType());
    Mat trainClass;	trainClass.create(0,1,CV_32FC1);
    int _classID;	

    std::list<int> _listClassAll;
    std::list<int> _listClassUnique;
    for(unsigned int k=0;k<vFilenameTrain.size();k++)
    {
        _fullFilePathImg = vFilenameTrain[k];
        _classID = vClassIDTrain[k];     
            
        if ((sman!=NULL) && (sman->stopRequested()))
        {
            sman->stopCompleted();
            return false;
        }
        _img = imread(_fullFilePathImg);
        if(_img.empty())
          continue;

        if((vBoundX[k]<0) || (vBoundY[k]<0) || 
          ((vBoundX[k]+vBoundWidth[k])>_img.cols) || 
          ((vBoundY[k]+vBoundHeight[k])>_img.rows))
          continue;

        _rect = Rect(vBoundX[k],vBoundY[k],vBoundWidth[k],vBoundHeight[k]);
        if((_rect.width != 0) && (_rect.height != 0))
	        _img = _img(_rect);
      	
        fDetector->detect(_img, _keypoints);
        bowExtractor->compute(_img, _keypoints, _descriptors);
        //cout << "n_pts of " << vFilenameTrain[k]  << ": " << _keypoints.size() << "\n"; fflush(stdout);
        if(_keypoints.size() > 1)
        {           
            trainDescriptors.push_back(_descriptors);
            trainClass.push_back(_classID);
            _listClassAll.push_back(_classID);
        }
        else
        {
            cout << "The file: " << vFilenameTrain[k] << "has no keypoints and will not be used for training!" << endl;
            fflush(stdout);
        }
        
    }
    _listClassUnique = _listClassAll;
    _listClassUnique.sort();
    _listClassUnique.unique();	


    //SVMTrainParamsExt svmParamsExt;
    //CvSVMParams svmParams;
    //CvMat class_wts_cv;
    //setSVMParams( svmParams, class_wts_cv, labels, true );
    //CvParamGrid c_grid, gamma_grid, p_grid, nu_grid, coef_grid, degree_grid;
    //setSVMTrainAutoParams( c_grid, gamma_grid,  p_grid, nu_grid, coef_grid, degree_grid );
    //classifierSVM[class_].train_auto( samples_32f, labels, Mat(), Mat(), svmParams, 10, c_grid, gamma_grid, p_grid, nu_grid, coef_grid, degree_grid );


    /*
    //////////////////////////////////////////////////////
    // Way #1 -
    classifierSVM.train(trainDescriptors,trainClass);
    //////////////////////////////////////////////////////
    */

    //////////////////////////////////////////////////////
    // Way #2
    CvSVMParams param;    
    if(_listClassUnique.size()==1)	//Just for one class
    {
        flagOneClass = true;

        mInclassIDforOneClass = (*_listClassUnique.begin());
        mOutclassIDforOneClass = (mInclassIDforOneClass==0)?1:0;
        stringstream oneclassIDstream;
        oneclassIDstream << mInclassIDforOneClass;
        dda->setProperty(BOW_ONECLASS_ID,oneclassIDstream.str());
        cout << "BoW - One Class Classification \n"; fflush(stdout);

        param.svm_type = CvSVM::ONE_CLASS;
        param.nu = _oneclassNu;	//empirically, this value doesn't have the effect on performance.        
        //param.kernel_type = CvSVM::LINEAR;	//empirically, it should NOT be linear for a better performance.
        classifierSVM.train(trainDescriptors,trainClass,cv::Mat(),cv::Mat(),param);	
    }
    else
    {
        flagOneClass = false;
        cout << "BoW - Multiple Classes Classification \n"; fflush(stdout);

        float* _classWeight = new float[_listClassUnique.size()];
        int _idx = 0;
        int _maxCount = -1;	
        for(std::list<int>::iterator _itr=_listClassUnique.begin();_itr!= _listClassUnique.end();++_itr)		
        {	
            int _count = std::count(_listClassAll.begin(),_listClassAll.end(),(*_itr));
            _maxCount = (_maxCount<_count)?_count:_maxCount;
            _classWeight[_idx++] = (float)_count;
        }
        for(unsigned int k=0;k<_listClassUnique.size();k++)
            _classWeight[k] = (float)_maxCount/_classWeight[k];		
        
        // The order of classWeight is the same with the order of classID. 
        // If there is a list of classID = {-1,2,1}. 
        // Then, the order of classWeights becomes {-1,1,2}. 

        //param.kernel_type = CvSVM::LINEAR;	//empirically, it should NOT be linear for a better performance.

        param.class_weights = new CvMat();	
        cvInitMatHeader(param.class_weights,1,_listClassUnique.size(),CV_32FC1,_classWeight);
        classifierSVM.train(trainDescriptors,trainClass,cv::Mat(),cv::Mat(),param);
        delete [] _classWeight;
    }

    string tSVMPath = _filepathForSavingResult + "/" + filenameSVM;
    classifierSVM.save(tSVMPath.c_str());    
    ///////////////////////////////////////////////////////////////////////////
    // END - Train Classifier (SVM)
    //////////////////////////////////////////////////////////////////////////
    dda->setProperty(BOW_OPENCV_VERSION,CV_VERSION);    

    flagTrain = true;

    return true;
}


bool bowCV::detect_run(const string& _fullfilename, int& _bestClass,int _boxX,int _boxY,int _boxWidth,int _boxHeight)
{	
    _bestClass = -1;	

    if(!flagName)
    {
        cout << "Need to initialize detector, extractor, and matcher with the function detect_initialize()." << endl;
        fflush(stdout);
        return false;
    }

    if(!flagTrain)
    {
        cout << "Error - No training flag found.  Before testing, training is necessary .. " << endl;
        fflush(stdout);
        return false;
    }

    _img = imread(_fullfilename);
    if(_img.empty())
    {
      string outMsg;
      outMsg = "There is no file " + _fullfilename + ".\n";
      msgLogger->message(MsgLogger::WARN,outMsg);
      return false;
    }
    else
    {
      if((_boxX<0) || (_boxY<0) || 
        ((_boxX+_boxWidth)>_img.cols) || ((_boxY+_boxHeight)>_img.rows))
      {
        string outMsg;
        outMsg = "Out of boundary in file " + _fullfilename + ".\n";
        msgLogger->message(MsgLogger::WARN,outMsg);
        return false;
      }
        Rect tRect = Rect(_boxX,_boxY,_boxWidth,_boxHeight);
        if((tRect.width != 0) && (tRect.height != 0))
            _img = _img(tRect);
    }

    fDetector->detect(_img, _keypoints);
    if(_keypoints.size()<1) //According to the version of openCV, it may cause an exceptional error.
    {
        cout << "Error - no feature: " << _fullfilename <<endl;	fflush(stdout);
        return false;
    }
    bowExtractor->compute(_img, _keypoints, _descriptors);
    // In the manual from OpenCV, it is written as follows:
    // "If true (it is for the second argument of this function)
    // and the problem is 2-class classification then the method
    // returns the decision function value that is signed distance to the margin,
    // else the function returns a class label (classification)
    // or estimated function value (regression)."
    // Since we're using it as a multi-class classification tool,
    // we can safely cast to int.
    float predicted = classifierSVM.predict(_descriptors);
    assert( fabs( predicted-((int)predicted) ) < 0.000001 );
    _bestClass = (int) predicted;

    if(!flagOneClass) //For Multi-class problem
    {
        return true;
        //if (-1 == _bestClass) // no class found
        //    return false;
        //else
        //    return true;
    }
    else  //For one-class problem
    {      
        if(_bestClass==1) //in class
            _bestClass = mInclassIDforOneClass;
        else //outlier
            _bestClass = mOutclassIDforOneClass;

        return true;
    }	
}


bool bowCV::detect_readTrainResult()
{	
  string _fullpath, _inputString;

#ifdef __APPLE__
    if (dda->getProperty(BOW_DETECTOR_NAME).compare("SURF") == 0)
    {
        cout << "WARNING!!! SURF detector may not run well on OSX" << endl;
    }
#endif // __APPLE__   

    if(!detect_setParameter(dda->getProperty(BOW_DETECTOR_NAME),
       dda->getProperty(BOW_EXTRACTOR_NAME),
       dda->getProperty(BOW_MATCHER_NAME)))
    {
        cout << "Need to names of detector, extractor, and matcher with the function detect_initialize()" << endl; 
        return false;
    }

    // Read VOC file
    _fullpath = dda->getFile(BOW_VOC_FILE);
    if(detect_readVocabulary(_fullpath,mVocabulary))
    {
        bowExtractor->setVocabulary(mVocabulary);
    }
    else
        return false;    

    // Read SVM file
    _fullpath = dda->getFile(BOW_SVM_FILE);		
    classifierSVM.load(_fullpath.c_str());

    // Read OpenCV Version    
    _inputString = dda->getProperty(BOW_OPENCV_VERSION);
    if(!isCompatibleOpenCV(_inputString))
    {       
        cout << "For the training, OpenCV " << _inputString
             << " was used. But, now you are using OpenCV "
             << CV_VERSION << ". It may cause an exceptional error." << endl; 
        //return false;
    }

    CvSVMParams tParam = classifierSVM.get_params();
    if(tParam.svm_type == CvSVM::ONE_CLASS)
    {
        flagOneClass = true;
        cout << "BoW - One Class Classification \n"; fflush(stdout);
        
        _inputString = dda->getProperty(BOW_ONECLASS_ID);
        if(_inputString.empty())
            mInclassIDforOneClass = 1; //default in-class ID of one-Class SVM
        else
            mInclassIDforOneClass = atoi(_inputString.c_str());  
        
        mOutclassIDforOneClass = (mInclassIDforOneClass==0)?1:0;
        cout << "In-class data will be represented by " << mInclassIDforOneClass << endl;
        cout << "Out-class data will be represented by " << mOutclassIDforOneClass << endl;
    }
    else
    {
        flagOneClass = false;
        cout << "BoW - Multiple Classes Classification \n"; fflush(stdout);
    }

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
    cout << "error" << endl;	fflush(stdout);
    return false;
}


bool bowCV::isCompatibleOpenCV(const string& _version)
{
    if(_version.empty())
    {
        cout << "Because this training data is old version, it doesn't include version "
             << "information. Therefore, it may cause an exceptional error while processing "
             << "images." << endl;
        return true;
    }

    string tVersionCurrent(CV_VERSION);
    if(_version == "2.4.2" || _version == "2.4.3")
    {
         if(tVersionCurrent == "2.4.2" || tVersionCurrent == "2.4.3")
            return true;
         else if(tVersionCurrent == "2.4.5")  //this case definitely causes error.
            return false;
         else
            return false;  //the other cases should be examined.
    }
    else if(_version == tVersionCurrent)
        return true;
    else
        return false;
}

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
