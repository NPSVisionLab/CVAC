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
#include <Data.h>
#include <Services.h>
#ifdef WIN32
#include <util/wdirent.h>
#else
#include <dirent.h>
#endif
#include <util/processRunSet.h>
#include <util/ServiceMan.h>
#include <Ice/Ice.h>
using namespace Ice;
using namespace cvac;

//===========================================================================

class DirectoryWalker
{
private:
    DetectorPtr _detect;
    const DetectorCallbackHandlerPrx &_callback;
    DoDetectFunc _detectFunc;
    ServiceManager *_serviceMan;
public:
    DirectoryWalker(DetectorPtr detect, DoDetectFunc detectFunc,
                   const DetectorCallbackHandlerPrx &callback,
                   ServiceManager *sman);
    void walk(const char *fname, const char* suffixes[], bool recursive);

};
//---------------------------------------------------------------------------

DirectoryWalker::DirectoryWalker(DetectorPtr detect, DoDetectFunc detectFunc,
                                 const DetectorCallbackHandlerPrx &callback,
                                 ServiceManager *sMan)
   : _callback(callback)
{
    _detect = detect;
    _detectFunc = detectFunc;
    _serviceMan = sMan;
}
//---------------------------------------------------------------------------

void DirectoryWalker::walk(const char* filename, const char* suffixes[],
                           bool recursive)
{
    struct dirent *walker;
    DIR *dir;
    char tempName[1024];


    dir = opendir(filename);
    if (dir == NULL)
        return;

    while ((walker = readdir(dir)) != NULL)
    {

        if (_serviceMan->stopRequested())
        {
            _serviceMan->stopCompleted();
            break;
        }
        if (strcmp(walker->d_name, "..") == 0 ||
            strcmp(walker->d_name, ".") == 0)
            continue;
        if (walker->d_type == DT_DIR && recursive)
        {
            strcpy(tempName, filename);
            strcat(tempName, "/");
            strcat(tempName, walker->d_name);
            walk(tempName, suffixes, recursive);

        }else if (walker->d_type == DT_REG)
        {
            int i = 0;
            const char *nextSuffix = suffixes[i++];
            while (nextSuffix != NULL)
            {
                if (strstr(walker->d_name, nextSuffix) != 0)
                {
                    strcpy(tempName, filename);
                    strcat(tempName, "/");
                    strcat(tempName, walker->d_name);
                    Labelable *label = new Labelable();  // Ice will clean up so don't delete
                    label->sub.path.filename = std::string(walker->d_name);
                    label->sub.path.directory.relativePath = std::string(filename);

                    ResultSetV2 res = (*_detectFunc)(_detect, tempName);
                    // put the "original" label into the result set
                    for (unsigned int idx=0; idx<res.results.size(); idx++)
                    {
                         res.results[idx].original = label;
                    }
                    _callback->foundNewResults(res);
                }
                nextSuffix = suffixes[i++];
            }
        }
    }
}

/*
// Check for any chars within the string that make the file inappropriate for the detector
static bool shouldIgnore(FilePath filePath) {
  
  std::string ignoreToken = ".svn";
  unsigned result = filePath.directory.relativePath.find(ignoreToken);
  bool ignoreFilePath = (result != std::string::npos);

  if(ignoreFilePath) {
    localAndClientMsg(VLogger::DEBUG_2, NULL, "Ignoring RunSet file matching ignore-substring: '.svn'");
  }

  return(ignoreFilePath);
}
*/

// Check for any chars within the string that could upset the detector
// Currently, OpenCv based detectors won't accept spaces
bool cvac::containsIllegalChars(FilePath filePath) {

  if(-1 == filePath.filename.find(' '))
  {
    if (-1 == filePath.directory.relativePath.find(' '))
        return(false);
    else
        return(true);
  } else
    return(true);
}

//===========================================================================

void cvac::processRunSet(DetectorPtr detector,
                         const DetectorCallbackHandlerPrx &client,
                         DoDetectFunc detectFunc, const RunSet &run,
                         const std::string &pathPrefix,
                         ServiceManager *sman)
{
    DirectoryWalker walker(detector, detectFunc, client, sman);
    // Fetch a temp directory to store symbolic links as needed
    std::string dir = getCurrentWorkingDirectory();
    localAndClientMsg(VLogger::DEBUG_2, client, "processRunSet-cwd: %s\n", dir.c_str());
    localAndClientMsg(VLogger::DEBUG_2, client, "processRunSet-prefix: %s\n", pathPrefix.c_str());
#ifdef WIN32
    char *tempName = _tempnam(dir.c_str(), NULL);
#else
    char *tempName = tempnam(dir.c_str(), NULL);
#endif /* WIN32 */
    std::string tempString = tempName;
    sman->setStoppable();

    // Step through the Runset doing a detect for each item
    std::vector<PurposedListPtr>::iterator it;
    std::vector<PurposedListPtr> plist = run.purposedLists;
    for (it = plist.begin(); it < plist.end(); it++)
    {
        
        PurposedListPtr p = (*it);
        PurposedLabelableSeqPtr sp =
                 PurposedLabelableSeqPtr::dynamicCast(p);
        PurposedDirectoryPtr dirptr =
                 PurposedDirectoryPtr::dynamicCast(p);

        if (sp)
        {
            std::vector<LabelablePtr>::iterator lIt;
            std::vector<LabelablePtr> llist = sp->labeledArtifacts;
            for (lIt = llist.begin(); lIt < llist.end(); lIt++)
            {
                if (sman->stopRequested())
                {
                    sman->stopCompleted();
                    break;
                }
                LabelablePtr lptr = *lIt;
                Substrate sub = lptr->sub;
                FilePath  filePath = sub.path;
                std::string fname;
                // If path is already absolute then leave it alone else
                // lets prepend our pathPrefix
                if ((filePath.directory.relativePath.length() > 1 && filePath.directory.relativePath[1] == ':' )||
                    filePath.directory.relativePath[0] == '/' ||
                    filePath.directory.relativePath[0] == '\\')
                {  // absolute path
                    fname = filePath.directory.relativePath;
                    localAndClientMsg(VLogger::WARN, NULL,
                                      "Labelable using absolute paths; ignoring file: %s\n", fname.c_str());
                    continue;
                } else { 
                    // prepend our prefix in a way visible to symlink filePath
                    // symbolic links require full paths for the targets
                    std::string cwd = getCurrentWorkingDirectory();
                    std::string abspath = cwd.c_str();
                    abspath += "/" + pathPrefix + "/" + filePath.directory.relativePath;
                    fname = abspath;
                    
                    localAndClientMsg(VLogger::DEBUG_2, NULL, "Labelable using relative paths\n");
                }

                fname += std::string("/");
                fname += filePath.filename;
                // Display full path to file entry, before symlink
                localAndClientMsg(VLogger::DEBUG_1, NULL, "RunSet-labelable filepath: %s\n", fname.c_str());


                // symlink needed?
                bool newSymlink, callSuccess;
                std::string symlinkFullPath = getLegalPath(tempString, filePath, newSymlink);

                if(newSymlink) { // File might have not needed any change
                  callSuccess = makeSymlinkFile(symlinkFullPath, fname);
                
                  if(!callSuccess) {
                      symlinkFullPath = fname;  // put the orig name back so it can fail.
                  }
                }
                

                // 'result' is a vector of result objects
                ResultSetV2 result = (*detectFunc)(detector, symlinkFullPath.c_str());

                // put the "original" label into the result set
                for (unsigned int idx=0; idx<result.results.size(); idx++)
                {
                  result.results[idx].original = lptr;
                  // printf("inserted original: %s\n", result.results[idx].original->lab.name.c_str());
                }
                
                // If we used a symbolic link, delete it
                if (newSymlink)
                {
                    deleteDirectory(tempString);
                }

                client->foundNewResults(result);
            }
        }else if (dirptr)
        { // a directory
            std::vector<std::string>::iterator sIt;
            int len = dirptr->fileSuffixes.size();
            const char **suffixes = new const char*[len];
            int i = 0;
            for (sIt = dirptr->fileSuffixes.begin();
                 sIt < dirptr->fileSuffixes.end(); sIt++)
            {
                suffixes[i++] = (*sIt).c_str();
            }
            std::string fname;
            suffixes[i] = NULL;
            // If path is already absolute then leave it alone else
            // lets prepend our pathPrefix
            if ((dirptr->directory.relativePath.length() > 1 && dirptr->directory.relativePath[1] == ':' )||
                dirptr->directory.relativePath[0] == '/' ||
                dirptr->directory.relativePath[0] == '\\')
            {  // absolute path
                fname = dirptr->directory.relativePath;
                localAndClientMsg(VLogger::WARN, NULL,
                                  "Labelable using absolute paths; ignoring file: %s\n", fname.c_str());
                continue;
            } else { // prepend our prefix

                fname = (pathPrefix + "/" + dirptr->directory.relativePath);

                // Display full relative-path to file to debug broken paths
                localAndClientMsg(VLogger::DEBUG_2, client, "RunSet 'dirptr': %s\n", fname.c_str());
            }

            walker.walk(fname.c_str(), 
                        suffixes,
                        dirptr->recursive);
        }
    }
    sman->clearStop();
    // The client should create a class derived from Ice::Shared to be get the completion
    // callback.  http://doc.zeroc.com/pages/viewpage.action?pageId=13173000
    //client->completedProcessing();
}

// Utility method for Unit Tests and Test Clients to add a new sample to the RunSet data structure
void cvac::addToRunSet( RunSet& runSet, const std::string& relativePath,
                        const std::string& filename, const Purpose& purpose,LocationPtr loc)
{
   PurposedLabelableSeq* purposeClass1 = NULL;

   for (size_t i=0; i<runSet.purposedLists.size(); i++)
   {
      if ( compatiblePurpose( runSet.purposedLists[i]->pur, purpose ))
      {
         purposeClass1 = static_cast<PurposedLabelableSeq*>(runSet.purposedLists[i].get());
         break;
      }
   }
   
   if (purposeClass1 == NULL)
   {
      purposeClass1 = new PurposedLabelableSeq();
      purposeClass1->pur.ptype = purpose.ptype;
      purposeClass1->pur.classID = purpose.classID;
      runSet.purposedLists.push_back(purposeClass1);
   }

   if(loc.get() == NULL)
   {
	   Labelable* class1Label = new Labelable();
	   class1Label->sub.isImage = true; class1Label->sub.isVideo = false;
	   class1Label->sub.path.filename = filename;
	   class1Label->sub.path.directory.relativePath = relativePath;
	   purposeClass1->labeledArtifacts.push_back(class1Label);
   }
   else
   {
	   LabeledLocation* class1Label = new LabeledLocation();
	   class1Label->sub.isImage = true; 
	   class1Label->sub.isVideo = false;
	   class1Label->sub.path.filename = filename;
	   class1Label->sub.path.directory.relativePath = relativePath;	
	   class1Label->loc = loc;	
	   purposeClass1->labeledArtifacts.push_back(class1Label);
   }
}

void cvac::addToRunSet( RunSet& runSet, const std::string& relativePath,
                  const std::string& filename, int classID,LocationPtr loc)
{
	Purpose purpose;
	purpose.ptype = MULTICLASS;
	purpose.classID = classID;
	if(loc.get() == NULL)
		addToRunSet( runSet, relativePath, filename, purpose );
	else
		addToRunSet( runSet, relativePath, filename, purpose, loc);
}



/** replaces space with underscore */
struct replace_functor  // Replacement rules
{
void operator()(char& c) { if(c == ' ') c = '_'; }
};

/** replace space with underscore in illegalFileName and return result 
 */
std::string cvac::getSymlinkSubstitution(const std::string& illegalFileName) {
  
  // Grab a copy of input path and replace illegal characters within it
  std::string replacedFileName = std::string(illegalFileName);
  std::for_each(replacedFileName.begin(), replacedFileName.end(), replace_functor());
  
  return(std::string(replacedFileName));
}

/**
 * Insure that the directories exist and that they don't have any illegal chars
 */
static std::string _makeDirectories(std::string tempName, DirectoryPath dirPath)
{ 
    std::string result = std::string(tempName);
    if (dirPath.relativePath.empty())
        return result;
    int lastIdx = 0;
#ifdef WIN32
    bool backSlash = false;
    int idx = dirPath.relativePath.find(':', 1);
    if (idx != -1)
    {
        // We have a drive letter, lets ignore this
        lastIdx = idx + 1;
    }
    // We need to support both forward and backward slashes
    idx = dirPath.relativePath.find('\\', 0);
    if (idx != -1)
    { // We assume we have backslashes in the path
        backSlash = true;
        if (dirPath.relativePath[lastIdx] == '\\')
             lastIdx++;   // ignore a first backslash
        idx = dirPath.relativePath.find('\\', lastIdx);
    }else
    {
        if (dirPath.relativePath[lastIdx] == '/')
             lastIdx++;   // ignore a first slash
        idx =  dirPath.relativePath.find('/', lastIdx);
    }
#else
    if (dirPath.relativePath[lastIdx] == '/')
         lastIdx++;   // ignore a first slash
    int idx =  dirPath.relativePath.find('/', lastIdx);
#endif /* WIN32 */
    
    while (idx != -1)
    {
        std::string substr = dirPath.relativePath.substr(lastIdx, idx - lastIdx); 
        std::string next = getSymlinkSubstitution(substr);
        result += "/";
        result += next;
        makeDirectory(result);
        lastIdx = idx+1;
#ifdef WIN32
        if (backSlash)
            idx = dirPath.relativePath.find('\\', lastIdx);
        else
            idx = dirPath.relativePath.find('/', lastIdx);
#else
        idx = dirPath.relativePath.find('/', lastIdx);
#endif /* WIN32 */
    }
    int len = dirPath.relativePath.length();
    if (lastIdx + 1 < len)
    { // We have a directory at the end
        
        std::string last = dirPath.relativePath.substr(lastIdx, len - lastIdx);
        std::string lastdir = getSymlinkSubstitution(last);
        result += "/";
        result += lastdir;
        makeDirectory(result);
    }
    return result;
}

/** Return path of a new symlink if fname contains illegal chars, or original path otherwise.
 *  The map between old and new names is managed directly by clients outside this function.
 *  If links need to be created, they'll be put into putLinksDir,
 *  which will be created on demand.
 */
std::string cvac::getLegalPath(std::string putLinksDir, FilePath filePath, bool &newSymlink) {

  //if(!shouldIgnore(filePath)) {  // No detection or symlink on these patterns

    if(containsIllegalChars(filePath)) {

      // We have a illegal character so we need to make a symbolic link using the tempName as the root directory.
      // To insure that we don't have any conflicts we will follow the same directory structure but use the
      // tempname as the root dir.
      makeDirectory(putLinksDir);
      std::string directories = _makeDirectories(putLinksDir, filePath.directory);
      newSymlink = true;
      std::string fullName = directories + "/" + filePath.filename;
      std::string linkFileName_withPath = getSymlinkSubstitution(fullName);  // cvac procedure to make symlink
      return linkFileName_withPath;
    }

    else {
      newSymlink = false;
      return filePath.directory.relativePath + '/' + filePath.filename;
    }
  //}
}

/**
 * Fixes the RunSet files so they don't contain spaces.
 * Returns the directory of symbolic links created.  This will need to be deleted after the run set is used.
 */
std::string cvac::fixupRunSet(RunSet &run, const std::string &CVAC_DataDir)
{
    std::string tempString = getTempFilename(CVAC_DataDir);
   
    // Step through the Runset changing bad file names.
    std::vector<PurposedListPtr>::iterator it;
    std::vector<PurposedListPtr> plist = run.purposedLists;
    for (it = plist.begin(); it < plist.end(); it++)
    {
        PurposedListPtr p = (*it);
        PurposedLabelableSeqPtr sp =
                 PurposedLabelableSeqPtr::dynamicCast(p);
        PurposedDirectoryPtr dirptr =
                 PurposedDirectoryPtr::dynamicCast(p);

        if (sp)
        {
            std::vector<LabelablePtr>::iterator lIt;
            std::vector<LabelablePtr> llist = sp->labeledArtifacts;
            for (lIt = llist.begin(); lIt < llist.end(); lIt++)
            {
                LabelablePtr lptr = *lIt;
                Substrate sub = lptr->sub;
                FilePath  filePath = sub.path;
                std::string cwd = getCurrentWorkingDirectory();
                std::string fname = cwd.c_str(); 
                fname +=  "/";
                fname += filePath.directory.relativePath;
                fname += std::string("/");
                fname += filePath.filename;
                // symlink needed?
                bool newSymlink, callSuccess;
                std::string symlinkFullPath = getLegalPath(tempString, filePath, newSymlink);

                if(newSymlink) { // File might have not needed any change
                  callSuccess = makeSymlinkFile(symlinkFullPath, fname);
                
                  if(callSuccess) {
                      // change run set entry to point to new symbolic link
                      // first break up the symbolic link into relativePath and filename
                      int len = symlinkFullPath.length();
                      int idx = symlinkFullPath.find_last_of('/');
                      if (idx == -1)
                      { // No slashes
                          lptr->sub.path.filename = symlinkFullPath;
                      }else
                      {
                          lptr->sub.path.filename = symlinkFullPath.substr(idx+1, len - idx);
                          lptr->sub.path.directory.relativePath = symlinkFullPath.substr(0, idx);
                      }
                  }
                }
            }
        }
    }
    return tempString;
}
///////////////////////////////////////////////////////////////////////////////
std::string cvac::getClientName(const Ice::Current &cur)
{
    std::string res = "localhost";
    std::string cstring = cur.con->toString();
    if (cstring.empty())
       return res;
    else
    {
        char local[256];
        char remote[256];
        int rval = sscanf(cstring.c_str(), "local address = %[^' '] address = %[^':']", local, remote);
        if (rval == 2)
        {
            std::string rstr = remote;
            // replace dots with underscores
            std::replace(rstr.begin(), rstr.end(), '.','_');
            return rstr; 
        }
    }
     return res;
}