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
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/ServiceMan.h>

#include "opencv2/core/core.hpp"
#include "opencv2/core/internal.hpp"
#include "cv.h"
#include "cascadeclassifier.h"
#include "cvhaartraining.h"

#include "CascadeTrainI.h"

using namespace std;
using namespace cvac;


///////////////////////////////////////////////////////////////////////////////
// This is called by IceBox to get the service to communicate with.
extern "C"
{
  //
  // ServiceManager handles all the icebox interactions so we construct
  // it and set a pointer to our detector.
  //
  ICE_DECLSPEC_EXPORT IceBox::Service* create(Ice::CommunicatorPtr communicator)
  {
    ServiceManager *sMan = new ServiceManager();
    CascadeTrainI *cascade = new CascadeTrainI(sMan);
    sMan->setService(cascade, cascade->getName());
    return (::IceBox::Service *) sMan->getIceService();

  }
}

CascadeTrainI::CascadeTrainI(ServiceManager *serv)
  : fInitialized(false)
{
  mServiceMan = serv;	
}

CascadeTrainI::~CascadeTrainI()
{
}

void CascadeTrainI::initialize(::Ice::Int verbosity,const ::Ice::Current& current)
{
  // Obtain CVAC verbosity
  Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
  string verbStr = props->getProperty("CVAC.ServicesVerbosity");
  if (!verbStr.empty())
  {
    vLogger.setLocalVerbosityLevel( verbStr );
  }
  
  fInitialized = true;
}

bool CascadeTrainI::isInitialized(const ::Ice::Current& current)
{
  return fInitialized;
}

void CascadeTrainI::destroy(const ::Ice::Current& current)
{
  fInitialized = false;
}
std::string CascadeTrainI::getName(const ::Ice::Current& current)
{
  return "CascadeTrain";
}
std::string CascadeTrainI::getDescription(const ::Ice::Current& current)
{
  return "CascadeTrain: OpenCV Cascade trainer";
}

void CascadeTrainI::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{
}

::TrainerPropertiesPrx CascadeTrainI::getTrainerProperties(const ::Ice::Current &current)
{
  return NULL;
}

// TODO: move from FileUtilsTest into FileUtils.h/cpp
std::string getTempFilename( const std::string& basedir="" ) { return "TODO"; }

// TODO: change to K's class name
class RunSetWrapper {
public:
  RunSetWrapper( const ::RunSet& runset ){}
};

// TODO: make this a member function
void writeBgFile( const RunSetWrapper& rsw, const string& bgFilename, int* pNumNeg )
{
  // TODO: iterate over NEGATIVE purposes only, count numNeg, write 
  // file names to bgFilename as we did for the old OpenCV Performance training;
  // something like:
  // Constraints con; // has a bunch of defaults
  // con.compatiblePurpose = NEGATIVE;
  // con.substrateType = IMAGES;
  // con.mimeTypes = { "jpg", "png", "bmp" };
  // con.spacesInFilenamesPermitted = false;
  // LabelableIterator it = rsw.iterator( con );
  // for labelable in it ...
}

// TODO: move to header
class SamplesParams
{
public:
  int numSamples;
  int width;
  int height;
};

// TODO: make this a member function
void createSamples( const RunSetWrapper& rsw, const SamplesParams& params,
                    const string& vecFilename, int* pNumPos )
{
  // TODO: put this file into tempDir (member variable? parameter?)
  string infoFilename = "cascade_positives.txt";
  // TODO: similar to above, iterate over rsw but this time only POSITIVE
  // see the code from the old OpenCV Performance on what to create
  bool showsamples = false;
  *pNumPos = cvCreateTrainingSamplesFromInfo( infoFilename.c_str(), vecFilename.c_str(), 
                                              params.numSamples, showsamples,
                                              params.width, params.height
                                             );
}

// TODO: move to header
class TrainParams
{
public:
  int numStages;
  int featureType; // CvFeatureParams::HAAR, LBP, or HOG
  int boost_type;
  float minHitRate;
  float maxFalseAlarm;
  float weight_trim_rate;
  int max_depth;
  int weak_count;
};

// TODO: make this a member function
void createClassifier( const string& tempDir, const string& vecFname, const string& bgName,
                       int numPos, int numNeg, const TrainParams& trainParams )
{
  CvCascadeClassifier classifier;
  int precalcValBufSize = 256,
      precalcIdxBufSize = 256;
  bool baseFormatSave = false;
  CvCascadeParams cascadeParams;
  CvCascadeBoostParams stageParams;
  Ptr<CvFeatureParams> featureParams[] = { Ptr<CvFeatureParams>(new CvHaarFeatureParams),
                                           Ptr<CvFeatureParams>(new CvLBPFeatureParams),
                                           Ptr<CvFeatureParams>(new CvHOGFeatureParams)
                                         };
  cascadeParams.featureType = trainParams.featureType;
  stageParams.boost_type = trainParams.boost_type;

  classifier.train( tempDir,
                    vecFname,
                    bgName,
                    numPos, numNeg,
                    precalcValBufSize, precalcIdxBufSize,
                    trainParams.numStages,
                    cascadeParams,
                    *featureParams[cascadeParams.featureType],
                    stageParams,
                    baseFormatSave );  
}

void CascadeTrainI::process(const Ice::Identity &client,const ::RunSet& runset,const ::Ice::Current& current)
{	
  TrainerCallbackHandlerPrx _callback =
    TrainerCallbackHandlerPrx::uncheckedCast(current.con->createProxy(client)->ice_oneway());		

  Ice::PropertiesPtr props = (current.adapter->getCommunicator()->getProperties());
  std::string CVAC_DataDir = props->getProperty("CVAC.DataDir");

  if(runset.purposedLists.size() == 0)
  {
    string _resStr = "Error: no data (runset) for processing\n";
    localAndClientMsg(VLogger::WARN, _callback, _resStr.c_str());
    return;
  }

  // Iterate over runset, inserting each POSITIVE Labelable into
  // the input file to "createsamples".  Add each NEGATIVE into
  // the bgFile.  Put both created files into a tempdir.
  // TODO:
  std::string tempDir = getTempFilename( CVAC_DataDir );
  makeDirectory( tempDir );
  RunSetWrapper rsw( runset );
  string bgName = tempDir + "/cascade_negatives.txt";
  int numNeg = 0;
  writeBgFile( rsw, bgName, &numNeg );

  // set parameters to createsamples
  SamplesParams samplesParams;
  samplesParams.numSamples = 1000;
  samplesParams.width = 5; // TODO: get this from this->TrainerProperties (see Services.ice)
  samplesParams.height = 5; // TODO

  // run createsamples
  std::string vecFname = "cascade_positives.vec";
  int numPos = 0;
  createSamples( rsw, samplesParams, vecFname, &numPos );

  // invoke the actual training
  TrainParams trainParams;
  trainParams.numStages = 20;
  trainParams.featureType = CvFeatureParams::HAAR; // HAAR, LBP, HOG;
  trainParams.boost_type = CvBoost::GENTLE;  // CvBoost::DISCRETE, REAL, LOGIT
  trainParams.minHitRate = 0.995F;
  trainParams.maxFalseAlarm = 0.5F;
  trainParams.weight_trim_rate;  // TODO: set to default from OpenCV/ml.h
  trainParams.max_depth;  // TODO: set to default from OpenCV/ml.h
  trainParams.weak_count;  // TODO: set to default from OpenCV/ml.h
  createClassifier( tempDir, vecFname, bgName,
                    numPos, numNeg, trainParams );

  // TODO: combine the directories into one XML file
  // there's a haartraining function to do this;

  // return the resulting trained model
  // TODO: zip the resulting file
  DetectorData detectorData;
  detectorData.type = ::cvac::FILE;
  detectorData.file.directory.relativePath = tempDir;
  detectorData.file.filename = "TODO: CascadeTrainResult.xml.zip";
  
  _callback->createdDetector(detectorData);
  
  localAndClientMsg(VLogger::INFO, _callback, "Cascade training done.\n");
}

// TODO: this is the old main function; here only for reference.  remove 
// once no longer needed
int nomain( int argc, char* argv[] )
{
    CvCascadeClassifier classifier;
    String cascadeDirName, vecName, bgName;
    int numPos    = 2000;
    int numNeg    = 1000;
    int numStages = 20;
    int precalcValBufSize = 256,
        precalcIdxBufSize = 256;
    bool baseFormatSave = false;

    CvCascadeParams cascadeParams;
    CvCascadeBoostParams stageParams;
    Ptr<CvFeatureParams> featureParams[] = { Ptr<CvFeatureParams>(new CvHaarFeatureParams),
                                             Ptr<CvFeatureParams>(new CvLBPFeatureParams),
                                             Ptr<CvFeatureParams>(new CvHOGFeatureParams)
                                           };
    int fc = sizeof(featureParams)/sizeof(featureParams[0]);
    if( argc == 1 )
    {
        cout << "Usage: " << argv[0] << endl;
        cout << "  -data <cascade_dir_name>" << endl;
        cout << "  -vec <vec_file_name>" << endl;
        cout << "  -bg <background_file_name>" << endl;
        cout << "  [-numPos <number_of_positive_samples = " << numPos << ">]" << endl;
        cout << "  [-numNeg <number_of_negative_samples = " << numNeg << ">]" << endl;
        cout << "  [-numStages <number_of_stages = " << numStages << ">]" << endl;
        cout << "  [-precalcValBufSize <precalculated_vals_buffer_size_in_Mb = " << precalcValBufSize << ">]" << endl;
        cout << "  [-precalcIdxBufSize <precalculated_idxs_buffer_size_in_Mb = " << precalcIdxBufSize << ">]" << endl;
        cout << "  [-baseFormatSave]" << endl;
        cascadeParams.printDefaults();
        stageParams.printDefaults();
        for( int fi = 0; fi < fc; fi++ )
            featureParams[fi]->printDefaults();
        return 0;
    }

    for( int i = 1; i < argc; i++ )
    {
        bool set = false;
        if( !strcmp( argv[i], "-data" ) )
        {
            cascadeDirName = argv[++i];
        }
        else if( !strcmp( argv[i], "-vec" ) )
        {
            vecName = argv[++i];
        }
        else if( !strcmp( argv[i], "-bg" ) )
        {
            bgName = argv[++i];
        }
        else if( !strcmp( argv[i], "-numPos" ) )
        {
            numPos = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-numNeg" ) )
        {
            numNeg = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-numStages" ) )
        {
            numStages = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-precalcValBufSize" ) )
        {
            precalcValBufSize = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-precalcIdxBufSize" ) )
        {
            precalcIdxBufSize = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-baseFormatSave" ) )
        {
            baseFormatSave = true;
        }
        else if ( cascadeParams.scanAttr( argv[i], argv[i+1] ) ) { i++; }
        else if ( stageParams.scanAttr( argv[i], argv[i+1] ) ) { i++; }
        else if ( !set )
        {
            for( int fi = 0; fi < fc; fi++ )
            {
                set = featureParams[fi]->scanAttr(argv[i], argv[i+1]);
                if ( !set )
                {
                    i++;
                    break;
                }
            }
        }
    }

    classifier.train( cascadeDirName,
                      vecName,
                      bgName,
                      numPos, numNeg,
                      precalcValBufSize, precalcIdxBufSize,
                      numStages,
                      cascadeParams,
                      *featureParams[cascadeParams.featureType],
                      stageParams,
                      baseFormatSave );
    return 0;
}
