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

// Archive file must include: 'usageOrder.txt', which allows filenames to be returned in a
// specific argument order without relying on fancy extensions or pattern matching.  
// Format: one filename per line.  Ordering documented by receiver for its expectations.
std::vector<std::string> expandSeq_fromFile(const std::string& filename, const std::string& expandSubfolder) {
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
    localAndClientMsg(VLogger::WARN, NULL, "DetectorDataArchive function: 'expandSeq_fromFile' could not open requested file: %s\n", filename.c_str());
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
    std::string fullSubfolderPath(".");
    fullSubfolderPath += expandSubfolder;
    fullSubfolderPath += "/";
    fullSubfolderPath += entryFilename;
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
  
  std::vector<std::string> entryFileNames;

    // Read the local 'usageOrder.txt' key file which should have been extracted
    ifstream orderingTxtFile;

    std::string orderingSubfolderPath(".");  // Build subfolder path
    orderingSubfolderPath += expandSubfolder;
    orderingSubfolderPath += "/usageOrder.txt";
    orderingTxtFile.open(orderingSubfolderPath.c_str());

    if(!orderingTxtFile) {
      localAndClientMsg(VLogger::WARN, NULL, "Archive file did not contain a file ordering in 'usageOrder.txt'.  Returning empty vector<string>.");
      return(entryFileNames);  // empty vector<string>
    }

    int lineCount = 0;
    std::string line;
    while(std::getline(orderingTxtFile, line))
    {
      entryFileNames.push_back(line);
      lineCount++;
    }

    // An empty ordering file results in an empty vector<string> returned
    if(0 == lineCount) {
      localAndClientMsg(VLogger::WARN, NULL, "Archive file contained 'usageOrder.txt', but no text of filenames.  Returning empty vector<string>.");
    }

  return(entryFileNames);
}
std::vector<std::string> expandSeq_fromFile(const std::string& filename)
{
  // Expand subfolder to the usual Current Working Directory
  return(expandSeq_fromFile(filename, ""));
}
