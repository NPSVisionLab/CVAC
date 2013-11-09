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

# include <util/MediaConverter.h>

using namespace cvac;

MediaConverter::MediaConverter(ServiceManager *_sman)
: mServiceMan(_sman)
{
}

MediaConverter_openCV_i2i::MediaConverter_openCV_i2i(ServiceManager *_sman)
: MediaConverter(_sman)
{
}

bool MediaConverter_openCV_i2i::convert(const string& _srcAbsPath,
                                        const string& _desAbsDir,
                                        const string& _desFilename,
                                        vector<string>& _resFilename,
                                        vector<string>& _resAuxInfo)
{
  _resAuxInfo.clear();
  _resFilename.clear();
  string tDesPath = _desAbsDir + "/" + _desFilename;

  cv::Mat tImg = cv::imread(_srcAbsPath);
  if(tImg.empty())
  {    
    localAndClientMsg(VLogger::WARN, NULL,"Conversion error from %s to %s.\n",
      _srcAbsPath.c_str(),tDesPath.c_str());
    return false;
  }

  if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
  {   
    mServiceMan->stopCompleted();
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

MediaConverter_openCV_v2i::MediaConverter_openCV_v2i(ServiceManager *_sman,
                                                     int _perFrame)
: MediaConverter(_sman), PerFrame(_perFrame)
{
}

MediaConverter_openCV_v2i::~MediaConverter_openCV_v2i()
{
  if(mVideoFile.isOpened())
    mVideoFile.release();
}

bool MediaConverter_openCV_v2i::convert(const string& _srcAbsPath,
                                        const string& _desAbsDir,
                                        const string& _desFilename,
                                        vector<string>& _resFilename,
                                        vector<string>& _resAuxInfo)
{
  _resAuxInfo.clear();
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
      _srcAbsPath.c_str(),tDesAbsPath.c_str());
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
      if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
      {        
        mServiceMan->stopCompleted();
        return false;
      }

      if((++tCount%PerFrame)==0)
      {	
        std::ostringstream ss;
        ss << std::setw( tnDigit ) << std::setfill( '0' ) << tCount;
        tfileNameNew = fileNamePre + "_" + ss.str() + fileNameSuf;
        tDesAbsPath = _desAbsDir + "/" + tfileNameNew;

        if(imwrite(tDesAbsPath,tMatFrame))
        {
          _resFilename.push_back(tfileNameNew);
          _resAuxInfo.push_back(ss.str());
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