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
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <stdio.h>
#include <exception>
#include <stdarg.h>
#include <archive.h>
#include <archive_entry.h>
#include "util/DetectorDataArchive.h"
#include "util/FileUtils.h"

using namespace cvac;
using namespace std;
#ifndef PATH_MAX
#define PATH_MAX 255
#endif

static const std::string PROPS = "trainer.properties";


///////////////////////////////////////////////////////////////////////////////
cvac::DetectorDataArchive::DetectorDataArchive()
{
}


///////////////////////////////////////////////////////////////////////////////
void cvac::DetectorDataArchive::createArchive(const std::string &tdir)
{

      // Create a trainer.properties file if we have any
      // properties to save.
      if (!mPropNames.empty() || !mFileIds.empty())
      {
          std::string props = tdir + "/" + PROPS;
          std::ofstream propFile;
          propFile.open(props.c_str(),std::ofstream::out);
          if (propFile.is_open())
          { // Add all the trainer properties
              int size = mPropNames.size();
              int i;
              // ":" indicates a property
              // "=" indicates a file id 
              for (i = 0; i < size; i++)
              {
                  propFile << mPropNames[i] <<  " : " << mPropValues[i] << std::endl; 
              }
              // add the file identifiers
              size = mFileIds.size();
              for (i = 0; i < size; i++)
              { 
                  // We need to store the filenames relative to the archive so when we unpack
                  // the archive the names in the propfile match the contents in a relative
                  // fashion.
                  std::string fname;  // relative file name
                  
                  fname = getFileName(mFileNames[i]);
                  int idx = fname.find(tdir,0);
                  if (idx == 0)
                      fname = fname.substr(tdir.length()+1);

                  propFile << mFileIds[i] <<  " = " << 
                           fname << std::endl; 
              }
              propFile.close();
              propFile.flush();
          }
      }
      std::vector<std::string> tListFiles;
      vector<string>::iterator it;
      if (!mFileNames.empty() || !mPropNames.empty())
          tListFiles.push_back(tdir + "/" + PROPS);
      for (it = mFileNames.begin(); it != mFileNames.end(); ++it)
      {
          // Here we assume that the path to the file added by the user is correct
          tListFiles.push_back(*it);      
      }
      if(!writeZipArchive(mArchiveName, tListFiles))
      {
          localAndClientMsg(VLogger::ERROR, NULL,
              "Could not write archive file %s.\n", mArchiveName.c_str());
          throw "Could not write archive file";
           
      }
     
}

///////////////////////////////////////////////////////////////////////////////
void cvac::DetectorDataArchive::unarchive(const string &archiveFile, const string &cdir)
{

    mArchiveName = archiveFile;
    // Clear any old trainer properties
    mPropNames.clear();
    mPropValues.clear();
    mFileNames.clear();
    mFileIds.clear();
    expandSeq_fromFile(archiveFile, cdir);
    // Read the trainer properties in from trainer.properties file
    string propName = cdir + "/" + PROPS;
    std::ifstream propfile(propName.c_str());
    std::string line;
    while( std::getline(propfile, line))
    {
        char name[512];
        char value[512];
        int res;
        // See if its a property
        int idx = line.find(":");
        if (idx > 0)
        {
            if (res = sscanf(line.c_str(), "%s : %s", name, value) > 0)
            { 
                // We got a name value pair to save
                mPropNames.push_back(string(name));
                mPropValues.push_back(string(value));
                continue;
            }
        } 
        // See if its an id
        if (res = sscanf(line.c_str(), "%s = %s", name, value) > 0)
        {
            // We got a name value pair to save
            mFileIds.push_back(string(name));
            mFileNames.push_back(cdir + "/" + string(value));
        }
    }
    
}

///////////////////////////////////////////////////////////////////////////////
void cvac::DetectorDataArchive::setProperty(const string &name,
                                             const string &value)
{
    int size = mPropNames.size();
    int i;
    bool found = false;
    for (i = 0; i < size; i++)
    {
        if (mPropNames[i].compare(name) == 0)
        {
            found = true;
            mPropValues[i] = value;
        }
    }
    if (found == false)
    {
        mPropNames.push_back(name);
        mPropValues.push_back(value);
    }
}

///////////////////////////////////////////////////////////////////////////////
vector<string> cvac::DetectorDataArchive::getProperties()
{
    return mPropNames;
}

///////////////////////////////////////////////////////////////////////////////
string cvac::DetectorDataArchive::getProperty(const string &name) const
{
    string empty;
    int size = mPropNames.size();
    int i;
    for (i = 0; i < size; i++)
    {
        if (mPropNames[i].compare(name) == 0)
        {
            return mPropValues[i];
        }
    }
    return empty;
}

///////////////////////////////////////////////////////////////////////////////
void cvac::DetectorDataArchive::setArchiveFilename(std::string &filename)
{
    mArchiveName = filename;
}

///////////////////////////////////////////////////////////////////////////////
bool cvac::DetectorDataArchive::addFile(const std::string &identifier,
                                        const std::string &filename)

{
    vector<string>::iterator it;
    for (it = mFileIds.begin(); it != mFileIds.end(); ++it)
    {
        if ((*it).compare(identifier) == 0)
            return false;
    }
    mFileIds.push_back(identifier);
    mFileNames.push_back(filename);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool cvac::DetectorDataArchive::removeFile(const std::string &identifier)
{
    int size = mFileIds.size();
    int i;
    for (i = 0; i < size; i++)
    {
        if (mFileIds[i].compare(identifier) == 0)
        {
            mFileIds.erase(mFileIds.begin() + i);
            mFileNames.erase(mFileNames.begin() + i);
            return true;
        }
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////
const std::vector<std::string> cvac::DetectorDataArchive::getFileIds()
{
    return mFileIds;
}

///////////////////////////////////////////////////////////////////////////////
const std::string
cvac::DetectorDataArchive::getFile(const std::string &identifier) const
{
    std::string empty;
    int size = mFileIds.size();
    int i;
    for (i = 0; i < size; i++)
    {
      // printf("%s: %s\n", mFileIds[i].c_str(), mFileNames[i].c_str());
        if (mFileIds[i].compare(identifier) == 0)
        {
            return mFileNames[i];
        }
    }
    return empty;
}

///////////////////////////////////////////////////////////////////////////////
int copy_data(struct archive *ar, struct archive *aw)
{
  int r;
  const void *buff;
  size_t size;

#if defined(__int64)   // Windows VS-2010, or other environments that define '__int64'
  __int64 offset;
#else
  __LA_INT64_T offset;
#endif

  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF)
      return (ARCHIVE_OK);
    if (r != ARCHIVE_OK)
      return (r);
    r = archive_write_data_block(aw, buff, size, offset);
    if (r != ARCHIVE_OK) {
      localAndClientMsg(VLogger::WARN, NULL, archive_error_string(aw));
      return (r);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// arguments: filename: the archive (zip) file
//            expandSubfolder: folder to extract into
void expandSeq_fromFile(const std::string& filename, const std::string& expandSubfolder) {
  struct archive *a;
  struct archive *ext;
  struct archive_entry *entry;
  int flags;
  int r;

  
  /* Select which attributes we want to restore. */
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_compression_all(a);
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);


//std::string directory = std::string(getCurrentWorkingDirectory().c_str());
  if((r = archive_read_open_file(a, filename.c_str(), 10240))) {
    localAndClientMsg(VLogger::WARN, NULL,
                      "DetectorDataArchive::expandSeq_fromFile could not open requested file: %s\n",
                      filename.c_str());
    throw "";
  }
  for (;;) {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF) // done loop
      break;

    if (r != ARCHIVE_OK)
      localAndClientMsg(VLogger::WARN, NULL, archive_error_string(a));

    if (r < ARCHIVE_WARN) {
      localAndClientMsg(VLogger::WARN, NULL, "Error reading archive header, in DetectorDataArchive expandSeq_fromFile\n");
      throw "";
    }

    // Augment entry-file which will get specified subfolder added-on
    const char* entryFilename = archive_entry_pathname(entry);
    std::string fullSubfolderPath = expandSubfolder + "/" + entryFilename;
    archive_entry_set_pathname(entry, fullSubfolderPath.c_str());

    r = archive_write_header(ext, entry);
    if (r != ARCHIVE_OK) {
      localAndClientMsg(VLogger::WARN, NULL, archive_error_string(ext));
    }
    //else if (archive_entry_size(entry) > 0) {
    else {
        r = copy_data(a, ext);
        if (r != ARCHIVE_OK) {
          localAndClientMsg(VLogger::WARN, NULL, archive_error_string(a));
        }
        if (r < ARCHIVE_WARN) {
          // Archive error string already echoed, throw exception for serious error
          localAndClientMsg(VLogger::WARN, NULL, "Error writing archive header, in DetectorDataArchive expandSeq_fromFile");
          throw "";
        }
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
          localAndClientMsg(VLogger::WARN, NULL, archive_error_string(ext));
        }
        if (r < ARCHIVE_WARN) {
          // Archive error string already echoed, throw exception for serious error
          localAndClientMsg(VLogger::WARN, NULL, "Error writing archive header, in DetectorDataArchive expandSeq_fromFile");
          throw "";
        }
     }
  }
  archive_read_close(a);
  archive_write_close(ext);
#if LIBARCHIVE_VERSION>=3
  archive_read_free(a);
  archive_write_free(ext);
#else
  archive_read_finish(a);
  archive_write_finish(ext);
#endif
  
}
void expandSeq_fromFile(const std::string& filename)
{
  // Expand subfolder to the usual Current Working Directory
  expandSeq_fromFile(filename, "");
}

///////////////////////////////////////////////////////////////////////////////
bool writeZipArchive(const std::string& _outpath,const std::vector<std::string>& _inPaths)
{
  //we need to add some error handling routines
  struct archive *a;
  struct archive_entry *entry;
  struct stat st;

  a = archive_write_new();  
  archive_write_set_format_zip(a);
  archive_write_open_filename(a, _outpath.c_str());
  for(unsigned int k=0;k<_inPaths.size();k++)
  {
    string tFilename = getFileName(_inPaths[k]);
    stat(_inPaths[k].c_str(), &st);
    entry = archive_entry_new();     
    archive_entry_copy_stat(entry, &st);
    archive_entry_set_pathname(entry, tFilename.c_str());
    archive_write_header(a, entry);  

    std::ifstream is(_inPaths[k].c_str(),std::ios::binary);    
    if(!is.is_open())
    {
      std::cout << "The target file: "
        << _inPaths[k].c_str() 
        << " may not exist or has a problem.\n";
      return false;
    }

    is.seekg (0, is.end);
    int tBuffSize = is.tellg();
    is.seekg (0, is.beg);
    char* tBuff = new char[tBuffSize]; 
    if(tBuff == 0 || tBuff == NULL)
    {
      std::cout << "The target file: "
        << _inPaths[k].c_str() 
        << " is too big to be compressed.\n";
      return false;
    }
    is.read(tBuff,tBuffSize);
    archive_write_data(a, tBuff, tBuffSize);    
    is.close(); 
    delete [] tBuff;

    archive_entry_free(entry);
  }
  archive_write_finish(a);  

  return true;
}
