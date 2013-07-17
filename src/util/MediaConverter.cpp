# include <util/MediaConverter.h>

using namespace cvac;

bool MediaConverter_openCV_i2i::convert(const string& _srcAbsPath,
                                        const string& _desAbsDir,
                                        const string& _desFilename,
                                        vector<string>& _resFilename)
{
  _resFilename.clear();
  string tDesPath = _desAbsDir + "/" + _desFilename;

  cv::Mat tImg = cv::imread(_srcAbsPath);
  if(tImg.empty())
  {    
    localAndClientMsg(VLogger::WARN, NULL,"Conversion error from %s to %s.\n",
      _srcAbsPath.c_str(),tDesPath.c_str());
    return false;
  }

  if(imwrite(tDesPath,tImg))
  { 
    _resFilename.push_back(_desFilename);
    return true;
  }
  else
    return false;
    
}

MediaConverter_openCV_v2i::~MediaConverter_openCV_v2i()
{
  if(mVideoFile.isOpened())
    mVideoFile.release();
}

bool MediaConverter_openCV_v2i::convert(const string& _srcAbsPath,
                                        const string& _desAbsDir,
                                        const string& _desFilename,
                                        vector<string>& _resFilename)
{
  _resFilename.clear();
   string tDesAbsPath = _desAbsDir + "/" + _desFilename;

  if(mVideoFile.isOpened())
    mVideoFile.release();
 
  //////////////////////////////////////////////////////////////////////////
  // Check - is Opened successfully?
  mVideoFile.open(_srcAbsPath);
  int tnFrame = -1;
  if(mVideoFile.isOpened())
    tnFrame = (long)mVideoFile.get(CV_CAP_PROP_FRAME_COUNT);
  else
  {
    localAndClientMsg(VLogger::WARN, NULL,"Conversion error from %s to %s.\n",
      _srcAbsPath.c_str(),tDesAbsPath.c_str());return false;		
    return false;
  }
  int tnDigit = (tnFrame>0)?(int)log10((double)tnFrame)+1:1;
  tnDigit += 1;
  //////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////
  // Adjusting Filename
  string fileNamePre,fileNameSuf;
  string::size_type dot = _desFilename.rfind(".");
  fileNamePre = _desFilename.substr(0,dot);    
  fileNameSuf = _desFilename.substr(dot,_desFilename.length());
  //////////////////////////////////////////////////////////////////////////  
  

  //////////////////////////////////////////////////////////////////////////
  // Video to Images
  cv::Mat tMatFrame;	
  string tfileNameNew;
  long tCount = 0;		  
  bool tFlagRepeat = true;
  do{
    tFlagRepeat = mVideoFile.read(tMatFrame);
    if(tFlagRepeat)
    {      
      if((++tCount%PerFrame)==0)
      {	
        std::ostringstream ss;
        ss << std::setw( tnDigit ) << std::setfill( '0' ) << tCount;
        tfileNameNew = fileNamePre + "_" + ss.str() + fileNameSuf;
        tDesAbsPath = _desAbsDir + "/" + tfileNameNew;

        if(imwrite(tDesAbsPath,tMatFrame))
        {
          _resFilename.push_back(tfileNameNew);
        }
        else
        {
          localAndClientMsg(VLogger::WARN, NULL,
            "Conversion error from %s to %s.\n",
            _srcAbsPath.c_str(),tDesAbsPath.c_str());
          return false;	
        }
      }
    }

  }while(tFlagRepeat);

  return true;  
}