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


RunSetIterator::RunSetIterator(RunSetWrapper& _rsw,Constraints& _cons)
{ 
  mMediaTempDirectory = "";
  mMediaRootDirectory = _rsw.getRootDir();
  vector<LabelablePtr> tSrcList = _rsw.getList();
  vector<rsMediaType> tSrcListType = _rsw.getListType();
  assert(tSrcList.size() == tSrcListType.size());

  mConstraintType = _cons.mineTypes;

  //////////////////////////////////////////////////////////////////////////    
  mConv_openCV_i2i = new MediaConverter_openCV_i2i();
  mConv_openCV_v2i = new MediaConverter_openCV_v2i(100);
  makeConversionList();  
  //////////////////////////////////////////////////////////////////////////  

  if(tSrcList.empty() || mConstraintType.empty())
  {
    localAndClientMsg(VLogger::WARN, NULL,"Empty RunSetWrapper or constraints.\n");
    initIterator(tSrcList);  //as it is
  }    
  else
  {
    makeList(tSrcList,tSrcListType);
    initIterator();
  }
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
}

void RunSetIterator::initIterator(vector<LabelablePtr>& _list)
{
  mList = _list;  
  initIterator();
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
void RunSetIterator::addToList(const LabelablePtr _pla)
{
  if(_pla==NULL)
  {
    localAndClientMsg(VLogger::WARN, NULL,"Empty LabelablePtr is assigned\n");
    return;
  }

  mList.push_back(_pla);
}


//---------------------------------------------------------------------------
bool RunSetIterator::makeList(vector<LabelablePtr>& _srclist,
                              vector<rsMediaType>& _srclistType)
{
  clear();

  rsMediaType tTypeSrc;
  string tAbsPath,tRelDir,tFname;

  bool tFlagMakeDirectory = true;
  srand((unsigned)time(NULL));
  std::ostringstream tConvNum2Str;
  tConvNum2Str << rand();
  mMediaTempDirectory = "converted_" + tConvNum2Str.str();

  vector<LabelablePtr>::iterator tItrSrc = _srclist.begin();
  vector<rsMediaType>::iterator tItrSrcType = _srclistType.begin();
  for(;tItrSrc!=_srclist.end();tItrSrc++,tItrSrcType++)
  {    
    tTypeSrc = (*tItrSrcType);
    if(isInConstraintType(*tItrSrcType))
      addToList(*tItrSrc);
    else 
    { 
      bool tFlagConversion = false;
      mConstraintTypeItr=mConstraintType.begin();
      for(;mConstraintTypeItr!=mConstraintType.end();mConstraintTypeItr++)
      {        
        mConvertibleItr = mConvertible.find(tTypeSrc + "2" + (*mConstraintTypeItr));
        if(mConvertibleItr!=mConvertible.end())
        {
          if(tFlagMakeDirectory)
          {
            makeDirectory(mMediaRootDirectory + "/" + mMediaTempDirectory);
            tFlagMakeDirectory = false;
          }
          convertAndAddToList((*tItrSrc),
            (*mConstraintTypeItr),
            mConvertibleItr->second,
            mMediaTempDirectory);
          tFlagConversion = true;
          break;  //Should be
        } 
      }

      if(!tFlagConversion)
      {
        localAndClientMsg(VLogger::WARN, NULL,
          "No conversion: %s\n",(*tItrSrc)->sub.path.filename.c_str());
      }
    }
  }  

  return true;
}


void RunSetIterator::convertAndAddToList(const LabelablePtr& _pla,
                                         const rsMediaType& _targerType,
                                         MediaConverter* _pConv,
                                         const string& _rDirTemp)
{
  string tFileNameOld = _pla->sub.path.filename;
  string tFileNameNew = tFileNameOld + "." + _targerType;
  string tRDirOld = _pla->sub.path.directory.relativePath;
  string tRDirNew = _rDirTemp;

  string tAbsPathOld = mMediaRootDirectory + "/" + tRDirOld + "/" + tFileNameOld;
  string tAbsDirNew = mMediaRootDirectory + "/" + tRDirNew;
  string tAbsPathNew = tAbsDirNew + "/" + tFileNameNew;

  vector<string> tListFileName;
  if(_pConv->convert(tAbsPathOld,tAbsDirNew,tFileNameNew,tListFileName))
  {
    vector<string>::iterator tItrFilename = tListFileName.begin();
    for(;tItrFilename!=tListFileName.end();tItrFilename++)
    {
      LabelablePtr _la = new Labelable();
      _la->sub.isImage = true;
      _la->sub.isImage = !(_la->sub.isImage);
      _la->sub.path.filename = (*tItrFilename);
      _la->sub.path.directory.relativePath = tRDirNew;
      addToList(_la);
    }    
  }
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
  if(mListItr+1 != mList.end())
    return true;
  else
    return false;
}

LabelablePtr RunSetIterator::getNext()
{
  if(hasNext())
    return (*(++mListItr));
  else
    localAndClientMsg(VLogger::WARN, NULL,
      "There is no more elements.\n");
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
}

