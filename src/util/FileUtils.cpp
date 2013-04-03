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
#include <util/FileUtils.h>
#include <util/Timing.h>
#include <sys/stat.h>

#if defined(WIN32)
   #include <direct.h> //for _mkdir()
   #define stat _stati64
   #define mkdir(x,y) _mkdir((x))
   //#define S_ISREG(m) (((m)&_S_IFREG)!=0)
   //#define S_ISDIR(m) (((m)&_S_IFDIR)!=0)
#endif

#include <stdio.h>  /* defines FILENAME_MAX */

#ifdef WIN32
      #include <direct.h>
      #include <util/wdirent.h>
      #define GetCurrentDir _getcwd
      #include <windows.h>

#else
      #include <unistd.h>
      #define GetCurrentDir getcwd
#endif
using namespace cvac;

static VLogger vLogger();  // Compile-time default base-level

///////////////////////////////////////////////////////////////////////////////
void cvac::localAndClientMsg(VLogger::Levels rqLevel, const ::CallbackHandlerPrx& callbackHandler, const char* fmt, ...) {

  va_list args;
  va_start(args, fmt);
  
  // Echo locally according to config.service property 'CVAC.ServicesVerbosity' in CVAC root
  if(cvac::vLogger.getBaseLevel() >= rqLevel)
  {
    cvac::vLogger.printv(rqLevel, fmt, args);
  }

  // Echo remotely based on client verbosity
  if((cvac::vLogger.getBaseLevel() > cvac::vLogger.getIntLevel(0))) //&&   (Tmp: always echo client messages unless SILENT)
     //(cvac::vLogger.getBaseLevel() >= rqLevel))   // Early pruning of expensive messages  (will use 'clientVerbosity')
  {
    // Echo through callbackHandler if available, assemble string for client message from arglist
    if(0 != callbackHandler) {
     
      char buffer[1024];
      memset(&buffer[0], 0, sizeof(buffer));
      vsprintf(buffer, fmt, args);

      callbackHandler->message(rqLevel, buffer);  // Send to client
    }
    va_end(args);
  }
}

///////////////////////////////////////////////////////////////////////////////
bool cvac::directoryExists(const std::string& directory)
{
   struct stat fileStat;
   if (stat(directory.c_str(), &fileStat) != 0)
   {
      return false;
   }

   if (fileStat.st_mode & S_ISDIR(fileStat.st_mode))
   {
      //found, and is a directory
      return true;
   }
   else
   {
      //found, and not a directory
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
std::string cvac::getFilePath(const std::string& fileName)
{
   std::string::size_type slash1 = fileName.find_last_of('/');
   std::string::size_type slash2 = fileName.find_last_of('\\');
   if (slash1==std::string::npos)
   {
      if (slash2==std::string::npos) return std::string();
      return std::string(fileName,0,slash2);
   }
   if (slash2==std::string::npos) return std::string(fileName,0,slash1);
   return std::string(fileName, 0, slash1>slash2 ?  slash1 : slash2);
}

////////////////////////////////////////////////////////////////////////////////
std::string cvac::getCurrentWorkingDirectory()
{
   std::string currentDir(FILENAME_MAX, '\0');

   if (!GetCurrentDir(&currentDir[0], FILENAME_MAX))
   {
      currentDir = "";
   }

   return currentDir;
}

///////////////////////////////////////////////////////////////////////////////

bool cvac::makeDirectories(const std::string& dirPath)
{ 
    std::string result;
    if (dirPath.empty())
        return false;
    int lastIdx = 0;
#ifdef WIN32
    int idx = dirPath.find(':', 1);
    if (idx != -1)
    {
        // We have a drive letter, lets ignore this
        lastIdx = idx + 1;
    }
    if (dirPath[lastIdx] == '\\')
         lastIdx++;   // ignore a first backslash
    idx = dirPath.find('\\', lastIdx);
    if (idx == -1)
         idx = dirPath.find('/', lastIdx); // try forward slash
#else
    if (dirPath[lastIdx] == '/')
         lastIdx++;   // ignore a first slash
    int idx =  dirPath.find('/', lastIdx);
#endif /* WIN32 */
    std::string substr;
    if (idx > 0)
    {
        std::string substr = dirPath.substr(0, idx - lastIdx);
        if (!makeDirectory(substr))
            return false;    
        result += substr; 
    }
    lastIdx = idx+1;
#ifdef WIN32
    idx = dirPath.find('\\', lastIdx);
    if (idx == -1)
         idx = dirPath.find('/', lastIdx); // try forward slash
#else
    idx = dirPath.find('/', lastIdx);
#endif /* WIN32 */
    while (idx != -1)
    {
        substr = dirPath.substr(lastIdx, idx - lastIdx); 
        result += "/";
        result += substr;
        if (!makeDirectory(result))
            return false;
        lastIdx = idx+1;
#ifdef WIN32
        idx = dirPath.find('\\', lastIdx);
        if (idx == -1)
             idx = dirPath.find('/', lastIdx); // try forward slash
#else
        idx = dirPath.find('/', lastIdx);
#endif /* WIN32 */
    }
    int len = dirPath.length();
    if (lastIdx + 1 < len)
    { // We have a directory at the end
        
        std::string last = dirPath.substr(lastIdx, len - lastIdx);
        result += "/";
        result += last;
        if (!makeDirectory(result))
            return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////
bool cvac::makeDirectory(const std::string& path)
{
   if (path.empty())
   {
      //no path supplied
      return false;
   }

   if (directoryExists(path))
   {
      //already exists
      return true;
   }

   if (mkdir(path.c_str(), 0755) < 0)
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////
std::string cvac::getFileName(const std::string& fileName)
{
   std::string::size_type slash1 = fileName.find_last_of('/');
   std::string::size_type slash2 = fileName.find_last_of('\\');
   if (slash1==std::string::npos)
   {
      if (slash2==std::string::npos) return fileName;
      return std::string(fileName.begin()+slash2+1,fileName.end());
   }
   if (slash2==std::string::npos) return std::string(fileName.begin()+slash1+1,fileName.end());
   return std::string(fileName.begin()+(slash1>slash2?slash1:slash2)+1,fileName.end());
}

///////////////////////////////////////////////////////////////////////////////
std::string cvac::getBaseFileName(const std::string& fileName)
{
   const std::string onlyFileName = getFileName(fileName);

   std::string::size_type dot = onlyFileName.find_first_of('.');
   if (dot==std::string::npos) return onlyFileName;
   return std::string(onlyFileName.begin(),onlyFileName.begin()+dot);
}

// TODO: please add documentation.
// TODO: I am a bit afraid of this - a recursive delete is dangerous.  has anybody checked if this follows symbolic links???
bool cvac::deleteDirectory(const std::string& path) 
#if defined(WIN32)
{
    // first off, we need to create a pointer to a directory
    DIR *pdir = NULL; // remember, it's good practice to initialise a pointer to NULL!
    char file[1024];
    pdir = opendir (path.c_str());
    struct dirent *pent = NULL;
    if (pdir == NULL) { // if pdir wasn't initialised correctly
        return false; // return false to say "we couldn't do it"
    } // end if

    while ((pent = readdir (pdir)) != NULL) { // while there is still something in the directory to list
        if (strcmp(pent->d_name, "..") == 0 ||
            strcmp(pent->d_name, ".") == 0)
            continue;
        if (pent->d_type == DT_DIR)
        {
            strcpy(file, path.c_str());
            strcat(file, "/");
            strcat(file, pent->d_name); // concatenate the strings to get the complete path
            deleteDirectory(file);
        }else if (pent->d_type == DT_REG)
        {
            strcpy(file, path.c_str());
            strcat(file, "/");
            strcat(file, pent->d_name); // concatenate the strings to get the complete path
            remove(file);
        }
    }

    // finally, let's clean up
    closedir (pdir); // close the directory
    if (!_rmdir(path.c_str())) return false; // delete the directory
    return true;
}
#else // not windows
{
    int res = rmdir( path.c_str() );
    return (res==0);  // success --> return true
}
#endif // defined(WIN32)

bool cvac::compatiblePurpose( const Purpose& actual, const Purpose& constraint ) 
{
  if (ANY==constraint.ptype) return true;
  if (actual.ptype!=constraint.ptype) return false;

  if(MULTICLASS==constraint.ptype && MULTICLASS==actual.ptype 
    && actual.classID!=constraint.classID)
    return false;

  return true;
}

void cvac::addFileToRunSet( RunSet& runSet, const std::string& relativePath,
                        const std::string& filename, const Purpose& purpose)
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

   Labelable* class1Label = new Labelable();
   class1Label->sub.isImage = true; class1Label->sub.isVideo = false;
   class1Label->sub.path.filename = filename;
   class1Label->sub.path.directory.relativePath = relativePath;
   purposeClass1->labeledArtifacts.push_back(class1Label);
}

void cvac::addFileToRunSet( RunSet& runSet, const std::string& relativePath,
                            const std::string& filename, int classID)
{
  Purpose purpose;
  purpose.ptype = MULTICLASS;
  purpose.classID = classID;
  addFileToRunSet( runSet, relativePath, filename, purpose );
}

bool cvac::makeSymlinkFile(const std::string linkFullPath, const std::string tgtFile) {

#if defined(WIN32)
  int winReturnCode = CreateSymbolicLink(linkFullPath.c_str(), 
                                         tgtFile.c_str(), 
                                         false);          // File, not directory
  if(0 != winReturnCode)
    return(true);
  else
  {
      int err = GetLastError();
      if (err == 183)
      { // All ready exists error. This means the link already exists so
        // just return true
          return true;      
      }
      if (err == 1314)
      {
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
          fprintf(stderr, "!!!!Admin rights required for creating a symbolic link!!!!\n");   
          fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      }
      printf("failed to create symbolic link for %s\n", tgtFile.c_str());
      printf("symbolic link name %s\n", linkFullPath.c_str());
      printf("Error %d\n", err);
  }
#else
  
  int unixReturnCode = symlink(tgtFile.c_str(), linkFullPath.c_str());
  if(0 == unixReturnCode)
    return(true);
  // ToDo return true if errno returns EEXIST

#endif

  // Win, 'CreateSymbolicLink', clients should call 'GetLastError' for extended information.
  // Unix, 'symlink', The errno global variable is set to indicate the error.

  return(false);  // Notify clients that call was known to have failed  
}

std::string cvac::expandFilename(std::string filename, std::string prefixDir)
{
    std::string result; 
    if ((filename.length() > 1 && filename[1] == ':' )||
        filename[0] == '/' ||
        filename[0] == '\\')
    {  // absolute path
        result = filename;
    } else { // prepend our prefix
        result = (prefixDir + "/" + filename);
    }
    return result;
}

void cvac::sleep(int numberOfMilliseconds)
   {
      #ifndef WIN32
         ::usleep(numberOfMilliseconds * 1000);   // usleep takes sleep time in us
      #else
         ::Sleep(numberOfMilliseconds);
      #endif
  }
/*
BaseException(const std::string& newMsg) { //const std::string &newMsg

  msg = newMsg;
}
*/
