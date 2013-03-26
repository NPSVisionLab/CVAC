
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
#ifndef FILEUTILS_H__
#define FILEUTILS_H__
#include <string>
#include <Data.h>  // Ice classes must be available for 'addFile_toRunSet..' definitions
#include <Services.h>
#include <exception>
#include <util/VLogger.h>
#include <stdarg.h>

#if defined(WIN32)  // Includes for windows utility: 'tmpnam_s' to generate a temp folder
  #include <stdio.h>
  #include <stdlib.h>
#endif


namespace cvac
{ 
  static VLogger vLogger;  // vLogger object accessed directly as 'cvac::vLogger'

  /** Echo message both locally ('printv'), and by sending a client message
     * only if there is adequate verbosity level.
     * @param level: Verbosity level inherent to the message.
     * @param callbackHandler: A reference to the Ice client callbackHandler object.
     */
  void localAndClientMsg(VLogger::Levels level, const ::cvac::CallbackHandlerPrx& callbackHandler, const char* fmt, ...);
  
   /** Does the supplied directory exist?
     * @param directory The path to the directory to verify
     * @return True if the directory exists, false if not (or if something exists
     *         but is not a directory.
     */
   bool directoryExists(const std::string& directory);

   /** Extract just the path to the supplied fully-qualified file name.
     * @code getFilePath("c:/temp/foo/myFile.txt"); //returns "c:/temp/foo"
     * @param fileName the full path and file name
     * @return The extracted path to the supplied fileName
     */
   std::string getFilePath(const std::string& fileName);

   /** Return the working directory that exists at the time of this function call
     * @return The current working directory
     */
   std::string getCurrentWorkingDirectory();

   /** Make a directory based on the supplied path. This will only create one
     * level of directory (i.e., it won't create every directory in the path).
     * @param path The directory to create (assumes the path's parent exists)
     * @return True if the command was successful, false otherwise
     */
   bool makeDirectory(const std::string& path);

   /**  Get the file name, excluding the path.
     *  @code getFileName("/temp/myfile.tar.gz") //returns "myfile.tar.gz"
     *  @param fileName The file name with path to extract just the file name from
     *  @return Just the file name
     */
   std::string getFileName(const std::string& fileName);

   /** Get the base of a file name without the path or extension. This contains
     * all the characters of the file name up to the first '.' character.
     * @code getBaseFileName("/temp/myfile.tar.gz") //returns "myfile"
     * @return The base part of the supplied fileName
     * @param fileName The full file name, with or without path
     */
   std::string getBaseFileName(const std::string& fileName);

   /** Remove the contents of the directory and delete the directory.
    *  @return True if successfull
    *  @param path of the directory to remove
    */
   bool deleteDirectory(const std::string& path);

  /** Ensure 'actual' purpose is compatible with the constraint
    */
   bool compatiblePurpose( const cvac::Purpose& actual, const cvac::Purpose& constraint );

  /** Add a file path to a RunSet reference, under the given purpopse.
    *  @param cvac::RunSet& runSet:             The target runset
    *  @param const std::string& relativePath:  The media file path to be added, relative to working dir
    *  @param const std::string& filename:      The media file name to be added
    *  @param const cvac::Purpose& purpose      The purpose describing intent and content of the media file
    */
   void addFileToRunSet( cvac::RunSet& runSet, const std::string& relativePath,
                           const std::string& filename, const cvac::Purpose& purpose);

  /** Add a file path to a RunSet reference, under the given purpopse.
    *  @param cvac::RunSet& runSet:             The target runset
    *  @param const std::string& relativePath:  The media file path to be added, relative to working dir
    *  @param const std::string& filename:      The media file name to be added
    *  @param int classID:                      Identifying class for media file
    */
   void addFileToRunSet( cvac::RunSet& runSet, const std::string& relativePath,
                           const std::string& filename, int classID);

  /** Create a symlink given the parameters using the appropriate Windows or Linux
   * API command.
   * @param const std::string linkFullPath:     Path to the link file which will be created.
   * @param const std::string tgtFile
   */
   bool makeSymlinkFile(const std::string linkFullPath, const std::string tgtFile);
   /** Expand filename based on relative/absolute and config setting
   * API command.
   * @param const std::string fileName file name to change if its a relative path
   * @param const std::string prefix string to prepend
   */
   std::string expandFilename(std::string fileName, std::string prefixDir);
};
#endif // FILEUTILS_H__
