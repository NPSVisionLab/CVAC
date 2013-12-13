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

using namespace Ice;
using namespace cvac;


RunSetIterator::RunSetIterator(RunSetWrapper* _rsw,RunSetConstraint& _cons,
                               ServiceManager *_sman,
                               int _nSkipFrames)
{ 
  mFlagInitialize = false;
  mServiceMan = _sman;

  mMediaTempDirectory = "";  
  if(_rsw != NULL)
  {
    mMediaRootDirectory = _rsw->getRootDir();
    mResultSet = _rsw->getResultSet();
    mResultSetType = _rsw->getResultSetType();
    assert(mResultSet.results.size() == mResultSetType.size());
  }      

  mConstraintType = _cons.mimeTypes;

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

  mList.push_back(_pla);  
  mListOrginalIdx.push_back(_originalIdx);
}

//---------------------------------------------------------------------------
bool RunSetIterator::makeList(ResultSet& _resultSet,
                              vector<rsMediaType>& _resultSetType)
{
  clear();

  string tAbsPath,tRelDir,tFname;

  bool tFlagMakeDirectory = true;
  srand((unsigned)time(NULL));
  std::ostringstream tConvNum2Str;
  tConvNum2Str << rand();
  mMediaTempDirectory = "converted_" + tConvNum2Str.str();
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
      bool tFlagConversion = false;
      mConstraintTypeItr=mConstraintType.begin();
      for(;mConstraintTypeItr!=mConstraintType.end();mConstraintTypeItr++)
      {         
        mConvertibleItr = mConvertible.find(_resultSetType[k] + "2" + (*mConstraintTypeItr));
        if(mConvertibleItr!=mConvertible.end())
        {
          if(tFlagMakeDirectory)
          {
            makeDirectory(mMediaRootDirectory + "/" + mMediaTempDirectory);
            tFlagMakeDirectory = false;
          }

          if(!convertAndAddToList(_resultSet.results[k].original,(*mConstraintTypeItr),
            mConvertibleItr->second,mMediaTempDirectory,k))
            return false;

          tFlagConversion = true;
          break;  //Should be
        } 
      }

      if(!tFlagConversion)
      {
        localAndClientMsg(VLogger::WARN, NULL,
          "No conversion: %s\n",(_resultSet.results[k].original)->sub.path.filename.c_str());
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


bool RunSetIterator::convertAndAddToList(const LabelablePtr& _pla,
                                         const rsMediaType& _targerType,
                                         MediaConverter* _pConv,
                                         const string& _rDirTemp,int _originalIdx)
{
  string tFileNameOld = _pla->sub.path.filename;
  string tFileNameNew = tFileNameOld + "." + _targerType;
  string tRDirOld = _pla->sub.path.directory.relativePath;
  string tRDirNew = _rDirTemp;

  string tAbsPathOld = mMediaRootDirectory + "/" + tRDirOld + "/" + tFileNameOld;
  string tAbsDirNew = mMediaRootDirectory + "/" + tRDirNew;
  string tAbsPathNew = tAbsDirNew + "/" + tFileNameNew;

  vector<string> tListFileName;
  vector<string> tAuxInfo;
  if(_pConv->convert(tAbsPathOld,tAbsDirNew,tFileNameNew,tListFileName,tAuxInfo))
  {
    vector<string>::iterator tItrFilename = tListFileName.begin();
    for(int _idx=0;tItrFilename!=tListFileName.end();tItrFilename++,_idx++)
    {
      if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
      {        
        mServiceMan->stopCompleted();
        return false;
      }

      LabelablePtr _la = new Labelable();
      _la->sub.isImage = true;
      _la->sub.isVideo = !(_la->sub.isImage);
      _la->sub.path.filename = (*tItrFilename);
      _la->sub.path.directory.relativePath = tRDirNew;

      if(!tAuxInfo.empty())
      {
        _la->lab.hasLabel = false;  
        _la->lab.name = tAuxInfo[_idx];
      }
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

bool RunSetIterator::hasNext()
{
  if(mListItr != mList.end())
    return true;
  else
    return false;
}

LabelablePtr RunSetIterator::getNext()
{
  if(hasNext())
  {
    mListOrginalIdxItr++;
    return (*(mListItr++));
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
  mConvertible["bmp2dib"] = mConv_openCV_i2i;
  mConvertible["bmp2jpeg"] = mConv_openCV_i2i;
  mConvertible["bmp2jpg"] = mConv_openCV_i2i;
  mConvertible["bmp2jpe"] = mConv_openCV_i2i;
  mConvertible["bmp2jp2"] = mConv_openCV_i2i;
  mConvertible["bmp2png"] = mConv_openCV_i2i;
  mConvertible["bmp2pbm"] = mConv_openCV_i2i;
  mConvertible["bmp2pgm"] = mConv_openCV_i2i;
  mConvertible["bmp2ppm"] = mConv_openCV_i2i;
  mConvertible["bmp2sr"] = mConv_openCV_i2i;
  mConvertible["bmp2ras"] = mConv_openCV_i2i;
  mConvertible["bmp2tiff"] = mConv_openCV_i2i;
  mConvertible["bmp2tif"] = mConv_openCV_i2i;
  mConvertible["dib2bmp"] = mConv_openCV_i2i;
  mConvertible["dib2jpeg"] = mConv_openCV_i2i;
  mConvertible["dib2jpg"] = mConv_openCV_i2i;
  mConvertible["dib2jpe"] = mConv_openCV_i2i;
  mConvertible["dib2jp2"] = mConv_openCV_i2i;
  mConvertible["dib2png"] = mConv_openCV_i2i;
  mConvertible["dib2pbm"] = mConv_openCV_i2i;
  mConvertible["dib2pgm"] = mConv_openCV_i2i;
  mConvertible["dib2ppm"] = mConv_openCV_i2i;
  mConvertible["dib2sr"] = mConv_openCV_i2i;
  mConvertible["dib2ras"] = mConv_openCV_i2i;
  mConvertible["dib2tiff"] = mConv_openCV_i2i;
  mConvertible["dib2tif"] = mConv_openCV_i2i;
  mConvertible["jpeg2bmp"] = mConv_openCV_i2i;
  mConvertible["jpeg2dib"] = mConv_openCV_i2i;
  mConvertible["jpeg2jpg"] = mConv_openCV_i2i;
  mConvertible["jpeg2jpe"] = mConv_openCV_i2i;
  mConvertible["jpeg2jp2"] = mConv_openCV_i2i;
  mConvertible["jpeg2png"] = mConv_openCV_i2i;
  mConvertible["jpeg2pbm"] = mConv_openCV_i2i;
  mConvertible["jpeg2pgm"] = mConv_openCV_i2i;
  mConvertible["jpeg2ppm"] = mConv_openCV_i2i;
  mConvertible["jpeg2sr"] = mConv_openCV_i2i;
  mConvertible["jpeg2ras"] = mConv_openCV_i2i;
  mConvertible["jpeg2tiff"] = mConv_openCV_i2i;
  mConvertible["jpeg2tif"] = mConv_openCV_i2i;
  mConvertible["jpg2bmp"] = mConv_openCV_i2i;
  mConvertible["jpg2dib"] = mConv_openCV_i2i;
  mConvertible["jpg2jpeg"] = mConv_openCV_i2i;
  mConvertible["jpg2jpe"] = mConv_openCV_i2i;
  mConvertible["jpg2jp2"] = mConv_openCV_i2i;
  mConvertible["jpg2png"] = mConv_openCV_i2i;
  mConvertible["jpg2pbm"] = mConv_openCV_i2i;
  mConvertible["jpg2pgm"] = mConv_openCV_i2i;
  mConvertible["jpg2ppm"] = mConv_openCV_i2i;
  mConvertible["jpg2sr"] = mConv_openCV_i2i;
  mConvertible["jpg2ras"] = mConv_openCV_i2i;
  mConvertible["jpg2tiff"] = mConv_openCV_i2i;
  mConvertible["jpg2tif"] = mConv_openCV_i2i;
  mConvertible["jpe2bmp"] = mConv_openCV_i2i;
  mConvertible["jpe2dib"] = mConv_openCV_i2i;
  mConvertible["jpe2jpeg"] = mConv_openCV_i2i;
  mConvertible["jpe2jpg"] = mConv_openCV_i2i;
  mConvertible["jpe2jp2"] = mConv_openCV_i2i;
  mConvertible["jpe2png"] = mConv_openCV_i2i;
  mConvertible["jpe2pbm"] = mConv_openCV_i2i;
  mConvertible["jpe2pgm"] = mConv_openCV_i2i;
  mConvertible["jpe2ppm"] = mConv_openCV_i2i;
  mConvertible["jpe2sr"] = mConv_openCV_i2i;
  mConvertible["jpe2ras"] = mConv_openCV_i2i;
  mConvertible["jpe2tiff"] = mConv_openCV_i2i;
  mConvertible["jpe2tif"] = mConv_openCV_i2i;
  mConvertible["jp22bmp"] = mConv_openCV_i2i;
  mConvertible["jp22dib"] = mConv_openCV_i2i;
  mConvertible["jp22jpeg"] = mConv_openCV_i2i;
  mConvertible["jp22jpg"] = mConv_openCV_i2i;
  mConvertible["jp22jpe"] = mConv_openCV_i2i;
  mConvertible["jp22png"] = mConv_openCV_i2i;
  mConvertible["jp22pbm"] = mConv_openCV_i2i;
  mConvertible["jp22pgm"] = mConv_openCV_i2i;
  mConvertible["jp22ppm"] = mConv_openCV_i2i;
  mConvertible["jp22sr"] = mConv_openCV_i2i;
  mConvertible["jp22ras"] = mConv_openCV_i2i;
  mConvertible["jp22tiff"] = mConv_openCV_i2i;
  mConvertible["jp22tif"] = mConv_openCV_i2i;
  mConvertible["png2bmp"] = mConv_openCV_i2i;
  mConvertible["png2dib"] = mConv_openCV_i2i;
  mConvertible["png2jpeg"] = mConv_openCV_i2i;
  mConvertible["png2jpg"] = mConv_openCV_i2i;
  mConvertible["png2jpe"] = mConv_openCV_i2i;
  mConvertible["png2jp2"] = mConv_openCV_i2i;
  mConvertible["png2pbm"] = mConv_openCV_i2i;
  mConvertible["png2pgm"] = mConv_openCV_i2i;
  mConvertible["png2ppm"] = mConv_openCV_i2i;
  mConvertible["png2sr"] = mConv_openCV_i2i;
  mConvertible["png2ras"] = mConv_openCV_i2i;
  mConvertible["png2tiff"] = mConv_openCV_i2i;
  mConvertible["png2tif"] = mConv_openCV_i2i;
  mConvertible["pbm2bmp"] = mConv_openCV_i2i;
  mConvertible["pbm2dib"] = mConv_openCV_i2i;
  mConvertible["pbm2jpeg"] = mConv_openCV_i2i;
  mConvertible["pbm2jpg"] = mConv_openCV_i2i;
  mConvertible["pbm2jpe"] = mConv_openCV_i2i;
  mConvertible["pbm2jp2"] = mConv_openCV_i2i;
  mConvertible["pbm2png"] = mConv_openCV_i2i;
  mConvertible["pbm2pgm"] = mConv_openCV_i2i;
  mConvertible["pbm2ppm"] = mConv_openCV_i2i;
  mConvertible["pbm2sr"] = mConv_openCV_i2i;
  mConvertible["pbm2ras"] = mConv_openCV_i2i;
  mConvertible["pbm2tiff"] = mConv_openCV_i2i;
  mConvertible["pbm2tif"] = mConv_openCV_i2i;
  mConvertible["pgm2bmp"] = mConv_openCV_i2i;
  mConvertible["pgm2dib"] = mConv_openCV_i2i;
  mConvertible["pgm2jpeg"] = mConv_openCV_i2i;
  mConvertible["pgm2jpg"] = mConv_openCV_i2i;
  mConvertible["pgm2jpe"] = mConv_openCV_i2i;
  mConvertible["pgm2jp2"] = mConv_openCV_i2i;
  mConvertible["pgm2png"] = mConv_openCV_i2i;
  mConvertible["pgm2pbm"] = mConv_openCV_i2i;
  mConvertible["pgm2ppm"] = mConv_openCV_i2i;
  mConvertible["pgm2sr"] = mConv_openCV_i2i;
  mConvertible["pgm2ras"] = mConv_openCV_i2i;
  mConvertible["pgm2tiff"] = mConv_openCV_i2i;
  mConvertible["pgm2tif"] = mConv_openCV_i2i;
  mConvertible["ppm2bmp"] = mConv_openCV_i2i;
  mConvertible["ppm2dib"] = mConv_openCV_i2i;
  mConvertible["ppm2jpeg"] = mConv_openCV_i2i;
  mConvertible["ppm2jpg"] = mConv_openCV_i2i;
  mConvertible["ppm2jpe"] = mConv_openCV_i2i;
  mConvertible["ppm2jp2"] = mConv_openCV_i2i;
  mConvertible["ppm2png"] = mConv_openCV_i2i;
  mConvertible["ppm2pbm"] = mConv_openCV_i2i;
  mConvertible["ppm2pgm"] = mConv_openCV_i2i;
  mConvertible["ppm2sr"] = mConv_openCV_i2i;
  mConvertible["ppm2ras"] = mConv_openCV_i2i;
  mConvertible["ppm2tiff"] = mConv_openCV_i2i;
  mConvertible["ppm2tif"] = mConv_openCV_i2i;
  mConvertible["sr2bmp"] = mConv_openCV_i2i;
  mConvertible["sr2dib"] = mConv_openCV_i2i;
  mConvertible["sr2jpeg"] = mConv_openCV_i2i;
  mConvertible["sr2jpg"] = mConv_openCV_i2i;
  mConvertible["sr2jpe"] = mConv_openCV_i2i;
  mConvertible["sr2jp2"] = mConv_openCV_i2i;
  mConvertible["sr2png"] = mConv_openCV_i2i;
  mConvertible["sr2pbm"] = mConv_openCV_i2i;
  mConvertible["sr2pgm"] = mConv_openCV_i2i;
  mConvertible["sr2ppm"] = mConv_openCV_i2i;
  mConvertible["sr2ras"] = mConv_openCV_i2i;
  mConvertible["sr2tiff"] = mConv_openCV_i2i;
  mConvertible["sr2tif"] = mConv_openCV_i2i;
  mConvertible["ras2bmp"] = mConv_openCV_i2i;
  mConvertible["ras2dib"] = mConv_openCV_i2i;
  mConvertible["ras2jpeg"] = mConv_openCV_i2i;
  mConvertible["ras2jpg"] = mConv_openCV_i2i;
  mConvertible["ras2jpe"] = mConv_openCV_i2i;
  mConvertible["ras2jp2"] = mConv_openCV_i2i;
  mConvertible["ras2png"] = mConv_openCV_i2i;
  mConvertible["ras2pbm"] = mConv_openCV_i2i;
  mConvertible["ras2pgm"] = mConv_openCV_i2i;
  mConvertible["ras2ppm"] = mConv_openCV_i2i;
  mConvertible["ras2sr"] = mConv_openCV_i2i;
  mConvertible["ras2tiff"] = mConv_openCV_i2i;
  mConvertible["ras2tif"] = mConv_openCV_i2i;
  mConvertible["tiff2bmp"] = mConv_openCV_i2i;
  mConvertible["tiff2dib"] = mConv_openCV_i2i;
  mConvertible["tiff2jpeg"] = mConv_openCV_i2i;
  mConvertible["tiff2jpg"] = mConv_openCV_i2i;
  mConvertible["tiff2jpe"] = mConv_openCV_i2i;
  mConvertible["tiff2jp2"] = mConv_openCV_i2i;
  mConvertible["tiff2png"] = mConv_openCV_i2i;
  mConvertible["tiff2pbm"] = mConv_openCV_i2i;
  mConvertible["tiff2pgm"] = mConv_openCV_i2i;
  mConvertible["tiff2ppm"] = mConv_openCV_i2i;
  mConvertible["tiff2sr"] = mConv_openCV_i2i;
  mConvertible["tiff2ras"] = mConv_openCV_i2i;
  mConvertible["tiff2tif"] = mConv_openCV_i2i;
  mConvertible["tif2bmp"] = mConv_openCV_i2i;
  mConvertible["tif2dib"] = mConv_openCV_i2i;
  mConvertible["tif2jpeg"] = mConv_openCV_i2i;
  mConvertible["tif2jpg"] = mConv_openCV_i2i;
  mConvertible["tif2jpe"] = mConv_openCV_i2i;
  mConvertible["tif2jp2"] = mConv_openCV_i2i;
  mConvertible["tif2png"] = mConv_openCV_i2i;
  mConvertible["tif2pbm"] = mConv_openCV_i2i;
  mConvertible["tif2pgm"] = mConv_openCV_i2i;
  mConvertible["tif2ppm"] = mConv_openCV_i2i;
  mConvertible["tif2sr"] = mConv_openCV_i2i;
  mConvertible["tif2ras"] = mConv_openCV_i2i;
  mConvertible["tif2tiff"] = mConv_openCV_i2i;


  mConvertible["mpg2bmp"] = mConv_openCV_v2i;
  mConvertible["mpg2dib"] = mConv_openCV_v2i;
  mConvertible["mpg2jpeg"] = mConv_openCV_v2i;
  mConvertible["mpg2jpg"] = mConv_openCV_v2i;
  mConvertible["mpg2jpe"] = mConv_openCV_v2i;
  mConvertible["mpg2jp2"] = mConv_openCV_v2i;
  mConvertible["mpg2png"] = mConv_openCV_v2i;
  mConvertible["mpg2pbm"] = mConv_openCV_v2i;
  mConvertible["mpg2pgm"] = mConv_openCV_v2i;
  mConvertible["mpg2sr"] = mConv_openCV_v2i;
  mConvertible["mpg2ras"] = mConv_openCV_v2i;
  mConvertible["mpg2tiff"] = mConv_openCV_v2i;
  mConvertible["mpg2tif"] = mConv_openCV_v2i;

  mConvertible["mpeg2bmp"] = mConv_openCV_v2i;
  mConvertible["mpeg2dib"] = mConv_openCV_v2i;
  mConvertible["mpeg2jpeg"] = mConv_openCV_v2i;
  mConvertible["mpeg2jpg"] = mConv_openCV_v2i;
  mConvertible["mpeg2jpe"] = mConv_openCV_v2i;
  mConvertible["mpeg2jp2"] = mConv_openCV_v2i;
  mConvertible["mpeg2png"] = mConv_openCV_v2i;
  mConvertible["mpeg2pbm"] = mConv_openCV_v2i;
  mConvertible["mpeg2pgm"] = mConv_openCV_v2i;
  mConvertible["mpeg2sr"] = mConv_openCV_v2i;
  mConvertible["mpeg2ras"] = mConv_openCV_v2i;
  mConvertible["mpeg2tiff"] = mConv_openCV_v2i;
  mConvertible["mpeg2tif"] = mConv_openCV_v2i;
}

