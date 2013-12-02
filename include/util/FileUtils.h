
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
#ifndef __FILEUTILS_H_INCLUDED__
#define __FILEUTILS_H_INCLUDED__
#include <string>
#include <Data.h>  // Ice classes must be available for 'addFile_toRunSet..' definitions
#include <Services.h>
#include <exception>
#include <util/VLogger.h>
#include <stdarg.h>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#if defined(WIN32)  // Includes for windows utility: 'tmpnam_s' to generate a temp folder
  #include <stdio.h>
  #include <stdlib.h>
#endif


namespace cvac
{ 
  static VLogger vLogger;  // vLogger object accessed directly as 'cvac::vLogger'

  /** Echo message both locally ('printv'), and by sending a client message
     * only if there is adequate verbosity level.
     * @param level Verbosity level inherent to the message.
     * @param callbackHandler A reference to the Ice client callbackHandler object.
     * @param fmt Format string and arguments akin to printf
     */
  void localAndClientMsg(VLogger::Levels level, const CallbackHandlerPrx& callbackHandler, const char* fmt, ...);

  /** Convenience functions to trim whitespace from strings.
   *  These should really be in another file.
   *  Copied from:
   * http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
   */
  // trim from start
  static inline std::string &ltrim(std::string &s)
  {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  // trim from end
  static inline std::string &rtrim(std::string &s)
  {
    s.erase(std::find_if(s.rbegin(), s.rend(),
       std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

  // trim from both ends
  static inline std::string &trim(std::string &s)
  {
    return ltrim(rtrim(s));
  }
  
   /** Does the supplied directory exist?
     * @param directory The path to the directory to verify
     * @return True if the directory exists, false if not (or if something exists
     *         but is not a directory.
     */
   bool directoryExists(const std::string& directory);

   /** Does the supplied file exists?  Must be specified as absolute path.
     * This implementation may not be able to handle the file which is larger than 2GB. 
     * @param abspath (directory + filename) to verify
     * @return True if the file exists, false if not 
     *         but is not a directory.
     */
   bool fileExists(const std::string& abspath);

   /** Extract just the path to the supplied fully-qualified file name.
     * \code getFileDirectory("c:/temp/foo/myFile.txt"); //returns "c:/temp/foo" \endcode
     * @param fileName the full path and file name
     * @return The extracted path to the supplied fileName
     */
   std::string getFileDirectory(const std::string& fileName);

   /** Return the working directory that exists at the time of this function call
     * @return The current working directory
     */
   std::string getCurrentWorkingDirectory();

   /** Make each directory based on the supplied path as required. 
     * @param path The directory tree to create.
     * @return True if the command was successful, false otherwise
     */
   bool makeDirectories(const std::string& path);

   /** Make a directory based on the supplied path. This will only create one
     * level of directory (i.e., it won't create every directory in the path).
     * @param path The directory to create (assumes the path's parent exists)
     * @return True if the command was successful, false otherwise
     */
   bool makeDirectory(const std::string& path);

   /**  Get the file name, excluding the path.
     *  \code getFileName("/temp/myfile.tar.gz") //returns "myfile.tar.gz" \endcode
     *  @param fileName The file name with path to extract just the file name from
     *  @return Just the file name
     */
   std::string getFileName(const std::string& fileName);

   /** Get the base of a file name without the path or extension. This contains
     * all the characters of the file name up to the first '.' character.
     * \code getBaseFileName("/temp/myfile.tar.gz") //returns "myfile" \endcode
     * @return The base part of the supplied fileName
     * @param fileName The full file name, with or without path
     */
   std::string getBaseFileName(const std::string& fileName);

   /**  Get the file extension, excluding the path and the filename.
     *  \code getFileExtension("/temp/myfile.tar.gz") //returns "tar.gz" \endcode
     *  @param path (directory + filename) 
     *  @return Just the extension
     */
   std::string getFileExtension(const std::string& path);

   /** Remove the contents of the directory and delete the directory.
    *  @return True if successfull
    *  @param path of the directory to remove
    */
   bool deleteDirectory(const std::string& path);

   /**
    * Produce a string identifier for the given Purpose.
    * @return a string to identify the purpose or an
    *     int to identify a multiclass class ID.
    */
   std::string getPurposeName( const cvac::Purpose& purpose );
   
  /** Ensure 'actual' purpose is compatible with the constraint
    */
   bool compatiblePurpose( const cvac::Purpose& actual, const cvac::Purpose& constraint );

  /** Add a file path to a RunSet reference, under the given purpopse.
    *  @param runSet:             The target runset
    *  @param relativePath:  The media file path to be added, relative to working dir
    *  @param filename:      The media file name to be added
    *  @param purpose      The purpose describing intent and content of the media file
    */
   void addFileToRunSet( cvac::RunSet& runSet, const std::string& relativePath,
                           const std::string& filename, const cvac::Purpose& purpose);

  /** Add a file path to a RunSet reference, under the given purpopse.
    *  @param runSet:             The target runset
    *  @param relativePath:  The media file path to be added, relative to working dir
    *  @param filename:      The media file name to be added
    *  @param classID:                      Identifying class for media file
    */
   void addFileToRunSet( cvac::RunSet& runSet, const std::string& relativePath,
                           const std::string& filename, int classID);

  /** Create a symlink given the parameters using the appropriate Windows or Linux
   * API command.
   * @param linkFullPath:     Path to the link file which will be created.
   * @param tgtFile
   */
   bool makeSymlinkFile(const std::string linkFullPath, const std::string tgtFile);

   /** Return a unique temporary file name.  If basedir is not defined 
   * then the file name is in the current systems temporary file .
   * @param basedir - Base directory of the temp filename.
   * @param prefix - Prefix the file name with this.
   * @return The temp file name including path.
   */

   std::string getTempFilename( const std::string &basedir="",  
                                const std::string &prefix = "");

   /** Return a filename formated by the date and time.  If basedir is not defined 
   * then the file name is a relative filename
   * @param basedir - Base directory of the dated filename.
   * @param prefix - Prefix the file name with prefix_.
   * @return The dated file name including path.
   */
   std::string getDateFilename( const std::string &basedir = "", 
                                   const std::string &prefix = "");

   /** Turn a CVAC path info a file system path
    * @param fp The filepath to change
    * @param CVAC_DataDir The CVAC data directory 
    * @return a string that concatinates CVAC_Dir and the relative path and
    * filename defined in fp.
    */
   std::string getFSPath(const cvac::FilePath &fp, 
                         const std::string &CVAC_DataDir = "");

   /** Copy a file
    * @param from file
    * @param to file 
    * @return true if file was copied
    */
   bool copyFile(const std::string& fromFile, const std::string& toFile);

   /** Returns true if the path is absolute
    * @param from filemake
    * @return true if file/dir has an absolute path
    */
   bool pathAbsolute(const std::string& filename);
};
#endif // __FILEUTILS_H_INCLUDED__
