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
#include <stdexcept>
#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
#include <util/FileUtils.h>
#include <util/processRunSet.h>
using namespace cvac;

class CallbackHandlerI : public TrainerCallbackHandler
{
public:
	CallbackHandlerI() {};
	~CallbackHandlerI(){};


	virtual void estimatedTotalRuntime(::Ice::Int seconds, const ::Ice::Current& current)
	{
		localAndClientMsg(VLogger::INFO, NULL, "Service says it'll take %d seconds.\n", seconds);
	};
	
   virtual void estimatedRuntimeLeft(::Ice::Int seconds, const ::Ice::Current& current) {};
	
   virtual void completedProcessing(const ::Ice::Current& current)
	{
		localAndClientMsg(VLogger::INFO, NULL, "Trainer service complete.\n");
	};

	virtual void message(::Ice::Int level, const ::std::string& messageString, const ::Ice::Current& current)
	{
		std::cout << messageString;
	};

  ///////////////////////////////////////////////////////////////////////////////
  // Output function for complete Detector
	void createdDetector(const ::DetectorData& ddata, const ::Ice::Current& = ::Ice::Current() )
	{
		localAndClientMsg(VLogger::INFO, NULL, "Trainer is finished with creating a detector/data/model, type is ");
		switch (ddata.type)
		{
			case ::cvac::BYTES:
			  localAndClientMsg(VLogger::INFO, NULL, "bytes\n");
			  break;
			case ::cvac::FILE:
			  localAndClientMsg(VLogger::INFO, NULL, "file: %s/%s\n", 
														   ddata.file.directory.relativePath.c_str(), 
														   ddata.file.filename.c_str());
			  break;
			case ::cvac::PROVIDER:
			  localAndClientMsg(VLogger::INFO, NULL, "provider\n");
			  break;
			default:    // In case exception is dropped by ice
			  localAndClientMsg(VLogger::WARN, NULL, "Unknown DetectorData type.\n");
			  throw new std::runtime_error("Unknown DetectorData type");
		}
	}
};


// Simple client application to connect to the IceBox and test out a MultiBoost object
class TrainerClientApp : public Ice::Application
{
public:

	TrainerClientApp();

	virtual int run(int, char*[]);
	/** To obtain a frame list of a corresponding video file:  
	*	this function may be generalized to deal with video files.
	*/
	bool parseFrame_SBD(std::string _filepath, std::string _filename, std::vector<long>& _vFrameList);
};

int main(int argc, char* argv[])
{
	TrainerClientApp app;
	return app.main(argc, argv, "config.client");
}

TrainerClientApp::TrainerClientApp() :
//
// Since this is an interactive demo we don't want any signal
// handling.
//
Ice::Application(Ice::NoSignalHandling)
{
}



bool TrainerClientApp::parseFrame_SBD(std::string _filepath, std::string _filename, std::vector<long>& _vFrameList)
{
	_vFrameList.clear();

	std::string _fullName = _filepath + "/" + _filename;

	std::ifstream _fileLog(_fullName.c_str());
	if(!_fileLog.is_open())
	{	
		return false;
	}

	char			_buf[255];
	std::string			_bufString;
	std::istringstream	_bufStream;	

	do
	{
		_fileLog.getline(_buf, 255);
		_bufStream.clear();	
		_bufStream.str(_buf);
		_bufStream >> _bufString;

		if (_bufString == "#")
			continue;
		else if(_bufString == "\0")	
			continue;

		_vFrameList.push_back(atoi(_bufString.c_str()));		

	}while(!_fileLog.eof());

	_fileLog.close();

	return true;
}

/// usage:
///TrainerClientApp.exe <trainer> 
/// <trainer>: The name of the trainer (found in the config.client file)
int TrainerClientApp::run(int argc, char* argv[])
{
	//argv[1]: proxyName
	//argv[2]: directory including training data

	if(argc != 3)
	{
		localAndClientMsg(VLogger::WARN, NULL, "Expected proxy name, then 3 training-related directories.  Missing: <trainer-string in config file>?");
		return EXIT_FAILURE;
	}

	//////////////////////////////////////////////////////////////////////////
	// Check Client Proxy
	const std::string trainerConfigStr = std::string(argv[1]);
	std::string proxStr = trainerConfigStr + ".Proxy";

	DetectorTrainerPrx trainer = NULL;
	try
	{
		trainer = DetectorTrainerPrx::checkedCast(
			communicator()->propertyToProxy(proxStr)->ice_twoway());
	}
	catch (const IceUtil::NullHandleException& e)
	{
		localAndClientMsg(VLogger::WARN, NULL, "Invalid proxy: '%s'. Exception: %s\n", trainerConfigStr.c_str(), e.what());
		return EXIT_FAILURE;
	}
	
	if(!trainer)
	{
		cerr << appName() << ": invalid proxy" << endl;
		return EXIT_FAILURE;
	}

	TrainerCallbackHandlerPtr callbackHandler = new CallbackHandlerI();
	Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TrainerCallback.Client");

	Ice::Identity _ident;
	_ident.name = IceUtil::generateUUID();
	_ident.category = "";

	adapter->add(callbackHandler, _ident);
	adapter->activate();
	trainer->ice_getConnection()->setAdapter(adapter);
	///////////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////
	// Start - Classify in terms of nClass and mediaType
	bool fMultiClass = false;
	bool fImage = false;	//lekomin: obtain from MediaType

	std::string trainerName = trainer->ice_getIdentity().name;
    printf("Using trainer %s\n", trainerName.c_str());
	if(trainerName == "bowTrain")
	{
		fMultiClass = true;
		fImage = true;
	}
	else if(trainerName == "SBDTrain")
	{
		fMultiClass = true;
		fImage = false;
	}
	else if( trainer->ice_getIdentity().name == "OpenCVCascadeTrainer" )
	{
		fMultiClass = false;
		fImage = true;
	}
	else
	{
		localAndClientMsg(VLogger::WARN, NULL, "No valid trainer\n");
		return EXIT_FAILURE;
	}
	std::string _classStr,_mediaStr;
	_classStr = (fMultiClass)?"Multiclass":"Non-multiclass";
	_mediaStr = (fImage)?"Image":"Video";
	localAndClientMsg(VLogger::DEBUG_1, NULL, "Identified trainer type as %s and %s: %s\n",_classStr.c_str(),_mediaStr.c_str(),trainerName.c_str());	
	// End - Classify in terms of nClass and mediaType
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Start - Fill the runSet
	RunSet runSet;
//	lekomin - Revival
 	Ice::PropertiesPtr props = (adapter->getCommunicator()->getProperties());
 	std::string CVAC_DataDir = props->getProperty("CVAC.DataDir");	//if it nee

	if(fImage)
	{
		if(fMultiClass)	//ex. Bow
		{
			BBox* pbox;

			addToRunSet(runSet, "trainImg/us", "us001.jpg", 1);
			pbox = new BBox(26,40,587,373);									
			addToRunSet(runSet, "trainImg/us", "us002.jpg", 1, pbox);
			pbox = new BBox(44,105,561,311);
			addToRunSet(runSet, "trainImg/us", "us003.jpg", 1, pbox);
			pbox = new BBox(121,75,519,356);
 			addToRunSet(runSet, "trainImg/us", "us004.jpg", 1, pbox);
			pbox = new BBox(56,60,564,401);
 			addToRunSet(runSet, "trainImg/us", "us005.jpg", 1, pbox);

 			
			addToRunSet(runSet, "trainImg/kr", "kr001.jpg", 2);
			addToRunSet(runSet, "trainImg/kr", "kr002.jpg", 2);
	 		addToRunSet(runSet, "trainImg/kr", "kr003.jpg", 2);
			//pbox = new BBox(0,83,640,397);
 			addToRunSet(runSet, "trainImg/kr", "kr004.jpg", 2);
			addToRunSet(runSet, "trainImg/kr", "kr005.jpg", 2);
			
			
			//pbox = new BBox(58,149,531,184);
			addToRunSet(runSet, "trainImg/ca", "ca0003.jpg", 3);
			//pbox = new BBox(109,158,375,106);
			addToRunSet(runSet, "trainImg/ca", "ca0004.jpg", 3);
			//pbox = new BBox(138,191,333,155);
			addToRunSet(runSet, "trainImg/ca", "ca0005.jpg", 3);
			//pbox = new BBox(138,100,466,224);
			addToRunSet(runSet, "trainImg/ca", "ca0006.jpg", 3);
			//pbox = new BBox(55,4,528,256);
			addToRunSet(runSet, "trainImg/ca", "ca0007.jpg", 3);
		}
		else	// CVAC_OpenCV_Trainer
		{
			Purpose positive;
			positive.ptype = POSITIVE;
			Purpose negative;
			negative.ptype = NEGATIVE;
			addToRunSet(runSet, "../../../../../Analyst_Media/VehicleTraining/images/small_sample", "IMG00005_003_000.jpg", positive);
			addToRunSet(runSet, "../../../../../Analyst_Media/VehicleTraining/images/small_sample", "IMG00005_008_000.jpg", positive);
			addToRunSet(runSet, "../../../../../Analyst_Media/VehicleTraining/images/small_sample", "IMG00013_008_000.jpg", positive);
			addToRunSet(runSet, "../../../../../Analyst_Media/VehicleTraining/images/small_sample", "IMG00013_009_000.jpg", positive);
			//addToRunSet(runSet, "../../../../../Analyst_Media/VehicleTraining/images/small_sample", "IMG00014_001_000.jpg", positive);

			addToRunSet(runSet, "../../../../../Analyst_Media/VehicleTraining/images/small_background", "IMG00006.jpg", negative);
		}
	}
	else
	{
		if(fMultiClass)	//ex. SBD
		{
			std::vector<long> _vFrameListInput;

			PurposedLabelableSeq* purposeVideo = new PurposedLabelableSeq();
			purposeVideo->pur.ptype = MULTICLASS;		
			runSet.purposedLists.push_back(purposeVideo);

			//////////////////////////////////////////////////////////////////////////
			// Repeat the next block if there are multiple videos
			{				
				LabeledTrack* videoLabel = new LabeledTrack();								
				videoLabel->sub.isImage = false; 
				videoLabel->sub.isVideo = true;	
				videoLabel->sub.path.filename = "shortTrain.mpg";							
				videoLabel->sub.path.directory.relativePath = std::string(argv[2]);				
				videoLabel->interp = cvac::DISCRETE;	//If a enum. variable is not initialized, it causes error in debug mode.
				
 				std::string _filepath = getFSPath(videoLabel->sub.path, CVAC_DataDir);							
 				parseFrame_SBD(_filepath,"AnnotationShortTrain.txt",_vFrameListInput);								

				for(unsigned int k=0;k<_vFrameListInput.size();k++)
				{	
					FrameLocation _frameIdx;
					_frameIdx.frame.time = _vFrameListInput[k];
					videoLabel->keyframesLocations.push_back(_frameIdx);
				}				
 				purposeVideo->labeledArtifacts.push_back(videoLabel);	
			}
		}
		else	//ex. None
		{
			localAndClientMsg(VLogger::WARN, NULL, "No valid trainer\n");
			return EXIT_FAILURE;
		}
	}
	// End - Classify in terms of nClass and mediaType
	//////////////////////////////////////////////////////////////////////////	

	try
	{
        trainer->initialize(0);	//0:verbosity        
		//localAndClientMsg(VLogger::DEBUG_3, NULL, "Test_Trainer invoking Ice call: 'process(receiver, runSet)'\n");        
        
		trainer->process(_ident, runSet);        
        //localAndClientMsg(VLogger::DEBUG_3, NULL, "Test_Trainer completed Ice call: 'process(receiver, runSet)'\n");	
	}
	catch (const Ice::Exception& ex)
	{
		localAndClientMsg(VLogger::WARN, NULL, "Exception: %s\n", ex.what());
		return EXIT_FAILURE;
	}

	communicator()->shutdown();

	return EXIT_SUCCESS;
}


