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
#include <util/Timing.h>
#include <util/FileUtils.h>

///////////////////////////////////////////////////////////////////////////////
cvac::ServiceManager::ServiceManager()
{
    mService = NULL;
    mAdapter = NULL;
    mStopState = None;
}

///////////////////////////////////////////////////////////////////////////////
void cvac::ServiceManager::setService(cvac::CVAlgorithmService *serv,
                      std::string serviceName)
{
    mService = serv;
    mServiceName = serviceName;
}

///////////////////////////////////////////////////////////////////////////////
void cvac::ServiceManager::start(const ::std::string& name,const 
                  ::Ice::CommunicatorPtr& communicator,const 
                  ::Ice::StringSeq&)
{
    localAndClientMsg(VLogger::INFO, NULL, "%s: starting\n", mServiceName.c_str());
	mIceName = name;
	clearStop();
	mAdapter = communicator->createObjectAdapter(name);
	mAdapter->add(mService, communicator->stringToIdentity(mServiceName));
	mAdapter->activate();
	localAndClientMsg(VLogger::INFO, NULL, "Service started: %s\n", 
                mServiceName.c_str());
}

///////////////////////////////////////////////////////////////////////////////
void cvac::ServiceManager::stop()
{
    localAndClientMsg(VLogger::INFO, NULL, "Stopping Service: %s\n", 
                     mServiceName.c_str());
    mAdapter->deactivate();
    waitForStopService();
	localAndClientMsg(VLogger::INFO, NULL, "Service stopped: %s\n",
                     mServiceName.c_str());
}

///////////////////////////////////////////////////////////////////////////////
std::string cvac::ServiceManager::getDataDir()
{
	Ice::PropertiesPtr props = (mAdapter->getCommunicator()->getProperties());
	vLogger.setLocalVerbosityLevel(props->getProperty("CVAC.ServicesVerbosity"));

	// Load the CVAC property: 'CVAC.DataDir'.  Used for the xml filename path, and to provide a prefix to Runset paths
	std::string dataDir = props->getProperty("CVAC.DataDir");
	if(dataDir.empty()) 
	{
		localAndClientMsg(VLogger::WARN, NULL, "Unable to locate CVAC Data directory, specified: 'CVAC.DataDir = path/to/dataDir' in </CVAC_Services/config.service>\n");
	}
	localAndClientMsg(VLogger::DEBUG, NULL, "CVAC Data directory configured as: %s \n", dataDir.c_str());
    return dataDir;
}

///////////////////////////////////////////////////////////////////////////////
void cvac::ServiceManager::stopService()
{
    mStopState = Stopping;
}


///////////////////////////////////////////////////////////////////////////////
void cvac::ServiceManager::waitForStopService()
{
    stopService();
    while (isStopCompleted() == false)
    {
        cvac::sleep(100);
    }
}

///////////////////////////////////////////////////////////////////////////////
bool cvac::ServiceManager::stopRequested()
{
    if (mStopState == Stopping)
        return true;
    else 
        return false;
}

///////////////////////////////////////////////////////////////////////////////
void cvac::ServiceManager::clearStop()
{
    mStopState = None;
}

///////////////////////////////////////////////////////////////////////////////
void cvac::ServiceManager::stopCompleted()
{
    mStopState = Stopped;
}

///////////////////////////////////////////////////////////////////////////////
bool cvac::ServiceManager::isStopCompleted()
{
    if (mStopState != Stopping)
        return true;
    else
        return false;
}


///////////////////////////////////////////////////////////////////////////////
std::string cvac::ServiceManager::getServiceName()
{
    return mServiceName;
}

///////////////////////////////////////////////////////////////////////////////
std::string cvac::ServiceManager::getIceName()
{
    return mIceName;
}
