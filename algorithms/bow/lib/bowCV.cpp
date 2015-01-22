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
const string bowCV::BOW_CLASS_WEIGHT = "ClassWeights";
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
    flagClassWeight = true;
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
                             bool _flagClassWeight,
                             DetectorDataArchive* _dda)
{
    //_detectorName;	//SURF, SIFT, FAST, STAR, MSER, GFTT, HARRIS
    //_extractorName;	//SURF, SIFT, OpponentSIFT, OpponentSURF
    //_matcherName;	//BruteForce-L1, BruteForce, FlannBased  

    flagClassWeight = _flagClassWeight;
    flagName = false;
    dda = _dda;

    std::string msgout;
    std::stringstream val2str;    
    val2str << _nCluster;
    
    msgout = "Info: Detector: " + _detectorName
      + ", Extractor: " + _extractorName
      + ", Matcher: " + _matcherName
      + ", nWords: " + val2str.str() + ".\n";
    message(msgout,MsgLogger::DEBUG);

    fDetector = FeatureDetector::create(_detectorName);    
// When SIFT feature is used, you may adjust its performence with the following options
//     fDetector->set("nOctaveLayers",1); //smaller -> smaller features
//     fDetector->set("contrastThreshold",0.1); //larger -> smaller features
//     fDetector->set("edgeThreshold",18.0); //larger -> smaller features
//     fDetector->set("sigma",1.9);//larger -> smaller features

    dExtractor = DescriptorExtractor::create(_extractorName);
    if( fDetector.empty() || dExtractor.empty() )
    {
      msgout = "Error: featureDetector or descExtractor was not created.\n";
      message(msgout,MsgLogger::WARN);
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
      msgout = "Error: descMatcher was not created.\n";
      message(msgout,MsgLogger::WARN);
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

    std::string msgout;    

    fDetector = FeatureDetector::create(_detectorName);
    dExtractor = DescriptorExtractor::create(_extractorName);
    if( fDetector.empty() || dExtractor.empty() )
    {
      msgout = "Error: featureDetector or descExtractor was not created.\n";
      message(msgout,MsgLogger::WARN);
	  return false;
    }	

    dMatcher = DescriptorMatcher::create(_matcherName);
    if (dMatcher.empty())
    {
      msgout = "Error: descMatcher was not created.\n";
      message(msgout,MsgLogger::WARN);
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
    std::string msgout;    

    _fullFilePathList = _filepathTrain + "/" + _filenameTrainList;
    ifstream ifList(_fullFilePathList.c_str());
    if(!ifList.is_open())
    {
      msgout = "Error: can't parse a train list from the file \"" 
        + _fullFilePathList + "\"\n";
      message(msgout,MsgLogger::WARN);
	  return false;
    }
    
    msgout = "Checking training files...";
    message(msgout,MsgLogger::DEBUG);
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

        _img = imread(_fullFilePathImg,CV_LOAD_IMAGE_GRAYSCALE);
        if(_img.empty())
        {
	      //ifList.close();
          msgout = "Error: can't read this image file \"" 
            + _fullFilePathImg + "\".\n";
          message(msgout,MsgLogger::WARN);
          continue;
	      //return false;
        }
        train_stackTrainImage(_fullFilePathImg,_classID,_rect.x,_rect.y,_rect.width,_rect.height);

    } while (!ifList.eof());
    ifList.close();	

    std::stringstream val2str;
    val2str << vFilenameTrain.size();

    msgout = "The number of images for training is " + val2str.str() + ".\n";
    message(msgout,MsgLogger::DEBUG);

    if(vFilenameTrain.size()<1)
      return false;
    else
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
    std::string msgout;

    if(!flagName)
    {
      msgout = "Error: need to initialize detector, extractor, matcher," 
        "and so on with the function train_initialize()";
      message(msgout,MsgLogger::WARN);
      return false;
    }

    if(vFilenameTrain.size() < 1)
    {
      msgout = "There is no training images.\n";
      message(msgout,MsgLogger::WARN);
      return false;
    }
    else
    {
      msgout = "Training is started.\n";
      message(msgout,MsgLogger::DEBUG);
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

        _img = imread(_fullFilePathImg,CV_LOAD_IMAGE_GRAYSCALE);
        if(_img.empty())//no file or not supported format
        {
          msgout = "The file \"" + _fullFilePathImg + 
            "\" has a problem (no file or not supported format). "+
            "So, it will not be processed in the training.\n";
          message(msgout,MsgLogger::WARN);
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
          msgout = "The file \"" + _fullFilePathImg + 
            "\" has a problem (out of boundary). " +
            "So, it will not be processed in the training.\n";
          message(msgout,MsgLogger::WARN);
          continue;
        }

        _rect = Rect(vBoundX[k],vBoundY[k],vBoundWidth[k],vBoundHeight[k]);
        if((_rect.width != 0) && (_rect.height != 0))
	        _img = _img(_rect);
        
        // Sometimes, opencv returns an memory-related error 
        // because of the large number of extracted features
        try
        {
          fDetector->detect(_img, _keypoints);          
        }
        catch(cv::Exception& e)
        { 
          // limitImageSize(_img);
          std::string err_str = std::string(e.what());
          char sizeStr[128];
          sprintf(sizeStr, "%dx%d", _img.cols, _img.rows);
          msgout = "The file \"" + _fullFilePathImg + 
            "\" has a problem (OpenCV Error). Image size= " + sizeStr +
            " So, it will not be processed in the training. Details: " + 
            err_str ;
          message(msgout,MsgLogger::WARN);          
          continue;
        }

        if(_keypoints.size()<1) //According to the version of openCV, it may cause an exceptional error.
        { 
          msgout = "The file \"" + _fullFilePathImg + 
            "\" has a problem (no feature). " + 
            "So, it will not be processed in the training.\n";
          message(msgout,MsgLogger::WARN);
          continue;
        }

        dExtractor->compute(_img, _keypoints, _descriptors);
        
        try
        {
          _descriptorRepository.push_back(_descriptors);          
        }
        catch(cv::Exception& e)
        { 
          // limitImageSize(_img);
          std::string err_str = std::string(e.what());

          msgout = "While processing this file \"" + _fullFilePathImg + 
            "\", a critical error occurred (OpenCV Error). It might be caused by " + 
            "insufficient memory. So. This training process is stopping. Details: " + 
            err_str;
          message(msgout,MsgLogger::ERROR);          
          return false;
        }

        std::stringstream val2str1,val2str2;
        val2str1 << k+1;
        val2str2 << vFilenameTrain.size();

        msgout = "Progress of Feature Extraction: " 
          + val2str1.str() + "/" + val2str2.str();        
        messageInSameline(msgout);
    }
    std::stringstream val2str;
    val2str << _descriptorRepository.rows;

    msgout = "\nTotal number of descriptors: " + val2str.str() 
      + ".\nStarting clustering. This may take a long time ...\n";
    message(msgout,MsgLogger::DEBUG);

    if(cntCluster > _descriptorRepository.rows)
    {
      msgout = "Error: the number of clusters is smaller "
               "than the number of descriptors\n";
      message(msgout,MsgLogger::WARN);
      return false;
    }
    BOWKMeansTrainer bowTrainer(cntCluster); 
    bowTrainer.add(_descriptorRepository);
    
    //for saving the memory
    _descriptorRepository.release();

    mVocabulary = bowTrainer.cluster();	

    if(!train_writeVocabulary(_filepathForSavingResult + "/" + filenameVocabulary,mVocabulary))
        return false;
    //////////////////////////////////////////////////////////////////////////
    // END - Clustering (Most time-consuming step)
    //////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////
    // START - Setup Vocabulary
    //////////////////////////////////////////////////////////////////////////
    msgout = "Starting vocabulary setup.\n";
    message(msgout,MsgLogger::DEBUG);

    bowExtractor->setVocabulary(mVocabulary);
    //////////////////////////////////////////////////////////////////////////
    // END - Setup Vocabulary
    //////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////
    // START - Train Classifier (SVM)
    //////////////////////////////////////////////////////////////////////////
    msgout = "Starting training the classifier.\n";
    message(msgout,MsgLogger::DEBUG);

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
        _img = imread(_fullFilePathImg,CV_LOAD_IMAGE_GRAYSCALE);
        if(_img.empty())
          continue;

        if((vBoundX[k]<0) || (vBoundY[k]<0) || 
          ((vBoundX[k]+vBoundWidth[k])>_img.cols) || 
          ((vBoundY[k]+vBoundHeight[k])>_img.rows))
          continue;

        _rect = Rect(vBoundX[k],vBoundY[k],vBoundWidth[k],vBoundHeight[k]);
        if((_rect.width != 0) && (_rect.height != 0))
	        _img = _img(_rect);

        try
        {
          fDetector->detect(_img, _keypoints);          
        }
        catch(cv::Exception& e)
        {
          //limitImageSize(_img);
          //std::string err_str = std::string(e.what());
          continue;
        }
        
        //cout << "n_pts of " << vFilenameTrain[k]  << ": " << _keypoints.size() << "\n"; fflush(stdout);
        if(_keypoints.size() >= 1)
        { 
          bowExtractor->compute(_img, _keypoints, _descriptors);

          try
          {
            trainDescriptors.push_back(_descriptors);
          }       
          catch(cv::Exception& e)
          { 
            // limitImageSize(_img);
            std::string err_str = std::string(e.what());

            msgout = "While processing this file \"" + _fullFilePathImg + 
              "\", a critical error occurred (OpenCV Error). " + 
              "This training process will be stopped. Details: " + 
              err_str;
            message(msgout,MsgLogger::ERROR);          
            return false;
          }
          trainClass.push_back(_classID);
          _listClassAll.push_back(_classID);
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
        msgout = "BoW - One Class Classification. \n";
        message(msgout,MsgLogger::DEBUG);

        param.svm_type = CvSVM::ONE_CLASS;
        param.nu = _oneclassNu;	//empirically, this value doesn't have the effect on performance.        
        //param.kernel_type = CvSVM::LINEAR;	//empirically, it should NOT be linear for a better performance.
        classifierSVM.train(trainDescriptors,trainClass,cv::Mat(),cv::Mat(),param);	
    }
    else
    {
        flagOneClass = false;
        msgout = "BoW - Multiple Classes Classification. \n";
        message(msgout,MsgLogger::DEBUG);

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
        if(flagClassWeight)
        {
          param.class_weights = new CvMat();	
          cvInitMatHeader(param.class_weights,1,_listClassUnique.size(),CV_32FC1,_classWeight);
        }
        dda->setProperty(BOW_CLASS_WEIGHT,flagClassWeight?"True":"False");
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


std::string bowCV::detect_run(const string& _fullfilename, int& _bestClass,int _boxX,int _boxY,int _boxWidth,int _boxHeight)
{	
    _bestClass = -1;	
    std::string msgout;
    std::string msgReturn;

    if(!flagName)
    {
      msgout = "Error: Need to initialize detector, extractor, and matcher "
               "with the function detect_initialize().\n";
      message(msgout,MsgLogger::WARN);
      msgReturn = "Error: need appropriate initialization";
      return msgReturn;
    }

    if(!flagTrain)
    {
      msgout = "Error: No training flag found. "
               "Before testing, training is necessary.\n";
      message(msgout,MsgLogger::WARN);
      msgReturn = "Error: need appropriate initialization";
      return msgReturn;
    }

    _img = imread(_fullfilename,CV_LOAD_IMAGE_GRAYSCALE);
    if(_img.empty())//no file or not supported format
    {
      msgout = "The file \"" + _fullfilename + 
        "\" has a problem (no file or not supported format). "+
        "So, it will not be processed.\n";
      message(msgout,MsgLogger::WARN);
      msgReturn = "Error: no file or not supported format";
      return msgReturn;
    }
    else
    {
      if((_boxX<0) || (_boxY<0) || 
        ((_boxX+_boxWidth)>_img.cols) || ((_boxY+_boxHeight)>_img.rows))
      {
        msgout = "The file \"" + _fullfilename + 
          "\" has a problem (out of boundary). "+
          "So, it will not be processed.\n";
        message(msgout,MsgLogger::WARN);
        msgReturn = "Error: invalid boundary info";
        return msgReturn;
      }
        Rect tRect = Rect(_boxX,_boxY,_boxWidth,_boxHeight);
        if((tRect.width != 0) && (tRect.height != 0))
            _img = _img(tRect);
    }

    // Sometimes, opencv returns an memory-related error 
    // while processing an image because of the large number of 
    // extracted features
    try
    {
      fDetector->detect(_img, _keypoints);
    }
    catch(cv::Exception& e)
    {
      //limitImageSize(_img);
      std::string err_str = std::string(e.what());
      char sizeStr[128];
      sprintf(sizeStr, "%dx%d", _img.cols, _img.rows);
      msgout = "The file \"" + _fullfilename + 
         "\" has a problem (OpenCV Error). Image size= " + sizeStr + " : "+
        err_str;
      message(msgout,MsgLogger::WARN);
      msgReturn = "Error: opencv error";
      return msgReturn;
    }
    
    if(_keypoints.size()<1) //According to the version of openCV, it may cause an exceptional error.
    {
      msgout = "The file \"" + _fullfilename + 
        "\" has a problem (no feature). "+
        "So, it will not be processed.\n";
      message(msgout,MsgLogger::WARN);
      msgReturn = "Error: no feature";
      return msgReturn;
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
       msgReturn = "";
       return msgReturn;
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

        msgReturn = "";
        return msgReturn;
    }	
}


bool bowCV::detect_readTrainResult()
{
  std::string msgout;
  string _fullpath, _inputString;

#ifdef __APPLE__
    if (dda->getProperty(BOW_DETECTOR_NAME).compare("SURF") == 0)
    {
      msgout = "WARNING!!! SURF detector may not run well on OSX.\n";
      message(msgout,MsgLogger::WARN);      
    }
#endif // __APPLE__   

    if(!detect_setParameter(dda->getProperty(BOW_DETECTOR_NAME),
       dda->getProperty(BOW_EXTRACTOR_NAME),
       dda->getProperty(BOW_MATCHER_NAME)))
    {
      msgout = "Error: need to initialize detector, extractor, matcher, "
               "and so on with the function detect_initialize().\n";
      message(msgout,MsgLogger::WARN);
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
      msgout = "For the training, OpenCV " + _inputString 
        + " was used. But, now you are using OpenCV "
        + CV_VERSION + ". It may cause an exceptional error.\n";
      message(msgout,MsgLogger::WARN);
        //return false;
    }

    CvSVMParams tParam = classifierSVM.get_params();
    if(tParam.svm_type == CvSVM::ONE_CLASS)
    {
        flagOneClass = true;
        msgout = "BoW - One Class Classification. \n";
        message(msgout,MsgLogger::DEBUG);
        
        _inputString = dda->getProperty(BOW_ONECLASS_ID);
        if(_inputString.empty())
            mInclassIDforOneClass = 1; //default in-class ID of one-Class SVM
        else
            mInclassIDforOneClass = atoi(_inputString.c_str());  
        
        mOutclassIDforOneClass = (mInclassIDforOneClass==0)?1:0;

        std::stringstream val2str1,val2str2;    
        val2str1 << mInclassIDforOneClass;
        val2str2 << mOutclassIDforOneClass;

        msgout = "In-class data will be represented by "+val2str1.str()+
          ", and Out-class data will be represented by "+val2str2.str()+".\n";
        message(msgout,MsgLogger::DEBUG);
    }
    else
    {
      flagOneClass = false;
      msgout = "BoW - Multiple Classes Classification. \n";
      message(msgout,MsgLogger::DEBUG);
    }

    flagTrain = true;

    return true;
}


bool bowCV::train_writeVocabulary(const string& _filename,const Mat& _vocabulary)
{
    std::string msgout;
    msgout = "Saving vocabulary.\n";
    message(msgout,MsgLogger::DEBUG);
    
    FileStorage fs( _filename, FileStorage::WRITE );
    if( fs.isOpened() )
    {
        fs << "vocabulary" << _vocabulary;
        fs.release();
        return true;
    }
    fs.release();

    msgout = "Error: in Saving vocabulary.\n";
    message(msgout,MsgLogger::WARN);
    return false;
}

bool bowCV::detect_readVocabulary( const string& _filename, Mat& _vocabulary )
{
    std::string msgout;
    msgout = "Reading vocabulary.\n";
    message(msgout,MsgLogger::DEBUG);
    
    FileStorage fs( _filename, FileStorage::READ );
    if( fs.isOpened() )
    {
        fs["vocabulary"] >> _vocabulary;
        //cout << "done" << endl;
        fs.release();
        return true;
    }
    fs.release();    
    msgout = "Error: in Reading vocabulary.\n";
    message(msgout,MsgLogger::WARN);
    return false;
}


bool bowCV::isCompatibleOpenCV(const string& _version)
{
    if(_version.empty())
    {
      std::string msgout = "Because this training data is old version, "
                           "it doesn't include version information. "
                           "Therefore, it may cause an exceptional error "
                           "while processing images.\n";
      message(msgout,MsgLogger::WARN);
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

void bowCV::message(const string& _msg,
                    MsgLogger::Levels _levelClient)
{
  //cout << _msg; fflush(stdout);

  if(MsgLogger::SILENT != _levelClient)
  {
    msgLogger->message(_levelClient,_msg);
  }
}

void bowCV::messageInSameline(const string& _msg)
{
  cout << _msg << '\xd'; fflush(stdout);
}

// void bowCV::limitImageSize(Mat& _image)
// {
//   return;
// 
//   int _row = _image.rows;
//   int _col = _image.cols;
// 
//   int maxSizeImg = 1000;
//   if(_row*_col > maxSizeImg*maxSizeImg)
//   {
//     double _rowRatio = (double)maxSizeImg/(double)_row;
//     double _colRatio = (double)maxSizeImg/(double)_col;
// 
//     int _newRow = (_rowRatio<_colRatio)?(_rowRatio*_row):(_colRatio*_row);
//     int _newCol = (_rowRatio<_colRatio)?(_rowRatio*_col):(_colRatio*_col);
// 
//     Mat imgOri = _image.clone();
//     resize( imgOri, _image, Size(_newCol,_newRow),0,0,INTER_LANCZOS4);
//     cout << "Iamge is resized to (" << _newCol << ", " << _newRow << ")\n";
//     fflush(stdout);
//   }
// }

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
