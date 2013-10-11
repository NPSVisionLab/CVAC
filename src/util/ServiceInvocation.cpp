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
#include <Ice/Communicator.h>
#include <Services.h>
#include <vector>
#include <util/FileUtils.h>
#include <util/Timing.h>
#include <util/ServiceInvocation.h>
/** Utilities for invoking CVAC services from C/C++
 */

using namespace cvac;

class CallbackHandlerI : public DetectorCallbackHandler
{
public:
  ResultSet rs;
  CallbackHandlerI()  {}
  ~CallbackHandlerI() {}

  void foundNewResults(const ResultSet& newResults, const ::Ice::Current& current)
  {
    rs.results.insert( rs.results.end(),
                       newResults.results.begin(), newResults.results.end() );
  }

  void estimatedTotalRuntime(::Ice::Int seconds, const ::Ice::Current& current) {}
  void estimatedRuntimeLeft(::Ice::Int seconds, const ::Ice::Current& current) {}
  void completedProcessing(const ::Ice::Current& current) {}
  void message(::Ice::Int level, const ::std::string& messageString, const ::Ice::Current& current) {}
  void cancelled(const Ice::Current&) {}
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
      }
    catch (const Ice::Exception& e)
      {
        fprintf( stderr, "Exception: %s\n", e.ice_name().c_str());
      }
    mFinished = true;
  }
  
private:
  bool mFinished;
};

typedef IceUtil::Handle<FinishedCallback> FinishedCallbackPtr;

/** Connect to the Ice Service
 */
DetectorPrx initIceConnection(std::string detectorNameStr, Ice::Identity& det_cb,
                              DetectorCallbackHandlerPtr cr)
{
  Ice::CommunicatorPtr comm = Ice::Application::communicator();
  Ice::PropertiesPtr props = comm->getProperties();
  std::string proxStr = detectorNameStr + ".Proxy";
  DetectorPrx detector = NULL;
  try
  {
    detector = DetectorPrx::checkedCast(
        comm->propertyToProxy(proxStr)->ice_twoway());
  }
  catch (const IceUtil::NullHandleException& e)
  {
    localAndClientMsg( VLogger::ERROR, NULL, "Invalid proxy: '%s'. %s\n", 
                       detectorNameStr.c_str(), e.what());
    return NULL;
  }

  Ice::ObjectAdapterPtr adapter = comm->createObjectAdapter("");
  det_cb.name = IceUtil::generateUUID();
  det_cb.category = "";
  adapter->add(cr, det_cb);
  adapter->activate();
  detector->ice_getConnection()->setAdapter(adapter);    
  
  // note that we need an ObjectAdapter to permit bidirectional communication
  // if we want to get past firewalls without Glacier2
  
  return detector;  // Success
}

/** A convenience function that creates the callback,
 *  collects results, and makes them available in the
 *  return argument.
 */
ResultSet cvac::detect( const std::string& algorithm,
                    const cvac::RunSet& runset,
                    const cvac::FilePath& model,
                    const cvac::DetectorProperties* detprops )
{
  // Connect to detector
  Ice::Identity det_cb;
  CallbackHandlerI* cr = new CallbackHandlerI();
  DetectorPrx detector = initIceConnection( algorithm, det_cb, cr );
  if(NULL == detector.get())
  {
    localAndClientMsg( VLogger::ERROR, NULL, "Could not connect to CVAC Ice Services" );
    return ResultSet();
  }
  cvac::DetectorProperties dprops;
  // If user did not supply any detector properties then provide default one.
  if (NULL == detprops)
  {
      detprops = & dprops;   
  }

  Ice::CommunicatorPtr comm = Ice::Application::communicator();
  Ice::PropertiesPtr iceprops = comm->getProperties();
  std::string dataDir = iceprops->getProperty("CVAC.DataDir");
/*
 TODO: "I think we don't have to use the FinishCallback at all because we don't want to"
    "invoke process asynchronously.  Not sure why the choice was made to do so in"
    "detectorClient.  Please try without first: just call "
    "detector->process(det_cb, runSet);";
  */
  try
    {	// Create our callback class so that we can be informed when process completes
      FinishedCallbackPtr finishCallback = new FinishedCallback();
      Ice::CallbackPtr finishedAsync = Ice::newCallback(finishCallback, &FinishedCallback::finished);
      
      Ice::AsyncResultPtr asyncResult =
        detector->begin_process(det_cb, runset, model, *detprops, finishedAsync);
      
      // end_myFunction should be call from the "finished" callback
      //detector->end_process(asyncResult);
      
      // Wait for the processing to complete before exiting the app
      while (!finishCallback->hasFinished())
      {
        cvac::sleep(100);
      }
    }
  catch (const Ice::Exception& ex)
    {
      throw ex;
    }

  comm->shutdown();  // Shut down at the end of either branch
  return cr->rs;
}

