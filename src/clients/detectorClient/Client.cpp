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
#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
#include <Services.h>
#include <Data.h>
#include <util/Timing.h>
#include <util/FileUtils.h>
#include <util/ConfusionMatrix.h>
#include <vector>
#include <util/ServiceInvocation.h>
using namespace cvac;

class ClientAppData {

public:
	ClientAppData() 
	{
		isVerification = false;
		multiclassDetection = false;
		videoInputType = false;
		needSBDVerified = false;
	}

	bool isVerification;
	bool multiclassDetection;
	bool videoInputType;
	bool needSBDVerified;
	std::vector<bool> detectionsVerified;
};

class CallbackHandlerI : public DetectorCallbackHandler
{
public:

	CallbackHandlerI(ClientAppData* appDataRef) 
	{
		this->appDataRef = appDataRef;
	};
	~CallbackHandlerI(){};

	ClientAppData* appDataRef;
    
    virtual void cancelled(const ::Ice::Current &current)
    {
        localAndClientMsg(VLogger::INFO, NULL, "Detection cancelled");
    }

	void foundNewResults(const ::ResultSet& resultSet, const ::Ice::Current& current)
	{
		int stopSize = resultSet.results.size();
		// Loop through each Result in the set
		for (int resultIndex = 0; resultIndex < stopSize; ++resultIndex)
		{
			const ::Result& result = resultSet.results[resultIndex];
			int numLabelsFound = result.foundLabels.size();
			localAndClientMsg(VLogger::INFO, NULL, "Found %d label%s in %s\n", 
					  numLabelsFound, 
					  (numLabelsFound==1)?"":"s",
					  result.original->sub.path.filename.c_str());
            std::string className;
            if (result.foundLabels.size() > 0)
            {
				className = result.foundLabels[0]->lab.name;
				localAndClientMsg(VLogger::INFO, NULL, "Class name: %s\n", className.c_str());
			}
			// Print Video labels if found
			for (int labelIdx = 0; labelIdx < numLabelsFound; labelIdx++)
			{
				LabelablePtr label = result.foundLabels[labelIdx];	
				//For verifying available typeIDs, use "ice_ids" or "ice_id" 			
				if(label->ice_isA("::cvac::LabeledVideoSegment", current))
				{
					LabeledVideoSegmentPtr vid = LabeledVideoSegmentPtr::dynamicCast(label);
					localAndClientMsg(VLogger::INFO, NULL, "Frame number: %d\n", vid->start.time);
				}
			}
			// Regression Test Verification
			if(appDataRef->isVerification)
			{
				// Signal detection for positive image in /CTest
				if(numLabelsFound >= 1)
				{
                   if (appDataRef->multiclassDetection) 
                   {
                       // class has to match the picture
                       int expectedClass;
                       // Class number is last char of filename followed by a 3 char extension
                       //debug
                       std::cout << result.original->sub.path.filename << std::endl;
                       int idx = result.original->sub.path.filename.length() - 5;
                       std::string eClassNum = result.original->sub.path.filename.substr(idx, 1);
                       std::cout << eClassNum << std::endl;
                       expectedClass = atoi(eClassNum.c_str());
                       if (expectedClass == atoi(className.c_str()))
                       {
                           appDataRef->detectionsVerified.push_back(true);
                       }else
                       {

                           localAndClientMsg(VLogger::INFO, NULL, "Found class: %d did not match expected %d\n", 
                                             atoi(className.c_str()), expectedClass);
                           appDataRef->detectionsVerified.push_back(false);
                       }
                   }else
                       appDataRef->detectionsVerified.push_back(true);
				}
				else
				{
				   appDataRef->detectionsVerified.push_back(false);
				}
			}

		}
	};


	void estimatedTotalRuntime(::Ice::Int seconds, const ::Ice::Current& current)
	{
		localAndClientMsg(VLogger::INFO, NULL, "Service reports it will take %d (sec).\n", seconds);
	};

	void estimatedRuntimeLeft(::Ice::Int seconds, const ::Ice::Current& current) {};

	void completedProcessing(const ::Ice::Current& current)
	{
		localAndClientMsg(VLogger::INFO, NULL, "Detector service complete.\n");
	};

	void message(::Ice::Int level, const ::std::string& messageString, const ::Ice::Current& current) 
	{
	  // append \n if detector didn't append it
		localAndClientMsg(VLogger::DEBUG, NULL, "Detector reports: %s%s", 
				  messageString.c_str(),
				  (messageString.length()==0 || 
				   messageString.c_str()[messageString.length()-1]!='\n')?"\n":"");
	};
};

// =============================================================================
/** callback for asynchronous call of Detector::process via ICE */
class FinishedCallback : public IceUtil::Shared
{
public:

	FinishedCallback() : mFinished(false){}

	bool hasFinished()
	{
		return mFinished;
	}

	void finished(const Ice::AsyncResultPtr& result)
	{
		DetectorPrx detector = DetectorPrx::uncheckedCast(result->getProxy());

		try
		{
			detector->end_process(result);
			localAndClientMsg(VLogger::DEBUG, NULL, "FinishedCallback: detector finished.\n");
		}
		catch (const Ice::Exception& e)
		{
			localAndClientMsg(VLogger::WARN, NULL, "Exception: %s\n", 
				 e.ice_name().c_str());
		}
		mFinished = true;
	}

private:
	bool mFinished;
};

typedef IceUtil::Handle<FinishedCallback> FinishedCallbackPtr;

// =============================================================================
// Simple client application to connect to the IceBox and test out a detector, also handles callbacks
class ClientApp : public Ice::Application
{
public:
	std::string m_detectorData;
	std::string m_detectorName;
	Ice::Identity ident;
	ClientAppData *appData;
    FilePath detectorData;

  ClientApp(ClientAppData *refAppData);
  virtual int run(int, char*[]);
  DetectorPrx initIceConnection(std::string detectorNameStr);
  int verification(std::string testFilePath, DetectorPrx detector);
  int SBDResultsOK();
  void parseOption(char* optionArgument, std::string &refConfigStr);
  int initializeDetector(DetectorPrx detector);

private:
    int SBD_TRUTH_TOLERANCE;
};

/** print usage information
 */
void usage( char* argv[] )
{
  printf("usage: %s data detector folder [config [verifyresults]]\n"
	 "       data:          archive with DetectorData\n"
	 "       detector:      name of the detector\n"
	 "       folder:        which images or videos to process\n"
	 "       config:        configuration file for ICE\n"
	 "       verifyresults: compare results against known-good results\n",
	 argv[0]
	 );
}

// =============================================================================
// Usage: <exe filename> <zipFile from config> <name of the detector> 
//        [path to directory of images to process] [flag: 'verifyresults']
int main(int argc, char* argv[])
{
	int mainResult;

#if 0
    std::string val = getenv( "PATH" );
    cout << "PATH=" << val << endl;
    return -1;
#endif //0

	try
	{
		if((argc < 2) || (argc > 6)) 
		{	// Wrong arguments
			localAndClientMsg(VLogger::WARN, NULL, 
					  "Too many or too few arguments (%d).\n", argc);
			usage( argv );
			return EXIT_FAILURE;
		}

		ClientAppData appData;
		ClientApp app(&appData);
		int res = 0;
		// Assumed to run from CVAC root, or use optional argument to specify
		std::string configstr = "config.client";
		app.m_detectorName = std::string(argv[1]);

        if(app.m_detectorName.find("BagOfWordsUSKOCA")!=std::string::npos)
		{
			appData.multiclassDetection = true;
		}

		if(app.m_detectorName == "ShotBoundaryDetector")
		{
      localAndClientMsg(VLogger::DEBUG_2, NULL, "Verifying shot boundary detector\n");
			appData.videoInputType = true;
			appData.needSBDVerified = true;
		}

		if((4 == argc) || (5 == argc)) 
		{	// Set m_detectorData or configstr based on option type
			app.parseOption(argv[3], configstr);
		}

		localAndClientMsg(VLogger::DEBUG, NULL, "config file: %s\n", configstr.c_str());
		localAndClientMsg(VLogger::DEBUG, NULL, "cwd: %s\n", getCurrentWorkingDirectory().c_str());

		mainResult = app.main(argc, argv, configstr.c_str());
	}
	catch (const Ice::FileException& e)
	{
		localAndClientMsg(VLogger::WARN, NULL, "Can't find required file: %s\n", e.what());
		//sleep(1000); // Keep window open without breakpoint
		return EXIT_FAILURE;
	}
	catch (const Ice::Exception& e)
	{
		localAndClientMsg(VLogger::WARN, NULL, "Ice::Exception in run method: %s\n", e.what());
		//sleep(1000);
		return EXIT_FAILURE;
	}
	catch (std::exception &e)
	{
		localAndClientMsg(VLogger::WARN, NULL, "Error: '%s'.  %s\n", argv[1], e.what());
		//sleep(1000);
		return EXIT_FAILURE;
	}
	localAndClientMsg(VLogger::DEBUG, NULL, "End result was: %d\n", mainResult);
	return mainResult;
}

ClientApp::ClientApp(ClientAppData *refAppData) :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling),
    SBD_TRUTH_TOLERANCE(3)
{
	appData = refAppData;
}


 /// usage:
    ///IceBoxTestClientApp.exe <detector> <directory>
    /// <detector>: The name of the detector (found in the config.client file)
    /// <directory>: The full path to a folder containing images to process
    /// Returns 0 on success ('EXIT_SUCCESS'), and 1 on failure ('EXIT_FAILURE')
DetectorPrx ClientApp::initIceConnection(std::string detectorNameStr)
{  // Connect to the Ice Service
	Ice::PropertiesPtr props = communicator()->getProperties();
    std::string proxStr = detectorNameStr + ".Proxy";
    DetectorPrx detector = NULL;
    try
    {
        detector = DetectorPrx::checkedCast(
                               communicator()->propertyToProxy(proxStr)->ice_twoway());
    }
    catch (const IceUtil::NullHandleException& e)
    {
       localAndClientMsg(VLogger::WARN, NULL, "Invalid proxy: '%s'. %s\n", 
               detectorNameStr.c_str(), e.what());
       return NULL;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("");
    ident.name = IceUtil::generateUUID();
    ident.category = "";
    DetectorCallbackHandlerPtr cr = new CallbackHandlerI(appData);
    adapter->add(cr, ident);
    adapter->activate();
    detector->ice_getConnection()->setAdapter(adapter);    

    // note that we need an ObjectAdapter to permit bidirectional communication
    // if we want to get past firewalls without Glacier2

    localAndClientMsg(VLogger::DEBUG, NULL, "IceBox test client: created callbackHandler proxy\n");
                
    if(!detector)
    {
		localAndClientMsg(VLogger::WARN, NULL, "%s: invalid proxy \n", appName());
		return NULL;
    }
    else
		return detector;  // Success

#if 0
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("");
    Ice::Identity ident;
    ident.name = IceUtil::generateUUID();
    ident.category = "";
    CallbackReceiverPtr cr = new CallbackReceiverI;
    adapter->add(cr, ident);
    adapter->activate();
    detector->ice_getConnection()->setAdapter(adapter);
    printf("about to call addClient\n");
//    RunSet runSet;
//    detector->process(ident, runSet);
    detector->addClient(ident);
    printf("just called addClient\n");
    communicator()->waitForShutdown();

    return 0;
#elif 0
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("");
    Ice::Identity ident;
    ident.name = IceUtil::generateUUID();
    ident.category = "";
    DetectorCallbackHandlerPtr cr = new CallbackHandlerI;
    adapter->add(cr, ident);
    adapter->activate();
    detector->ice_getConnection()->setAdapter(adapter);
    printf("about to call process\n");
    RunSet runSet;
    detector->process(ident, runSet);
//    detector->addClient(ident);
    printf("just called process\n");
    communicator()->waitForShutdown();

    return 0;
#endif

}



int ClientApp::run(int argc, char* argv[])
{
  string verbStr = communicator()->getProperties()->getProperty("CVAC.ClientVerbosity");
  if (!verbStr.empty())
  {
      getVLogger().setLocalVerbosityLevel( verbStr );
  }
  localAndClientMsg(VLogger::DEBUG_2, NULL, "App 2nd arg (testFileFolder): %s\n", argv[2]);
  std::string testFileFolder = std::string(argv[2]);

  if(argc < 3 || argc > 5)  // Warn wrong args
  {
	localAndClientMsg(VLogger::ERROR, NULL,
	"not enough or too many command line arguments\n", appName());
	localAndClientMsg(VLogger::ERROR, NULL,
	"<exe filename> <detector xml or zipFile from config> <name of the detector> [path to directory of images to process] [flag: 'verifyresults']\n");
	return EXIT_FAILURE;
  }
  ResultSet result;
  RunSet runSet;
  Ice::PropertiesPtr props = communicator()->getProperties();
  std::string dataDir = props->getProperty("CVAC.DataDir");
		
  PurposedDirectoryPtr dir = new PurposedDirectory();

  localAndClientMsg(VLogger::DEBUG, NULL, "IceBoxTestClient-mainLoop setting RunSet dir: %s\n", testFileFolder.c_str());
	  dir->directory.relativePath = std::string(testFileFolder.c_str());
  runSet.purposedLists.push_back(dir);

  std::string _detectorName = argv[1];
  if(appData->videoInputType) {
		 dir->fileSuffixes.push_back("mpg");
         localAndClientMsg(VLogger::DEBUG_2, NULL, "Using (.mpg) extension.\n");
  }
  else {
		 dir->fileSuffixes.push_back("jpg");
  }

  dir->recursive = true;
  // get the model file
  std::string filenameStr = m_detectorName + ".DetectorFilename";
  
  std::string filename = props->getProperty(filenameStr);
		
  if (filename.length() == 0)
  {
		localAndClientMsg(VLogger::WARN, NULL,
					  "No .DetectorFilename pair found for %s.\n", 
					  m_detectorName.c_str());
		return EXIT_FAILURE;
  }
  FilePath file;
  if ((filename.length() > 1 && filename[1] == ':' )||
			filename[0] == '/' ||
			filename[0] == '\\')
  {  // absolute path
			int idx = filename.find_last_of('/');
			file.directory.relativePath = filename.substr(0, idx);
			file.filename = filename.substr(idx+1, filename.length() - idx);
  }
  else
  { //Add the current directory
			file.directory.relativePath = "detectors";
			file.filename = filename;
  }
	
 
  
  // Connect to detector
  DetectorPrx detector = initIceConnection(m_detectorName);
  if(NULL == detector.get()) {
    localAndClientMsg(VLogger::ERROR, NULL, "Could not connect to CVAC Ice Services\n");
    return EXIT_FAILURE;
  }

  int resultInit = initializeDetector(detector);
  if((EXIT_SUCCESS != resultInit)) {
      
    localAndClientMsg(VLogger::ERROR, NULL, "Detector->isInitialized() failed.  Aborting IceTestClient.\n");
    return(EXIT_FAILURE);
  }
  else {
    localAndClientMsg(VLogger::DEBUG, NULL, "IceBoxTestClient detector: initialization OK. \n");
  }

  //CTest requested?
  if(5 == argc) 
  {
	  std::string lastArgStr = std::string(argv[4]);
	  if(-1 == lastArgStr.find("verifyresults")) 
	  {
		  localAndClientMsg(VLogger::ERROR, NULL, "Invalid value for argument #5: %s.  CMakeLists.txt must specify token: 'verifyresults'\n", argv[4]);
		  return(EXIT_FAILURE);
	  }
		
	  int verResult = verification(testFileFolder, detector);
	  if(EXIT_SUCCESS != verResult)
	  {
		  return(EXIT_FAILURE);
	  }
  }
  else
	{
	  // Main Loop: build RunSet from 'testFileFolder'
	  RunSet runSet;
	  Ice::PropertiesPtr props = communicator()->getProperties();
	  std::string dataDir = props->getProperty("CVAC.DataDir");
		
	  PurposedDirectoryPtr dir = new PurposedDirectory();

    localAndClientMsg(VLogger::DEBUG, NULL, "IceBoxTestClient-mainLoop setting RunSet dir: %s\n", testFileFolder.c_str());
	  dir->directory.relativePath = std::string(testFileFolder.c_str());
	  runSet.purposedLists.push_back(dir);

	  std::string _detectorName = argv[1];
    if(appData->videoInputType) {
		  dir->fileSuffixes.push_back("mpg");
      localAndClientMsg(VLogger::DEBUG_2, NULL, "Using (.mpg) extension.\n");
    }
    else {
		  dir->fileSuffixes.push_back("jpg");
    }

	  dir->recursive = true;
		
	  try
	  {	// Create our callback class so that we can be informed when process completes
		  FinishedCallbackPtr finishCallback = new FinishedCallback();
		  Ice::CallbackPtr finishedAsync = Ice::newCallback(finishCallback, &FinishedCallback::finished);
	      cvac::DetectorProperties dprops;
		  Ice::AsyncResultPtr asyncResult = detector->begin_process(ident, runSet, detectorData, dprops, finishedAsync);
		  localAndClientMsg(VLogger::DEBUG, NULL, "IceBox test client: initiated processing\n");
			
		  // end_myFunction should be call from the "finished" callback
		  //detector->end_process(asyncResult);
			
		  // Wait for the processing to complete before exiting the app
		  while (!finishCallback->hasFinished())
		  {
			  sleep(100);
		  }
		  localAndClientMsg(VLogger::DEBUG_2, NULL, "IceBox test client: finished processing\n");
	  }
	  catch (const Ice::Exception& ex)
	  {
		  localAndClientMsg(VLogger::WARN, NULL, "Ice- name():  %s\n", ex.ice_name().c_str());
		  localAndClientMsg(VLogger::WARN, NULL, "Ice- stackTrace(): \n%s\n", ex.ice_stackTrace().c_str());
		  localAndClientMsg(VLogger::WARN, NULL, "%s\n", ex.what());
		  return EXIT_FAILURE;
	  }
  }

  communicator()->shutdown();  // Shut down at the end of either branch
  return EXIT_SUCCESS;
  
  
}


int ClientApp::verification(std::string testFilePath, DetectorPrx detector)
{
	appData->isVerification = true;
	Purpose positive(POSITIVE, 0);
	RunSet runSet;
	std::string suffix;
	if(appData->videoInputType)
	{
		suffix = "mpg";
	}
	else
	{
		suffix = "jpg";
	}
	
	if(appData->multiclassDetection)
	{
		std::string testFileName0 = (m_detectorName + "0." + suffix);
		std::string testFileName1 = (m_detectorName + "1." + suffix);
		std::string testFileName2 = (m_detectorName + "2." + suffix);

		addToRunSet(runSet, testFilePath, testFileName0, Purpose(MULTICLASS, 0));
		addToRunSet(runSet, testFilePath, testFileName1, Purpose(MULTICLASS, 1));
		addToRunSet(runSet, testFilePath, testFileName2, Purpose(MULTICLASS, 2));
	}
	else
	{
    localAndClientMsg(VLogger::DEBUG, NULL, "IceBoxTestClient-verification setting RunSet dir: %s\n", testFilePath.c_str());
		std::string testFileName = (m_detectorName + "." + suffix);
		addToRunSet(runSet, testFilePath, testFileName, positive);  // Path relative to CVAC.data dir
	}
	
	try
	{    // Create our callback class so that we can be informed when process completes
		FinishedCallbackPtr finishCallback = new FinishedCallback();
		Ice::CallbackPtr finishedAsync = Ice::newCallback(finishCallback, &FinishedCallback::finished);
        cvac::DetectorProperties dprops;
		Ice::AsyncResultPtr asyncResult = detector->begin_process(ident, runSet, detectorData, dprops, finishedAsync);
		
		// end_myFunction should be call from the "finished" callback
		//detector->end_process(asyncResult);
		
		// Wait for the processing to complete before exiting the app
		while (!finishCallback->hasFinished())
		{
			sleep(100);
		}
		localAndClientMsg(VLogger::DEBUG_2, NULL, "IceBox test client: finished verification\n");
	}
	catch (const Ice::Exception& ex)
	{
		localAndClientMsg(VLogger::WARN, NULL, "Ice- name():  %s\n", ex.ice_name().c_str());
		localAndClientMsg(VLogger::WARN, NULL, "Ice- stackTrace(): \n%s\n", ex.ice_stackTrace().c_str());
		localAndClientMsg(VLogger::WARN, NULL, "%s\n", ex.what());
		return EXIT_FAILURE;
	}
	
	communicator()->shutdown();
	if(appData->needSBDVerified)
	{
		if(false == SBDResultsOK())
		{
			appData->detectionsVerified.push_back(false);  // Add SBD-test failure
		}
	}

  // Scan for missed detections
  bool missedDetections = false;
  while(appData->detectionsVerified.size() > 0) {

    bool element = appData->detectionsVerified.back();
    appData->detectionsVerified.pop_back();

    if(false == element) {
      missedDetections = true;
    }
  }

  if(missedDetections) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


int ClientApp::SBDResultsOK()
{
	try
	{
		// Get line counts from both files
		std::ifstream truthTmpFile("data/CTest/shotBoundaryDetector_result.txt");
		std::ifstream resultTmpFile("data/CTest/shotBoundaryDetector.truth");
		int truthLines = std::count(std::istreambuf_iterator<char>(truthTmpFile),
			std::istreambuf_iterator<char>(), '\n');
		int resultLines = std::count(std::istreambuf_iterator<char>(resultTmpFile),
			std::istreambuf_iterator<char>(), '\n');
		
		truthTmpFile.close();
		resultTmpFile.close();
		
		ifstream sbdResults("data/CTest/shotBoundaryDetector_result.txt");
		ifstream sbdTruth("data/CTest/shotBoundaryDetector.truth");
		
		string truthNum, resultNum;
		if(truthLines != resultLines)
		{
			localAndClientMsg(VLogger::WARN, NULL, "Shot boundary detector verification failed.  Different numbers of boundaries between reference and detected output.\n");
			return(false);
		}
		
		if((sbdResults.is_open()) && (sbdTruth.is_open()))
		{
			while(sbdResults.good() && (sbdTruth.good()))
			{
				getline (sbdTruth, truthNum);
				getline (sbdResults, resultNum);
				
				int truth = atoi(truthNum.c_str());
				int result = atoi(resultNum.c_str());
				int truthDistance = abs(truth - result);

				if(truthDistance > SBD_TRUTH_TOLERANCE)
				{
					localAndClientMsg(VLogger::WARN, NULL, "Shot boundary detector verification failed, one or more boundaries outside tolerance.\n");
					return(false);
				}
			}
			sbdResults.close();
		}
	}
	catch(exception e) 
	{
		return(false);
	}
	localAndClientMsg(VLogger::DEBUG_1, NULL, "Shot boundary detector verification complete.\n");
	
	// All lines of result file matched truth
	return(true);
}

void ClientApp::parseOption(char* optionArgument, std::string &refConfigStr)
{
	std::string optionStr = std::string(optionArgument);
	int idx = optionStr.find("zip");
	if (-1 != idx)
	{
		m_detectorData = optionStr; // Optional arg is a zip file
	} 
	else
	{
		refConfigStr = optionStr;  // Override config file with option
	}
}

int ClientApp::initializeDetector(DetectorPrx detector)
{

	localAndClientMsg(VLogger::DEBUG, NULL, "Client using Detector-name: %s\n", detector->ice_getIdentity().name.c_str());
	
	// If a detector dat file was passed then use that instead of getting it from the config file
	if (m_detectorData.length() > 0)
	{
		detectorData.directory.relativePath = getFileDirectory(m_detectorData);
		detectorData.filename = getFileName(m_detectorData);
	}
	else
	{ // Determine which data files to pass as detector data based on .DetectorFilename in 'config.client'
		std::string filenameStr = m_detectorName + ".DetectorFilename";
		Ice::PropertiesPtr props = communicator()->getProperties();
		std::string filename = props->getProperty(filenameStr);
		
		if (filename.length() == 0)
		{
			localAndClientMsg(VLogger::WARN, NULL,
					  "No .DetectorFilename pair found for %s.\n", 
					  m_detectorName.c_str());
			return EXIT_FAILURE;
		}
		
		if ((filename.length() > 1 && filename[1] == ':' )||
			filename[0] == '/' ||
			filename[0] == '\\')
		{  // absolute path
			int idx = filename.find_last_of('/');
			detectorData.directory.relativePath = filename.substr(0, idx);
			detectorData.filename = filename.substr(idx+1, filename.length() - idx);
		}
		else
		{ //Add the current directory
			detectorData.directory.relativePath = "detectors";
			detectorData.filename = filename;
		}
	}
	

  return EXIT_SUCCESS;

}


