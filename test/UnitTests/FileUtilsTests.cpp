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
#include <UnitTest++.h>
#include <util/FileUtils.h>
#include <util/processRunSet.h>
#include <util/DetectorDataArchive.h>
#include <fstream>
#include <map>
#include <utility>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(WIN32) && !defined(S_ISDIR)
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif
using namespace Ice;
using namespace cvac;
using namespace UnitTest;
using namespace std;
typedef pair<std::string, std::string> Str_Pair;
///////////////////////////////////////////////////////////////////////////////

bool pdebug = true;// (VLogger::DEBUG_2 <= cvac::vLogger.getBaseLevel());

SUITE(UnitTests_cvac)
{
  // Declared inside this Test suite  
  void printSymlinkHintsPerPlatform(bool callSuccess);
  std::string getTempFilename( const std::string& basedir="" );
  bool createAndTestFile( const std::string& srcFile,
                          const std::string& testtext="somerandomtext\n" );
  bool deleteFile( const std::string& filename );

  TEST(getBaseFileNameWithDotInThePath)
  {
    printf("getBaseFileNameWithDotInThePath\n");
    const std::string filename = "c:/temp/something.here/myfile.txt";

    CHECK_EQUAL("myfile", cvac::getBaseFileName(filename));
  }

  TEST(testCurrentWorkingDirectoryIsNotEmpty)
  {
    printf("testCurrentWorkingDirectoryIsNotEmpty\n");
    CHECK(!getCurrentWorkingDirectory().empty());
  }

  TEST(checkMakeTieredDirectoryAndClear)
  {
    printf("checkMakeTieredDirectoryAndClear\n");
    std::string tempDir = getTempFilename();
    
    // Add hierarchy root folder
    std::string runsetRootDir = (tempDir + "/rootDir");
    if (pdebug)
      cout << "Creating tiered test directory at:  \n" << runsetRootDir << endl;
    makeDirectory(tempDir);
    CHECK( directoryExists(tempDir) );
    makeDirectory(runsetRootDir);
    CHECK( directoryExists(runsetRootDir) );

    // Add nested '/images' folder
    std::string secondTierFolder = (runsetRootDir + "/images");
    if (pdebug)
      cout << "Creating tiered test directory at:  \n" << secondTierFolder << endl;
    makeDirectory(secondTierFolder);
    CHECK( directoryExists(secondTierFolder) );

    // Clear folders: deepest first, to root
    CHECK( deleteDirectory(secondTierFolder) );
    CHECK( deleteDirectory(runsetRootDir) );
    CHECK( deleteDirectory(tempDir) );
  }

  TEST(checkPathsForIllegalChars)
  {
    printf("checkPathsForIllegalChars\n");
    // Currently, the OpenCv based detectors like 'Cv_Cmd' will not
    // accept spaces in filenames.  
    FilePath fpath;
    fpath.filename = std::string("abc.jpg");
    bool noSpaces_Ok = containsIllegalChars(fpath);
    CHECK(false == noSpaces_Ok);
    fpath.filename = std::string("a bc.jpg");
    bool withSpaces_Illegal = containsIllegalChars(fpath);
    CHECK(true == withSpaces_Illegal);
  }

  // Note that the path to target file must be Absolute,
  // at least was true on Windows development machine
  TEST(makeSymlinkFile)
  {
    printf("makeSymlinkFile\n");

    // create a link from a tgtFile to an existing srcFile,
    // where the created link (tgtFile) is in a subdirectory
    // of the srcFile's directory
    std::string srcDir = getTempFilename();
    std::string srcFile = srcDir + "/sourceFile.txt";
    std::string tgtDir = srcDir + "/linktgtdir";
    std::string tgtFile = tgtDir + "/targetFile.txt";
    printf("tgtDir: %s\n", tgtDir.c_str());
    // create both src and tgt directories
    bool madeDirs = makeDirectories(tgtDir);
    CHECK( directoryExists( tgtDir ) );

    // Create and test access to source file
    std::string testtext = "testtext";
    bool created = createAndTestFile( srcFile, testtext );
    if (!created)
    {
      printf( "Could not find source file: '%s'.\n", srcFile.c_str());
    }
    CHECK( created );

    // Check Paths
    if (pdebug)
    {
      cout << "  source at: " << srcFile << endl;
      cout << "  target at: " << tgtFile << endl;
    }

    // create symlink
    bool callSuccess = makeSymlinkFile(tgtFile, srcFile);

    // test if we can open the file via the symlink
    fstream tgtStream;
    tgtStream.open(tgtFile.c_str(), ios::in);
    bool symlinkFileExists = tgtStream.is_open();

    if(symlinkFileExists)
    {
      // TODO: check file contents, compare against "testtext"
      if (pdebug)
        printf("Test found output symlink file.\n");
      tgtStream.close();
    }
    else
    {        // Explain failure in making symlink
            printf("UnitTest could not find link target:  '%s' \n", tgtFile.c_str() );
            printSymlinkHintsPerPlatform(callSuccess);
    }
    // Found expected Symlink file on disk?
    CHECK( symlinkFileExists );

    // remove all files and directories
    CHECK( deleteFile( srcFile ));
    CHECK( deleteFile( tgtFile ));
    CHECK( deleteDirectory(tgtDir) );
    CHECK( deleteDirectory(srcDir) );
  }

  TEST(addSamplesToRunset)
  {
    printf("addSamplesToRunset\n");
    std::map<std::string, std::string> symlinkFilenames;
    RunSet runSet;
    Purpose pos_vehicle;
    pos_vehicle.ptype = POSITIVE;
    Purpose neg_background;
    neg_background.ptype = NEGATIVE;
    std::string tempDir = getTempFilename();
    CHECK( makeDirectory( tempDir ) );
    std::string srcFilename = "A a.jpg";
    std::string srcPath = tempDir + "/" + srcFilename;
    if (pdebug) cout << "  target at " << srcPath << endl;

    CHECK( createAndTestFile( srcPath, "notreallyajpg\n" ) );
    // Can't really proceed without input file...
    
    // Get symlink path for the illegal input path;
    // place the symlink into linkDir
    FilePath fpath;
    fpath.directory.relativePath = "";
    fpath.filename = srcFilename;

    bool newSymlink; // this is an output argument
    // linkDir will be created on demand by getLegalPath, put it inside tempDir
    std::string linkDir = getTempFilename( tempDir );
    if (pdebug) cout << "  linkDir " << linkDir << endl;
    std::string symlinkFullPath = 
      getLegalPath(linkDir.c_str(), fpath, newSymlink);
    if (pdebug) cout << "  creating symlink at " << symlinkFullPath << endl;
    CHECK( makeSymlinkFile(symlinkFullPath, srcPath) );

    // Verify symlink file exists
    fstream symFile;
    symFile.open(symlinkFullPath.c_str(), ios::in);
    if(symFile.is_open()) {
      symFile.close();
    }
    else {
        printf("Fatal Error, could not find generated symlink file: %s\n", srcPath.c_str());
        printSymlinkHintsPerPlatform(false);
        CHECK(false); // Can't proceed
    }

    // Add original and symlink names to map
    symlinkFilenames.insert(Str_Pair(srcPath, symlinkFullPath));

    // Add 1st simlinked name to runset
    std::string substName = getSymlinkSubstitution(srcFilename);
    addToRunSet(runSet, symlinkFullPath, substName, pos_vehicle);
    CHECK(1 == runSet.purposedLists.size());

    //// ### Each 'tempFolder' is unique in tempDir ###
    //std::string tempDir = getTempFilename();
    //std::string runsetRootDir;
    //char tempFolder[L_tmpnam_s];
    //errno_t err;
    //err = tmpnam_s(tempFolder, L_tmpnam_s);

    //// Create root folder of temporary hierarchy
    //runsetRootDir = (tempDir + std::string("/") + std::string(tempFolder));
    //if(!err)
    //{
//  printf("Creating root RunSet folder: %s\n", tempFolder);
    //  makeDirectory(runsetRootDir);
    //}
    //else {
    //  printf("Error creating root RunSet folder: %s\n", tempFolder);
    //}

    //// Create folder for 1st RunSet File

    //// Create 1st RunSet File
    //makeDirectory(substName);

    //// Clear root RunSet folder
    //CHECK( deleteDirectory(runsetRootDir);

    CHECK( deleteFile(symlinkFullPath) );
    CHECK( deleteDirectory(linkDir) );
    CHECK( deleteFile(srcPath) );
    CHECK( deleteDirectory(tempDir) );
  }

  TEST(cvacLibArchiveExpand)
  {
    printf("cvacLibArchiveExpand\n");
    // Test: with subfolder
    // Expand files from source archive
    std::string detectZip = "bowUSKOCA.zip";
    std::string archiveFileName1("../data/detectors/" + detectZip);
    std::string subDir = archiveFileName1 + "_tmpSubDir";

    expandSeq_fromFile(archiveFileName1, subDir);
    // Check for expanded files
    

    // rm -f current not implemented - delete the files manually
    CHECK( deleteFile( subDir+"/logTrain_svm.xml.gz") );
    CHECK( deleteFile( subDir+"/logTrain_Table.txt") );
    CHECK( deleteFile( subDir+"/logTrain_Vocabulary.xml.gz") );
    CHECK( deleteFile( subDir+"/trainer.properties") );
    CHECK( directoryExists(subDir) ); // we expect this temp dir to NOT delete itself
    CHECK( deleteDirectory( subDir) );
  }

 

  void printSymlinkHintsPerPlatform(bool callSuccess) {

  #if defined(WIN32)  // Windows Specific Hints
    printf("You must execute the 'MakeSymlinkFile' Unit Test as Adminstrator from the console:\n");
    printf("(right click on console in StartMenu, run as Admistrator\n\n");

  #else               // Unix Specific Hints
    if(false == callSuccess)
      printf("The unix call 'symlink(..' failed.  The globally set value of 'errno' is: %d\n", errno);
    else
      printf("The unix call 'symlink(..' reported success creating the symlink.\n");
  #endif
  }

  std::string getTempFilename( const std::string& basedir )
  {
    // for debugging:
    //    if (basedir=="") return "/your/named/tmpdir";
#ifdef WIN32
    char *tempName = _tempnam(basedir.c_str(), NULL);
#else
    char *tempName = tempnam(basedir.c_str(), NULL);
#endif /* WIN32 */
    return std::string( tempName );
  }
  
  bool createAndTestFile( const std::string& srcFile, const std::string& testtext )
  {
    ::FILE* fp = fopen( srcFile.c_str(), "w" );
    fprintf( fp, "%s", testtext.c_str() );
    fclose( fp );
    fstream srcStream;
    srcStream.open( srcFile.c_str(), ios::in );
    if(srcStream.is_open()) {
      srcStream.close();
      return true;
    }
    if(pdebug) {
      printf("Fatal Error, could not create file: %s\n", srcFile.c_str());
    }
    return false;
  }

  /** remove the file.  fails for directories, at least on *nix and OSX (by design).
   */
  bool deleteFile( const std::string& filename )
  {
#ifdef WIN32
    int res = remove( filename.c_str() );
#else // WIN32
    int res = unlink( filename.c_str() );
#endif // WIN32
    return res==0;
  }

  /* already part of FileUtils
  bool directoryExists( const std::string& fname )
  {
    struct stat s;
    int err = stat(fname.c_str(), &s);
    bool retval = (0==err) && S_ISDIR(s.st_mode);
    if (pdebug && !retval)
    {
      printf("failed to create directory %s\n", fname.c_str());
    }
    return retval;
  }
  */
}
