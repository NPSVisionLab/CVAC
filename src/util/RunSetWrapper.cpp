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
#include <util/RunSetWrapper.h>

using namespace Ice;
using namespace cvac;

//---------------------------------------------------------------------------
RunSetWrapper::RunSetWrapper(const RunSet* _runset,string _mediaRootPath,
                             ServiceManager *_sman)
{
  mFlagIntialize = false;
  mRunset = NULL;
  mMediaRootPath = "";	
  mServiceMan = NULL;	

  if(_runset)
  {
    mRunset = _runset;
    mMediaRootPath = _mediaRootPath;
    mServiceMan = _sman;

    if(!makeBasicList())
    {
      mRunset = NULL;
      mMediaRootPath = "";        
      mServiceMan = NULL;
      localAndClientMsg(VLogger::WARN, NULL,
        "The runset may contain serious problems.\n");
      mFlagIntialize = false;
    }
    else 
      mFlagIntialize = true;
  }
  else
  {		
    localAndClientMsg(VLogger::WARN, NULL,
    "This runset is empty, and it may cause undesirable or unexpected results.\n");
    mFlagIntialize = false;
  }
}

//---------------------------------------------------------------------------
RunSetWrapper::~RunSetWrapper()
{
	
}


//---------------------------------------------------------------------------
rsMediaType RunSetWrapper::getType(const string _aPath)
{
  string tExt = getFileExtension(_aPath);
  return (rsMediaType)tExt;
}

//---------------------------------------------------------------------------
rsMediaType RunSetWrapper::getType(const LabelablePtr _pla)
{
  rsMediaType tType = "unknown";

  if(!_pla)
  {
    return tType;
  }
  else
  {
    string tAbsFilePath = convertToAbsDirectory(_pla->sub.path.directory.relativePath);
    tAbsFilePath += "/";
    tAbsFilePath += _pla->sub.path.filename;

    return getType(tAbsFilePath);
  }	
}

//---------------------------------------------------------------------------
string RunSetWrapper::convertToAbsDirectory(const string& _directory)
{
  return convertToAbsDirectory(_directory,mMediaRootPath);
}


//---------------------------------------------------------------------------
string RunSetWrapper::convertToAbsDirectory(const string& _directory,
                                            const string& _prefix)
{
  string tNew;
  if(isAbsDirectory(_directory))	// absolute path
    tNew = _directory;
  else
    tNew = _prefix + "/" + _directory;

  while(true)
  {
    if(tNew.empty())
      break;
    
    // removing the last '/' or '\\'
    if( (tNew[tNew.length()-1] == '/') || (tNew[tNew.length()-1] == '\\') )
      tNew = tNew.substr(0,tNew.length()-1);
    else
      break;
  }

  return tNew;	
}

//---------------------------------------------------------------------------
bool RunSetWrapper::isAbsDirectory(const string& _directory)
{
  if( (_directory.length() > 1 && _directory[1] == ':' )||
      _directory[0] == '/' ||
      _directory[0] == '\\')
    return true;
  else
    return false;
}

//---------------------------------------------------------------------------
bool RunSetWrapper::isInRunset(const string&_rDir,const string& _fname,
                               const vector<rsMediaType>& _types,rsMediaType& _resType)
{
  //In future, fileType will be examined by accessing the file itself (not using the extension)
  //So, the directory is remained in this function.
  _resType = getType(convertToAbsDirectory(_rDir) + "/" + _fname);

  if(_types.empty())
    return true;
  else
    return (find(_types.begin(),_types.end(),_resType)==_types.end())?false:true;	
}


//---------------------------------------------------------------------------
void RunSetWrapper::addToList(const LabelablePtr _pla,const rsMediaType _type)
{
  if(!_pla)
  {
    localAndClientMsg(VLogger::WARN, NULL,"Empty LabelablePtr is assigned\n");
    return;
  }

  mList.push_back(_pla);
  mListType.push_back(_type);
}

//---------------------------------------------------------------------------
bool RunSetWrapper::makeBasicList()
{

    std::string dir = getCurrentWorkingDirectory();
#ifdef WIN32
    char *tTempName = _tempnam(dir.c_str(), NULL);
#else
    char *tTempName = tempnam(dir.c_str(), NULL);
#endif /* WIN32 */
    std::string tTempDir = tTempName; //for being used in making a symbolic link


    PurposedLabelableSeqPtr	out_seq = NULL;
    PurposedDirectoryPtr	out_dir = NULL;
    vector<PurposedListPtr>::iterator out_itr;
	
    LabelablePtr			in_la = NULL;
    vector<LabelablePtr>::iterator in_itr;
    
    vector<PurposedListPtr> list_out = mRunset->purposedLists;
    
	for(out_itr=list_out.begin();out_itr!=list_out.end();out_itr++)
    {
      out_seq=PurposedLabelableSeqPtr::dynamicCast(*out_itr);
      out_dir=PurposedDirectoryPtr::dynamicCast(*out_itr);

      if(out_seq)	//PurposedLabelableSeqPtr
      {
        vector<LabelablePtr> list_in = out_seq->labeledArtifacts;
        for(in_itr=list_in.begin();in_itr!=list_in.end();in_itr++)
        {				
          in_la=LabelablePtr::dynamicCast(*in_itr);

          if(in_la)	//default: LabelablePtr
          {	
            string tAbsDir = convertToAbsDirectory(in_la->sub.path.directory.relativePath);
            string tPath = tAbsDir + "/" + in_la->sub.path.filename;
            FilePath tFilePath;
            tFilePath.directory.relativePath = tAbsDir;
            tFilePath.filename = in_la->sub.path.filename;           
            
            //////////////////////////////////////////////////////////////////////////
            // From processRunSet.cpp
            bool newSymlink;
            std::string symlinkFullPath = getLegalPath(tTempDir, tFilePath, newSymlink);            

            if(newSymlink)
            {             
              cout << "symlink will be generated..\n";
              if(!makeSymlinkFile(symlinkFullPath, tPath))
              {
                cout << "But, it's failed..\n";
                symlinkFullPath = tPath;  // put the orig name back so it can fail.
              }
            }
            //////////////////////////////////////////////////////////////////////////
            
            if(fileExists(symlinkFullPath))
              addToList(in_la,getType(symlinkFullPath));
            else	//no file
              continue;					
          }
          else
          {
            localAndClientMsg(VLogger::WARN, NULL,
              "There is no proper type for this runset (Label..).\n");
            return false;
          }
        }
      }
      else if(out_dir)	//PurposedDirectoryPtr
      {
        string tRelativePath = out_dir->directory.relativePath;
        string tAbsPath = convertToAbsDirectory(tRelativePath,mMediaRootPath);			
        if(!makeBasicList_parse(tAbsPath,true,tRelativePath,out_dir->fileSuffixes))
          return false;
      }
      else
      {
        localAndClientMsg(VLogger::WARN, NULL,
          "There is no proper type for this runset (purposed..).\n");	
        return false;
      }
	}   

	return true;
}


//---------------------------------------------------------------------------
bool RunSetWrapper::makeBasicList_parse(const string& _absDir,bool _recursive,
                                   const string& _rDir,const vector<rsMediaType>& _types)
{
    struct dirent *walker;
    DIR *dir;
    string dirNext;

    dir = opendir(_absDir.c_str());
    if (dir == NULL)
    {
      localAndClientMsg(VLogger::WARN, NULL,
        "No directory for the file %s\n",_absDir.c_str());
      return true;
    }

    while ((walker = readdir(dir)) != NULL)
    {
      if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
      {
        mServiceMan->stopCompleted();
        return false;
      }

      if(strcmp(walker->d_name, "..") == 0 ||
        strcmp(walker->d_name, ".") == 0)
        continue;

      if(walker->d_type == DT_DIR && _recursive)
      {
        dirNext = _absDir + "/" + string(walker->d_name);
        string tRDir = _rDir + "/" + string(walker->d_name);
        if(!makeBasicList_parse(dirNext, _recursive,tRDir,_types))
          return false;
      }
      else if (walker->d_type == DT_REG)
      {
        string tfname = string(walker->d_name);
        rsMediaType tType;
        if(isInRunset(_rDir,tfname,_types,tType))
        {
          LabelablePtr tpla = new Labelable();	
          //tpla->sub.isImage = true; 
          //tpla->sub.isVideo = false;
          tpla->sub.path.filename = tfname;
          tpla->sub.path.directory.relativePath = _rDir;
          addToList(tpla,tType);
        }        
      }
    }

    return true;
}

vector<LabelablePtr>& RunSetWrapper::getList()
{
  return mList;
}

vector<rsMediaType>& RunSetWrapper::getListType()
{
  return mListType;
}


void RunSetWrapper::showList()
{
  cout << "RunSetWrapper Lists ========\n";

  vector<LabelablePtr>::iterator tItr = mList.begin();

  LabelablePtr tpla;
  for(;tItr!=mList.end();tItr++)
  {
    tpla = (*tItr);
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


