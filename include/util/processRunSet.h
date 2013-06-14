#pragma once
/****
 *CVAC Software Disclaimer
 *
 *This software was developed at the Naval Postgraduate School, Monterey, CA,
 *by employees of the Federal Government in the course of their official duties.
 *Pursuant to title 17 Section 105 of the United States Code this software
 *is not subject to copyright protection and is in the public domain. It is 
 *an experimental system.  The Naval Postgraduate School assumes no
 *responsibility whatsoever for its use by other parties, and makes
 *no guarantees, expressed or implied, about its quality, reliability, 
 *or any other characteristic.
 *We would appreciate acknowledgement and a brief notification if the software
 *is used.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above notice,
 *      this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the Naval Postgraduate School, nor the name of
 *      the U.S. Government, nor the names of its contributors may be used
 *      to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****/
/**
 * Utilities to read in a RunSet and then make callbacks to a user supplied
 * routine to process each file of the RunSet.
 */
#include <util/FileUtils.h>
#include <util/ServiceMan.h>
#include <Data.h>
#include <Services.h>
#include <iostream>
#include <map>
#include <utility>

#if defined(WIN32)
  #include <winbase.h>
#endif

namespace cvac
{

   /**
   * This function is called from processRunSet() as the RunSet is
   * traversed.  It will be called for each file that needs to have the
   * detector run on it. The detector is responsible for returning a filled out
   * ResultSet (whether or not it found anything).
   * Here it is assumed that the detector pointer really points to a
   * DetectorI (a detector instance).
   */
   typedef ResultSetV2 (*DoDetectFunc) ( DetectorPtr detector, const char *filename);

   /**
   * Process a RunSet calling the passed in DoDetectFunc for each file in
   * to be processed and then calling the DetectorCallbacHandler with that
   * result.
   */
   void processRunSet(DetectorPtr detector,
                      const DetectorCallbackHandlerPrx &client,
                      DoDetectFunc detectFunc, 
                      const RunSet &run, 
                      const std::string &pathPrefix,
                      ServiceManager *servMan);

   void sendResultsToClient( const DetectorCallbackHandlerPrx &client, 
                             const ResultSetV2& results );

   // Check for any chars within the string that could upset the detector
   bool containsIllegalChars(FilePath filePath);
   
   // Skip detector execution on input files that match undesired substring(s)
   //bool shouldIgnore(FilePath filePath);


   void addToRunSet( RunSet& runSet, const std::string& relativePath,
	   const std::string& filename, const Purpose& purpose, LocationPtr loc = NULL);	// Using Purpose
   void addToRunSet( RunSet& runSet, const std::string& relativePath,
                     const std::string& filename, int classID, LocationPtr loc = NULL);	// Using ID of Class

   // Pair the old filename and the new symlink filename in a map 
   void addFilenamePair(const std::string& filename, const std::string& symLinkName);

   std::string getSymlinkSubstitution(const std::string& illegalFileName);

   /** Return path of a new symlink if fname contains illegal chars, or original path otherwise.
    *  The map between old and new names is managed directly by clients outside this function.
    */
   std::string getLegalPath(std::string tempDir, FilePath filePath, bool &newSymlink);
   
   /**
    * Fix the Run Set by linking files with illegal names (containing spaces) to valid
    * file names.  These links are stored in the directory name returned and should be
    * deleted after the run set is finished being processed.
    */
   std::string fixupRunSet( RunSet &runset);
}
