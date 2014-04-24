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
#include <util/RunSetIterator.h>
#include <util/FileUtils.h>

using namespace Ice;
using namespace cvac;


RunSetIterator::RunSetIterator(RunSetWrapper* _rsw,RunSetConstraint& _cons,
                               ServiceManager *_sman,
                               const CallbackHandlerPrx& _callback,
                               int _nSkipFrames)
{   
  mFlagInitialize = false;
  mServiceMan = _sman;

  mCallback2Client = _callback;
  assert(mCallback2Client != 0);

  mMediaTempDirectory = "";  
  if(_rsw != NULL)
  {
    mMediaRootDirectory = _rsw->getRootDir();
    mResultSet = _rsw->getResultSet();
    mResultSetType = _rsw->getResultSetType();
    assert(mResultSet.results.size() == mResultSetType.size());
  }      

  mConstraintType = _cons.mimeTypes;
  mConstraintPurpose = _cons.compatiblePurpose;
  mLost = _cons.excludeLostFrames;
  if (_cons.spacesInFilenamesPermitted)
	  mNoSpaces = false;
  else {
	  mNoSpaces = true;
	  std::string dir = getCurrentWorkingDirectory();
#ifdef WIN32
      char *tTempName = _tempnam(dir.c_str(), NULL);
#else
      char *tTempName = tempnam(dir.c_str(), NULL);
#endif /* WIN32 */
      mTempDir = tTempName; //for being used in making a symbolic link
  }
  mOccluded = _cons.excludeOccludedFrames;

  //////////////////////////////////////////////////////////////////////////    
  mConv_openCV_i2i = new MediaConverter_openCV_i2i(_sman);
  mConv_openCV_v2i = new MediaConverter_openCV_v2i(_sman,_nSkipFrames);
  makeConversionList();  
  //////////////////////////////////////////////////////////////////////////    

  convert();
}

RunSetIterator::~RunSetIterator()
{	
  delete mConv_openCV_i2i;
  
  clear();
  if (!mTempDir.empty())
  {
	  deleteDirectory(mTempDir);
  }
  //if(!mMediaTempDirectory.empty())
  //  deleteDirectory(mMediaRootDirectory + "/" + mMediaTempDirectory);
}

void RunSetIterator::clear()
{
  mList.clear();
  mListOrginalIdx.clear();
}

//---------------------------------------------------------------------------
bool RunSetIterator::isInConstraintType(const rsMediaType& _type)
{  
  if(mConstraintType.empty())
    return true;
  else
    return (find(mConstraintType.begin(),mConstraintType.end(),_type)==mConstraintType.end())?false:true;
}

//---------------------------------------------------------------------------
void RunSetIterator::addToList(const LabelablePtr _pla,int _originalIdx)
{
  if(!_pla)
  {
    localAndClientMsg(VLogger::WARN, NULL,"Empty LabelablePtr is assigned\n");
    return;
  }

  if (mNoSpaces)
  { // check if filename has spaces and get symlink
		bool newSymlink;
		std::string symlinkFullPath = getLegalPath(mTempDir, _pla->sub.path, newSymlink);            
		if(newSymlink)
		{             
			cout << "symbolic link is going to be generated..\n";
			string fPath = getFSPath(_pla->sub.path, mMediaRootDirectory);
			if (makeSymlinkFile(symlinkFullPath, fPath))
			{
				DirectoryPath dirPath;
				dirPath.relativePath = getFileDirectory(symlinkFullPath);
				FilePath filePath;
				filePath.directory = dirPath;
				filePath.filename = getFileName(symlinkFullPath);
				Substrate sub(_pla->sub.isImage, _pla->sub.isVideo, filePath, 0, 0);
			    LabelablePtr newlab = new Labelable(_pla->confidence, _pla->lab, sub);
				mList.push_back(newlab);
			}else
				mList.push_back(_pla);
		}else
			mList.push_back(_pla);

  }else
  {
    mList.push_back(_pla);  
  }
  mListOrginalIdx.push_back(_originalIdx);
}

//---------------------------------------------------------------------------
bool RunSetIterator::makeList(ResultSet& _resultSet,
                              vector<rsMediaType>& _resultSetType)
{
  clear();

  string tAbsPath,tRelDir,tFname;

  //Make a directory with a random name
  bool tFlagMakeDirectory = true;
  srand((unsigned)time(NULL));
  std::ostringstream tConvNum2Str;
  tConvNum2Str << rand();
  mMediaTempDirectory = "converted_" + tConvNum2Str.str();

  //Make a final set with a conversion if it's necessary
  unsigned int k;
  for(k=0;k<_resultSet.results.size();k++)  
  {
    if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
    {        
      mServiceMan->stopCompleted();
      return false;
    }
    
    if(isInConstraintType(_resultSetType[k]))
      addToList(_resultSet.results[k].original,k);
    else 
    {
      //if necessary, convert to one of constraint types
      bool tFlagConversion = false;
      mConstraintTypeItr=mConstraintType.begin();
      for(;mConstraintTypeItr!=mConstraintType.end();mConstraintTypeItr++)
      {         
        mConvertibleItr = mConvertible.find(_resultSetType[k] + "2" 
                          + (*mConstraintTypeItr));
        if(mConvertibleItr!=mConvertible.end())
        {
          if(tFlagMakeDirectory)
          {
            makeDirectory(mMediaRootDirectory + "/" + mMediaTempDirectory);
            tFlagMakeDirectory = false;
          }
          
          if(!convertAndAddToList(_resultSet.results[k].original,(*mConstraintTypeItr),
            mConvertibleItr->second,mMediaTempDirectory,k))
          {
            //this case may be very rare: error while converting, stopping by user interruption
            localAndClientMsg(VLogger::ERROR, mCallback2Client,
              "Conversion is stopped by error or user interruption for %s.\n",
              (_resultSet.results[k].original)->sub.path.filename.c_str());
            return false;
          }

          tFlagConversion = true;
          break;  //Should be
        } 
      }

      //When there is no proper converter
      if(!tFlagConversion)  
      {
        localAndClientMsg(VLogger::WARN, mCallback2Client,
          "No conversion for %s because of no proper converter\n",
          (_resultSet.results[k].original)->sub.path.filename.c_str());
      }
    }
  } 

  return true;
}

//---------------------------------------------------------------------------
bool RunSetIterator::makeList(ResultSet& _resultSet)
{
  clear(); 
  unsigned int k;
  for(k=0;k<_resultSet.results.size();k++)  
    addToList(_resultSet.results[k].original,k);

  return true;
}

// Create a new Labelable that is consistant with the target type.  If we have a video source type and
// and a image target type then convert the Labelable from a video type to image type. We need
// to new it since the contents will be changed.  For now we assume the target type will always be
// an image target type.
LabelablePtr RunSetIterator::cloneLabelablePtr(const LabelablePtr _pla, int frameNum)
{
  LabelablePtr result;
  LabeledLocationPtr locptr = LabeledLocationPtr::dynamicCast(_pla);
  LabeledVideoSegmentPtr vsptr = LabeledVideoSegmentPtr::dynamicCast(_pla);
  LabeledTrackPtr tptr = LabeledTrackPtr::dynamicCast(_pla);
  if (locptr)
  {
    locptr = new LabeledLocation(*locptr);
    result = (LabelablePtr)locptr;
  }else if (vsptr)
  {
    locptr = new LabeledLocation();
    locptr->loc = vsptr->loc;
    result = (LabelablePtr)locptr;
  }else if (tptr && frameNum != -1)
  {
    FrameLocationList frames = tptr->keyframesLocations;
    FrameLocationList::iterator it;
    for (it = frames.begin(); it != frames.end(); it++)
    {
        FrameLocation floc = *it;
        if (floc.frame.framecnt == frameNum)
        {
            locptr = new LabeledLocation();
            locptr->loc = floc.loc;
            break;
        }
    }
    if (locptr.get() != NULL)
        //result = LabelablePtr::dynamicCast(locptr);
        result = (LabelablePtr)locptr;
    else
    {
        // The object is not in this frame.  If we return a default labelable
        // The the whole image size will be selected so instead we return
        // NULL which should cause frame not to be used.
        result = NULL;
    }
  }else
  {
    result = new Labelable(*_pla);
  }
  return result;
}

bool RunSetIterator::convertAndAddToList(const LabelablePtr& _pla,
                                         const rsMediaType& _targetType,
                                         MediaConverter* _pConv,
                                         const string& _rDirTemp,int _originalIdx)
{
  string tFileNameOld = _pla->sub.path.filename;
  string tFileNameNew = tFileNameOld + "." + _targetType;
  string tRDirOld = _pla->sub.path.directory.relativePath;
  string tRDirNew = _rDirTemp;

  string tAbsPathOld = mMediaRootDirectory + "/" + tRDirOld + "/" + tFileNameOld;
  string tAbsDirNew = mMediaRootDirectory + "/" + tRDirNew;
  string tAbsPathNew = tAbsDirNew + "/" + tFileNameNew;

  vector<string> tListFileName;
  vector<string> tAuxInfo;
  if(_pConv->convert(tAbsPathOld,tAbsDirNew,tFileNameNew,tListFileName,tAuxInfo))
  {
    //if tAuxInfo is not empty, above conversion comes from "video2image"
    //the variable "tAuxInfo" includes frame numbers
    vector<string>::iterator tItrFilename = tListFileName.begin();
    for(int _idx=0;tItrFilename!=tListFileName.end();tItrFilename++,_idx++)
    {
      if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
      {        
        mServiceMan->stopCompleted();
        return false;
      }

      int frameNum = -1;
      if(!tAuxInfo.empty())
          frameNum = atoi(tAuxInfo[_idx].c_str());
      
      LabelablePtr _la = cloneLabelablePtr(_pla, frameNum);
      if (_la.get() != NULL)
      {
          _la->sub.isImage = true;
          //currently, conversion target is only an image.
          //refer to the function in RunSetWrapper: getTypeMacro
          _la->sub.isVideo = !(_la->sub.isImage);
          _la->sub.path.filename = (*tItrFilename);
          _la->sub.path.directory.relativePath = tRDirNew;
          _la->lab.hasLabel = false;

          if(!tAuxInfo.empty())
            _la->lab.name = tAuxInfo[_idx];
          else
            _la->lab.name = "";
          /*
          if(!tAuxInfo.empty())
          {
            FrameLocation _tFrm;
            _tFrm.frame.time = 0;
            _tFrm.frame.framecnt = atoi(tAuxInfo[_idx].c_str());
            (static_cast<LabeledTrack*>(_la.get()))->keyframesLocations.push_back(_tFrm);
          }
          */
          addToList(_la,_originalIdx);
       }
    }
    return true;
  }
  else
    return false;
}

bool RunSetIterator::convert()
{ 
  mFlagInitialize = true;

  if(mResultSet.results.size()==0 || mConstraintType.empty())
  {    
    localAndClientMsg(VLogger::WARN, NULL,"Empty RunSetWrapper or constraints.\n");
    if(makeList(mResultSet))
      initIterator(); //as it is
    mFlagInitialize = false;
  }    
  else
  {
    if(makeList(mResultSet,mResultSetType))
      initIterator();
    else
      mFlagInitialize = false;      
  }  

  return mFlagInitialize;
}

void RunSetIterator::showList()
{
  cout << "RunSetIterator Lists ========\n";
  
  while(hasNext())
  {
    LabelablePtr tpla = getNext();

    string tRelDir,tFname,tRelPath;
    tRelDir = tpla->sub.path.directory.relativePath;
    tFname = tpla->sub.path.filename;
    if(tRelDir.compare("")==0)
      tRelPath = tFname; 
    else
      tRelPath = tRelDir + "/" + tFname; 
    cout << "Item: " << tRelPath << '\n';
  }
}

bool RunSetIterator::matchPurpose(int origIdx)
{
  Label lab = mResultSet.results[origIdx].original->lab;
  std::string pname = getPurposeName(mConstraintPurpose);
  if (pname.compare("any") != 0 && pname.compare("unpurposed") != 0)
  { // only return labelables that match the contraint purpose
      if (lab.hasLabel && pname.compare(lab.name) == 0)
        return true;
      else
        return false;
  }else
      return true;
}

bool RunSetIterator::isConstrained(int origIdx, LabelablePtr lptr)
{
  if (matchPurpose(origIdx) == false)
      return true;
  // If we have a video then make sure its not lost or occluded if that is in the constrants
  if (mLost == false && mOccluded == false)
      return false;
  if (mResultSet.results[origIdx].original->sub.isVideo == false)
      return false;
  if (lptr->lab.name.empty())
      return false;
  int frameNum = atoi(lptr->lab.name.c_str());
  if (frameNum > -1)
  {// Find this frame and see if its lost or occluded
     LabeledTrackPtr origtptr = LabeledTrackPtr::dynamicCast(mResultSet.results[origIdx].original);
     if (origtptr.get() != NULL)
     { // We have a valid LabeledTrackPtr so lets see if this frame is lost or occluded
        FrameLocationList::iterator it;
        FrameLocationList frames = origtptr->keyframesLocations;
        for (it = frames.begin(); it != frames.end(); it++)
        {
            FrameLocation floc = *it;
            if (floc.frame.framecnt == frameNum)
            {
                if (floc.occluded && mOccluded)
                    return true;
                if (floc.outOfFrame && mLost)
                    return true;
                return false;
            }
        }
     }
   }
   return false;
}
  
bool RunSetIterator::hasNext()
{
  if(mListItr != mList.end())
  {
    while (isConstrained(*mListOrginalIdxItr, *mListItr))
    {
        mListItr++;
        mListOrginalIdxItr++;
        if (mListItr == mList.end())
            return false;
    }
    return true;
  }else
  {
    return false;
  }
}

LabelablePtr RunSetIterator::getNext()
{
  if(hasNext())
  {
    LabelablePtr lptr = *mListItr;
    int idx = *mListOrginalIdxItr;
    mListOrginalIdxItr++;
    mListItr++;
    while (isConstrained(idx, lptr))
    {
        if (mListItr == mList.end())
        {
            localAndClientMsg(VLogger::WARN, NULL,
               "There is no more elements.\n");
            return NULL;
        }
        lptr = *mListItr;
        idx = *mListOrginalIdxItr;
        mListOrginalIdxItr++;
        mListItr++;
    }
    return lptr;
  }
  else
  {
    localAndClientMsg(VLogger::WARN, NULL,
      "There is no more elements.\n");
    return NULL;
  }
}

Result& RunSetIterator::getCurrentResult()
{  
  return mResultSet.results[*(mListOrginalIdxItr-1)];
}

ResultSet& RunSetIterator::getResultSet()
{
  return mResultSet;
}


void RunSetIterator::makeConversionList()
{  
  //assuming openCV
  const std::string _supportedImage[] = {"bmp","dib","jpeg","jpg","jpe","jp2",
                                         "png","pbm","pgm","ppm","sr","ras",
                                         "tiff","tif"};

  //assuming openCV
  const std::string _supportedVideo[] = {"mpg","mpeg"};

  vector<string> typeImage;
  for(int k = 0;k<(sizeof(_supportedImage)/sizeof(_supportedImage[0]));k++)
    typeImage.push_back(_supportedImage[k]);

  vector<string> typeVideo;
  for(int k = 0;k<(sizeof(_supportedVideo)/sizeof(_supportedVideo[0]));k++)
    typeVideo.push_back(_supportedVideo[k]);
  
  unsigned int i,j;
  for(i=0;i<typeImage.size();i++)
  {
    for(j=0;j<typeImage.size();j++)
    {
	  if(i==j)
	    continue;
	  else
	  {
	    string msg = typeImage[i] + "2" + typeImage[j];
		mConvertible[msg.c_str()] = mConv_openCV_i2i;
	  }
    }
    
    for(j=0;j<typeVideo.size();j++)
    {
	  string msg = typeVideo[j] + "2" + typeImage[i];
	  mConvertible[msg.c_str()] = mConv_openCV_v2i;
	}
  }
}

