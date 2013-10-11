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
#include <util/ServiceMan.h>
#include <util/ServiceManI.h>
#include <util/Timing.h>
#include <util/FileUtils.h>
#include <Services.h>
#include <Ice/Ice.h>
#include <IceBox/IceBox.h>

using namespace cvac;
using namespace Ice;

///////////////////////////////////////////////////////////////////////////////
ServiceManagerI::ServiceManagerI( CVAlgorithmService *service,
                                  StartStop *ss )
{
    mService = service;
    mSS = ss;
    mAdapter = NULL;
    mStopState = None;
    mSandbox = NULL;
}

///////////////////////////////////////////////////////////////////////////////
/**
 * The start function called by IceBox to start this service.
 */
void ServiceManagerI::start(const ::std::string& name,
                           const CommunicatorPtr& communicator,
                           const StringSeq&)
{
  mServiceName = name;
  localAndClientMsg(VLogger::INFO, NULL, "starting service: %s\n", mServiceName.c_str());
  mAdapter = communicator->createObjectAdapter(mServiceName);
  clearStop();
  assert( mService );
  mAdapter->add(mService, communicator->stringToIdentity(mServiceName));
  mAdapter->activate();
  createSandbox();
  if (NULL!=mSS) mSS->starting();
  localAndClientMsg(VLogger::INFO, NULL, "service started: %s\n", mServiceName.c_str());
}

/**
 * The stop function called by IceBox to stop this service.
 */
void ServiceManagerI::stop()
{
  localAndClientMsg(VLogger::INFO, NULL, "Stopping Service: %s\n", 
                    mServiceName.c_str());
  if (NULL!=mSS) mSS->stopping();
  mAdapter->deactivate();
  waitForStopService();
  localAndClientMsg(VLogger::INFO, NULL, "Service stopped: %s\n",
                    mServiceName.c_str());
}

// look for ServiceNamex.TrainedModel
// Note that the x is significant
string ServiceManagerI::getModelFileFromConfig()
{
    CommunicatorPtr comm = mAdapter->getCommunicator();
    if ( comm )
    {
        PropertiesPtr props = comm->getProperties();
        if (props==true)
        {
            string propname = mServiceName + "x.TrainedModel";
            string propval = props->getProperty( propname );
            return propval;
        }
    }
    return "";
}


///////////////////////////////////////////////////////////////////////////////
std::string ServiceManagerI::getDataDir()
{
	PropertiesPtr props =
            mAdapter->getCommunicator()->getProperties();
	vLogger.setLocalVerbosityLevel(props->getProperty("CVAC.ServicesVerbosity"));

	// Load the CVAC property: 'CVAC.DataDir'.  Used for the xml filename path,
        // and to provide a prefix to Runset paths
	std::string dataDir = props->getProperty("CVAC.DataDir");
	if(dataDir.empty()) 
	{
            localAndClientMsg(VLogger::WARN, NULL,
                              "Unable to locate CVAC Data directory, specified: "
                              "'CVAC.DataDir = path/to/dataDir' in config.service\n");
	}
	localAndClientMsg(VLogger::DEBUG, NULL,
                          "CVAC Data directory configured as: %s \n", dataDir.c_str());
    return dataDir;
}
///////////////////////////////////////////////////////////////////////////////
void ServiceManager::createSandbox()
{
    mSandbox = new SandboxManager(getDataDir());
}
///////////////////////////////////////////////////////////////////////////////
void ServiceManager::stopService()
{
    if (mStopState == Running)
        mStopState = Stopping;
    else
        mStopState = Stopped;
}

///////////////////////////////////////////////////////////////////////////////
void ServiceManager::waitForStopService()
{
    stopService();
    while (isStopCompleted() == false)
    {
        sleep(100);
    }
}

///////////////////////////////////////////////////////////////////////////////
bool ServiceManager::stopRequested()
{
    if (mStopState == Stopping)
        return true;
    else 
        return false;
}

///////////////////////////////////////////////////////////////////////////////
void ServiceManager::clearStop()
{
    mStopState = None;
}

///////////////////////////////////////////////////////////////////////////////
void ServiceManager::stopCompleted()
{
    mStopState = Stopped;
}

///////////////////////////////////////////////////////////////////////////////
void ServiceManager::setStoppable()
{
    mStopState = Running;
}

///////////////////////////////////////////////////////////////////////////////
bool ServiceManager::isStopCompleted()
{
    if (mStopState != Stopping)
        return true;
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////
std::string ServiceManager::getServiceName()
{
    return mServiceName;
}

///////////////////////////////////////////////////////////////////////////////
// SandboxManager classes
///////////////////////////////////////////////////////////////////////////////

ClientSandbox::ClientSandbox(const std::string &clientName, 
                                  const std::string &CVAC_DataDir )
{
    _clientName = clientName;
    _cvacDataDir = CVAC_DataDir;
}
///////////////////////////////////////////////////////////////////////////////
std::string ClientSandbox::createTrainingDir()
{
    if (!_trainDir.empty())
    {// Get rid of old training directory 
        deleteDirectory(_trainDir);
    }
    if (_clientDir.empty())
    {
        getClientDir();
    }
    _trainDir = getTempFilename(_clientDir , "train_");   
    if (directoryExists(_trainDir))
    { // This should never happen
        deleteDirectory(_trainDir);
    } 
    makeDirectories(_trainDir);   
    return _trainDir;
    
}

///////////////////////////////////////////////////////////////////////////////
std::string ClientSandbox::getClientDir()
{
    if (_clientDir.empty())
    {
        _clientDir = _cvacDataDir + "/" + SANDBOX + "/" + _clientName;
        if (!directoryExists(_clientDir))
        {
            makeDirectories(_clientDir);
        }
    }
    return _clientDir;
}

///////////////////////////////////////////////////////////////////////////////
void ClientSandbox::deleteTrainingDir()
{
    if (!_trainDir.empty())
    {
        deleteDirectory(_trainDir);
        _trainDir.erase();
    }
}

///////////////////////////////////////////////////////////////////////////////
SandboxManager::SandboxManager(const std::string & CVAC_DataDir)
{
    mCVAC_DataDir = CVAC_DataDir;
}

///////////////////////////////////////////////////////////////////////////////
std::string SandboxManager::createClientName(const std::string &serviceName,
                                             const std::string &connectionName)
{
    std::string clientName = serviceName + "_" + connectionName;
    std::vector<ClientSandbox>::iterator it;
    for (it = mSandboxes.begin(); it < mSandboxes.end(); ++it)
    {
        ClientSandbox sbox = (*it);
        if (clientName.compare(sbox.getClientName()) == 0)
        {
            return clientName;
        }
    }
    // We don't have that client name so add a new sandbox
    ClientSandbox *sandbox = new ClientSandbox(clientName, mCVAC_DataDir);
    mSandboxes.push_back(*sandbox);
    return clientName;
}

///////////////////////////////////////////////////////////////////////////////
std::string SandboxManager::createTrainingDir(const std::string &clientName)
{
    std::vector<ClientSandbox>::iterator it;
    for (it = mSandboxes.begin(); it < mSandboxes.end(); ++it)
    {
        if (clientName.compare((*it).getClientName()) == 0)
        {
           return (*it).createTrainingDir();
        }
    }
    std::string empty;
    return empty;
}

///////////////////////////////////////////////////////////////////////////////
void SandboxManager::deleteTrainingDir(const std::string &clientName)
{
    std::vector<ClientSandbox>::iterator it;
    for (it = mSandboxes.begin(); it < mSandboxes.end(); ++it)
    {
        if (clientName.compare((*it).getClientName()) == 0)
        {
           (*it).deleteTrainingDir();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
std::string SandboxManager::getTrainingDir(const std::string &clientName)
{
    std::vector<ClientSandbox>::iterator it;
    for (it = mSandboxes.begin(); it < mSandboxes.end(); ++it)
    {
        if (clientName.compare((*it).getClientName()) == 0)
        {
           return (*it).getTrainingDir();
        }
    }
    std::string empty;
    return empty;
}

///////////////////////////////////////////////////////////////////////////////
std::string SandboxManager::createClientDir(const std::string &clientName)
{
    std::vector<ClientSandbox>::iterator it;
    for (it = mSandboxes.begin(); it < mSandboxes.end(); ++it)
    {
        if (clientName.compare((*it).getClientName()) == 0)
        {
           return (*it).getClientDir();
        }
    }
    std::string empty;
    return empty;
}

/** test if .services_started.lock file exists
 */
bool servicesStarted()
{
  printf("TODO: servicesStarted()\n");
  return true;
}

/** "exec" startIcebox.sh/bat and wait for completion (a few seconds)
 */
void doStartServices()
{
  printf("TODO: exec job()\n");
}

/** Parse the config.services file for any configured service.
 *  We do this by looking for the names that come before the ".Endpoints"
 *  text, in lines that don't start with #.
 */
void parseConfigServices( StringSet& configured )
{
  printf("parseConfigServices\n");
  // for line in config.services:
  //   if not line starts with #
  //     dotpos = strfind( ".Endpoints" )
  //     if dotpos != line.end()
  //       serviceName = line.substring( 1, dotpos-1 )
  //       serviceName.eraseLeadingTrailingWhitespace
  //       configured.push_back( serviceName )
}

// see documentation in .h file
StringSet startServices()
{
  // for now, we don't test individual services but only whether
  // bin/startIcebox has been run, based on a "touched" lock file
  if (!servicesStarted())
  {
    doStartServices();
  }

  // if still no lock file, report no services
  StringSet running;
  if (servicesStarted())
  {
    // otherwise, parse config.services for which services MIGHT
    // have been successfully started because they're configured
    // in config.services.  Note that this does not take the icebox
    // configuration or even startup success into account at all.
    parseConfigServices( running );
  }
  
  return running;
}
