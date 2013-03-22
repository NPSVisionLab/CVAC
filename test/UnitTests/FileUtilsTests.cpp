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
using namespace Ice;
using namespace cvac;
using namespace UnitTest;
using namespace std;
typedef pair<std::string, std::string> Str_Pair;
///////////////////////////////////////////////////////////////////////////////

SUITE(UnitTests_cvac)
{
  void printSymlinkHintsPerPlatform(bool callSuccess);  // Declared inside this Test suite

  TEST(getBaseFileNameWithDotInThePath)
  {
     cout << endl << endl << endl << "#####################################################################" << endl;
     const std::string filename = "c:/temp/something.here/myfile.txt";

     CHECK_EQUAL("myfile", cvac::getBaseFileName(filename));
  }

  TEST(currentWorkingDirectoryShouldntBeEmpty)
  {
     CHECK(!getCurrentWorkingDirectory().empty());
  }

  TEST(checkMakeTieredDirectoryAndClear)
  {
    std::string CWD = std::string(getCurrentWorkingDirectory().c_str());
    
    // Add hierarchy root folder
    std::string runsetRootDir = (CWD + "/rootDir");
    cout << "Creating tiered test directory at:  \n" << runsetRootDir << endl << endl;
    makeDirectory(runsetRootDir);

    // Add nested '/images' folder
    std::string secondTierFolder = (runsetRootDir + "/images");
    cout << "Creating tiered test directory at:  \n" << secondTierFolder << endl << endl;
    makeDirectory(runsetRootDir);

    // Set breakpoint here, to check for successful creation of folders

    // Clear folders: deepest first, to root
    deleteDirectory(secondTierFolder);
    deleteDirectory(runsetRootDir);
  }

  TEST(checkPaths_forIllegalChars)
  {
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

  TEST(makeSymlinkFile) {  // Note path to target file must be Absolute, at least was true on Windows development machine
    fstream linkOutputFile, linkSourceFile;
    std::string linkDir = std::string(getCurrentWorkingDirectory().c_str());
    std::string tempDir = "../test/temp/testLink";
    std::string linkPath = tempDir + "/linkOutputFile.txt";
    std::string tgtFile = linkDir + "/../test/data/sourceLinkFile.txt";  // Relative to CVAC/bin

    // Test checks access to source file
    linkSourceFile.open(tgtFile.c_str(), ios::in);
    if(linkSourceFile.is_open()) {
      linkSourceFile.close();
    }
    else {
      printf("Could not find source file: 'sourceLinkFile.txt'.  This test should \n");
      printf("be run from an Administrator Console with CWD as the 'CVAC/bin' directory.  \n");
      printf("The relative path to the source file is thus '../test/UnitTests/' from CWD.  \n");
    }

    // Check Paths
    cout << endl << "######### Make Symlink File        ##################################" << endl;
    cout << "Target at " << tgtFile << endl;
    cout << "linkFile at: " << linkPath << endl;
    // In case its still around after a failure delete it and the link.
    deleteDirectory(tempDir);
    makeDirectory(tempDir);


    // Verify input file exists
    bool callSuccess = makeSymlinkFile(linkPath, tgtFile);
    linkOutputFile.open(linkPath.c_str(), ios::in);
    bool testSuccess = linkOutputFile.is_open();


    if(!testSuccess) {        // Explain failure in making symlink
            printf("\n");
            printf("UnitTest could not find link target:  '%s' \n", tgtFile );
            printf("subfolder of CWD.\n");
            printSymlinkHintsPerPlatform(callSuccess);
    }
    else {  // Clear dir and link file
      printf("Test found output symlink file.\n");
      linkOutputFile.close();
      deleteDirectory(tempDir);
    }

    // Found expected Symlink file on disk?
    CHECK(testSuccess);
  }

  TEST(addSamples_toRunset)
  {
    cout << endl << "######### Add Samples to RunSet    ##################################" << endl;
    std::map<std::string, std::string> symlinkFilenames;
    fstream inputFile, symFile;
    RunSet runSet;
    Purpose pos_vehicle;
    pos_vehicle.ptype = POSITIVE;
    Purpose neg_background;
    neg_background.ptype = NEGATIVE;
    std::string tgtFilename, path, input_fullPath, symlinkFullPath = "";
    tgtFilename = "A a.jpg";
    std::string dir = getCurrentWorkingDirectory().c_str(); // c_str needed to properly size the string.
    input_fullPath = dir + std::string("/../test/data/");
    input_fullPath += tgtFilename;
    cout << "Target at " << input_fullPath << endl;

    // Verify input file exists
    inputFile.open(input_fullPath.c_str(), ios::in);
    if(inputFile.is_open()) {
      inputFile.close();
    }
    else {
        printf("Fatal Error, could not find required input file: %s\n", input_fullPath.c_str());
        CHECK(false); // Can't proceed without input file
        exit(0);
    }
    
    // Get symlink path for the illegal input path
    bool newSymlink, callSuccess;
    
#ifdef WIN32
    char *tempName = _tempnam(dir.c_str(), NULL);
#else
    char *tempName = tempnam(dir.c_str(), NULL);
#endif /* WIN32 */
    std::string tempString = tempName;
    FilePath fpath;
    fpath.directory.relativePath = "test\\data";
    fpath.filename = tgtFilename;
    symlinkFullPath = getLegalPath(tempName, fpath, newSymlink);  
    callSuccess = makeSymlinkFile(symlinkFullPath, input_fullPath);

    // Verify symlink file exists
    symFile.open(symlinkFullPath.c_str(), ios::in);
    if(symFile.is_open()) {
      symFile.close();
    }
    else {
        printf("Fatal Error, could not find generated symlink file: %s\n", input_fullPath.c_str());
        printSymlinkHintsPerPlatform(callSuccess);

        CHECK(false); // Can't proceed
        exit(0);
    }

    // Add original and symlink names to map
    symlinkFilenames.insert(Str_Pair(input_fullPath, symlinkFullPath));

    // Add 1st simlinked name to runset
    std::string substName = getSymlinkSubstitution(tgtFilename);
    addToRunSet(runSet, symlinkFullPath, substName, pos_vehicle);
    CHECK(1 == runSet.purposedLists.size());

    //// ### Each 'tempFolder' is unique in CWD ###
    //std::string CWD = std::string(getCurrentWorkingDirectory().c_str());
    //std::string runsetRootDir;
    //char tempFolder[L_tmpnam_s];
    //errno_t err;
    //err = tmpnam_s(tempFolder, L_tmpnam_s);

    //// Create root folder of temporary hierarchy
    //runsetRootDir = (CWD + std::string("/") + std::string(tempFolder));
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
    //deleteDirectory(runsetRootDir);
    deleteDirectory(tempName);
  }

  TEST(printCWD) {
    std::string CWD = std::string(getCurrentWorkingDirectory().c_str());
    printf("Commands start out relative to Curent Working Directory:\n");
    printf("--------------------------------------------------------\n");
    printf("%s\n\n", CWD.c_str());

    int stopHere = 0;  // breakpoint
  }

  TEST(cvacLibArchiveExpand) {

    printf("TEST: cvacLibArchiveExpand\n");
    // Test: with subfolder
    // Expand files from source archive
    std::string expandDir = "testtemp";
    std::string detectZip = "TrainUSKOJI.zip";
    std::string archiveFileName1("../data/DefaultDetectors/" + detectZip);
    // wipe out any old directory we will expand into
    deleteDirectory(expandDir);
    std::vector<std::string> fileNameStrings = expandSeq_fromFile(archiveFileName1, expandDir);

    // Check for expanded files
    ifstream testForXml;
    std::string CWD = getCurrentWorkingDirectory();
    std::string trimmedCWD = CWD.c_str();
    std::string pathToClear = (trimmedCWD + expandDir);
    std::string checkFilePath = (pathToClear + "logTrain_svm.xml.gz");
    testForXml.open(checkFilePath.c_str(), ifstream::in);

    // File exists after extraction
    CHECK(true == (0 != testForXml));
    printf("Success opening expanded file (from subfolder).");

    // Clear
    deleteDirectory("." + expandDir);
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

  // confusion table (in Utils)
}
