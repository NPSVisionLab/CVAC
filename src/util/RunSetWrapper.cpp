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

const std::string _typeMacro_Image[] = {"bmp","dib","jpeg","jpg","jpe","jp2",
                                  "png","pbm","pgm","ppm","sr","ras",
                                  "tiff","tif","gif","giff","emf","pct",
                                  "pcx","pic","pix","vga","wmf"};

const std::string _typeMacro_Video[] = {"mpg","mpeg","avi","wmv","wmp","wm",
                                  "asf","mpe","m1v","m2v","mpv2","mp2v",
                                  "dat","ts","tp","tpr","trp","vob","ifo",
                                  "ogm","ogv","mp4","m4v","m4p","m4b","3gp",
                                  "3gpp","3g2","3gp2","mkv","rm","ram",
                                  "rmvb","rpm","flv","swf","mov","qt",
                                  "amr","nsv","dpg","m2ts","m2t","mts",
                                  "k3g","skm","evo","nsr","amv","divx","webm"};

const std::string _typeMacro[] = {"image","video","etc"};

//---------------------------------------------------------------------------
RunSetWrapper::RunSetWrapper(const RunSet* _runset,string _mediaRootPath,
                             ServiceManager *_sman)
{
  mFlagIntialize = false;
  mRunset = NULL;
  mMediaRootPath = "";	
  mServiceMan = NULL;  

  for(int k = 0;k<(sizeof(_typeMacro_Image)/sizeof(_typeMacro_Image[0]));k++)
    mTypeMacro_Image.push_back(_typeMacro_Image[k]);

  for(int k = 0;k<(sizeof(_typeMacro_Video)/sizeof(_typeMacro_Video[0]));k++)
    mTypeMacro_Video.push_back(_typeMacro_Video[k]); 

  for(int k = 0;k<(sizeof(_typeMacro)/sizeof(_typeMacro[0]));k++)
    mTypeMacro.push_back(_typeMacro[k]);

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
rsMediaType RunSetWrapper::getTypeMicro(const string _aPath)
{
  string tExt = getFileExtension(_aPath);  
  return (rsMediaType)tExt;
}

//---------------------------------------------------------------------------
rsMediaType RunSetWrapper::getTypeMicro(const LabelablePtr _pla)
{
  rsMediaType tType = "unknown";

  if(!_pla)
  {
    return tType;
  }
  else
  {
    string tAbsFilePath = getPath(_pla, true);
    return getTypeMicro(tAbsFilePath);
  }	
}
//---------------------------------------------------------------------------
cvac::FilePath RunSetWrapper::getFilePath(const Labelable& lab)
{
 SubstratePtr sub = lab.sub; 
 ImageSubstratePtr isub = ImageSubstratePtr::dynamicCast(sub);
 
 FilePath path;
 if (isub)
 { 
      path = isub->path;
 }else
 {
     VideoSubstratePtr vsub = VideoSubstratePtr::dynamicCast(sub);
     if (vsub)
     {
         path = vsub->videopath;
     } else
     { 
         localAndClientMsg(VLogger::WARN, NULL,
              "Unsupported Substrate type \n");
     }
 }
 return path;
}
//---------------------------------------------------------------------------
cvac::FilePath RunSetWrapper::getFilePath(const LabelablePtr _pla)
{
 SubstratePtr sub = _pla->sub; 
 ImageSubstratePtr isub = ImageSubstratePtr::dynamicCast(sub);
 
 FilePath path;
 if (isub)
 { 
      path = isub->path;
 }else
 {
     VideoSubstratePtr vsub = VideoSubstratePtr::dynamicCast(sub);
     if (vsub)
     {
         path = vsub->videopath;
     } else
     { 
         localAndClientMsg(VLogger::WARN, NULL,
              "Unsupported Substrate type \n");
     }
 }
 return path;
}

bool RunSetWrapper::isVideo(const LabelablePtr _pla)
{
   SubstratePtr sub = _pla->sub; 
   ImageSubstratePtr isub = ImageSubstratePtr::dynamicCast(sub);
   if (isub)
        return false;
   else
        return true;
}

//---------------------------------------------------------------------------
string RunSetWrapper::getPath(const LabelablePtr _pla, bool abs)
{
 FilePath path = getFilePath(_pla);
 
 if (abs)
 {
     string absdir = convertToAbsDirectory(path.directory.relativePath);
     absdir += "/";
     absdir += path.filename;
     return absdir;
  } else
  {
     string reldir = path.directory.relativePath;
     if (reldir.compare("") == 0)
         reldir = path.filename;
     else
     {
         reldir += "/";
         reldir += path.filename;
     }
     return reldir;
  }
}
//---------------------------------------------------------------------------
string RunSetWrapper::getFilename(const LabelablePtr _pla)
{
  FilePath path = getFilePath(_pla);
  return path.filename;
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
  _resType = getTypeMicro(convertToAbsDirectory(_rDir) + "/" + _fname);

  if(_types.empty())
    return true;
  else
    return (find(_types.begin(),_types.end(),_resType)==_types.end())?false:true;	
}


//---------------------------------------------------------------------------
void RunSetWrapper::addToList(const LabelablePtr _pla,const rsMediaType _type,cvac::Purpose purpose)
{
  if(!_pla)
  {
    localAndClientMsg(VLogger::WARN, NULL,"Empty LabelablePtr is assigned\n");
    return;
  }
  
  cvac::Result  _result;
  _result.original = _pla;
  _result.original->lab.hasLabel = true;
  _result.original->lab.name = getPurposeName(purpose);
  mResultSet.results.push_back(_result);
  mResultSetType.push_back(_type);
}

//---------------------------------------------------------------------------
std::string RunSetWrapper::getTypeMacro(const std::string& _path)
{
  string tExt = getFileExtension(_path);   

  vector<string>::iterator _itr;
  _itr = std::find(mTypeMacro_Image.begin(),mTypeMacro_Image.end(),tExt);
  if(_itr!=mTypeMacro_Image.end())
    return (rsMediaType)mTypeMacro[0];  //for image

  _itr = std::find(mTypeMacro_Video.begin(),mTypeMacro_Video.end(),tExt);
  if(_itr!=mTypeMacro_Video.end())
    return (rsMediaType)mTypeMacro[1];  //for video
  else
    return (rsMediaType)mTypeMacro[2];  //for etc
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
      PurposedListPtr p= (*out_itr);

      cvac::Purpose curPurpose = p->pur;
      if (p->ice_isA("::cvac::PurposedLabelableSeq"))
      { // For some reason the dynamicCast is failing on Osx 10.8
        // We do an isA to confirm what it is and then the static cast is safe
          //out_seq = IceInternal::Handle<PurposedLabelableSeq>(dynamic_cast<PurposedLabelableSeq*>(p._ptr));
          out_seq = IceInternal::Handle<PurposedLabelableSeq>(static_cast<PurposedLabelableSeq*>(p._ptr));
      }
      //out_seq=PurposedLabelableSeqPtr::dynamicCast(p);

      if (p->ice_isA("::cvac::PurposedDirectory"))
      { // For some reason the dynamicCast is failing on Osx 10.8
        // We do an isA to confirm what it is and then the static cast is safe
          //out_dir=PurposedDirectoryPtr::dynamicCast(p);
          out_dir = IceInternal::Handle<PurposedDirectory>(static_cast<PurposedDirectory*>(p._ptr));
      }
      if(out_seq)	//PurposedLabelableSeqPtr
      {
        vector<LabelablePtr> list_in = out_seq->labeledArtifacts;
        for(in_itr=list_in.begin();in_itr!=list_in.end();in_itr++)
        {	
          // Dyanmic cast is losing derived type info and is not needed!!		
          //in_la=LabelablePtr::dynamicCast(*in_itr);
          in_la = *in_itr;
          if(in_la)	//default: LabelablePtr
          {	
			
            addToList(in_la,getTypeMicro(getFilename(in_la)), curPurpose);
          
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
        if(!makeBasicList_parse(tAbsPath,true,tRelativePath,out_dir->fileSuffixes, curPurpose))
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
                                   const string& _rDir,const vector<rsMediaType>& _types,
                                   cvac::Purpose purpose)
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
        if(!makeBasicList_parse(dirNext, _recursive,tRDir,_types, purpose))
          return false;
      }
      else if (walker->d_type == DT_REG)
      {
        string tfname = string(walker->d_name);
        rsMediaType tType;
        if(isInRunset(_rDir,tfname,_types,tType))
        {
          LabelablePtr tpla = new Labelable();
          if (tType.compare(mTypeMacro[0]) == 0)
          {
              ImageSubstratePtr isub = new ImageSubstrate();
              isub->path.filename = tfname;
              isub->path.directory.relativePath = _rDir;
              tpla->sub = isub;
          } else if (tType.compare(mTypeMacro[1]) == 0)
          {
              VideoSubstratePtr vsub = new VideoSubstrate();
              vsub->videopath.filename = tfname;
              vsub->videopath.directory.relativePath = _rDir;
              tpla->sub = vsub;
          }else
          {
              localAndClientMsg(VLogger::WARN, NULL,
                              "Not a vaild media type in makeBasicList_parse\n");	
               return false;
          }
          addToList(tpla,tType, purpose);
        }        
      }
    }

    return true;
}

ResultSet& RunSetWrapper::getResultSet()
{
  return mResultSet;
}

vector<rsMediaType>& RunSetWrapper::getResultSetType()
{
  return mResultSetType;
}

void RunSetWrapper::showList()
{
  cout << "RunSetWrapper Lists ========\n";

  LabelablePtr tpla;
  unsigned int i;
  for(i=0;i<mResultSet.results.size();i++)
  {
    tpla = mResultSet.results[i].original;
   
    string tRelPath;
    tRelPath = getPath(tpla, false);
    cout << "Item: " << tRelPath << '\n';
  }
}


