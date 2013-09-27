#ifndef __SERVICEMAN_H__
/***************************************************************************
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
 *     *  Redistributions of source code must retain the above notice,
 *       this list of conditions and the following disclaimer.
 *     *  Redistributions in binary form must reproduce the above notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     *  Neither the name of the Naval Postgraduate School, nor the name of
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
 * **************************************************************************/
#define __SERVICEMAN_H__

#include <string>
#include <vector>
#include <set>
//#include <Ice/Ice.h>
//#include <IceBox/IceBox.h>
//#include <Services.h>


#define TRAIN_PREFIX "train_"
#define SANDBOX "sboxes"

/**
 * Functions to manage a CVAC service.  Programs that take a long time to
 * complete need to periodically call stopRequested to see if the user
 * has canceled the operation.  If they have then the processing should stop
 * and stopCompleted should be called.  The service names should be the
 * service names defined in config.services. 
 */
namespace cvac
{
    class ServiceManagerIceService; 
    class CVAlgorithmService;
    /**
     * ClientSandbox - keep track of the client resources handed out.
     */
    class ClientSandbox
    {
    public:
        ClientSandbox(const std::string &clientName,
                      const std::string &CVAC_DataDir);
        /**
         * Get the current training directory
         */
        std::string getTrainingDir(){ return _trainDir; }
        /**
         * Delete the old training directory if it exists and creat a new one
         */
        std::string createTrainingDir();
        /**
         * Delete the current training directory
         */
        void deleteTrainingDir();
        /*
         * If the client directory does not exist create it and return it
         */
        std::string getClientDir();
        /**
         * Get the client name
         */
        std::string getClientName() { return _clientName; }
    private:
        std::string _clientName;
        std::string _cvacDataDir;
        std::string _trainDir;
        std::string _clientDir;
    };

    /**
     * SandboxManager - Manage allocation of directory and file resources for Services.
     */
    class SandboxManager
    {
    public:
        /**
         * Look and see what directories are there from the last time we where run
         * and add them to the sandbox list. Clean up any training directories that
         * failed to get cleaned up because of an exception or early stop.
         */
        SandboxManager(const std::string &CVAC_DataDir);

        /**
         * Request a client name.  This name is based on the service
         * name and the connection client name gotten from the service manager.
         */
        std::string createClientName(const std::string &serviceName,
                                     const std::string &connectionName); 
        /**
         * Create a training directory in the client directory.  If the client directory
         * does not exist then create it.
         */
        std::string createTrainingDir(const std::string &clientName);
        /**
         * Delete the training directory.
         */
        void deleteTrainingDir(const std::string &clientName);
        /**
         * Get the clients training directory
         */
        std::string getTrainingDir(const std::string &clientName);

        /**
         * Create a client directory if this is the first time we have seen this client
         * else return the existing client directory.
         */
        std::string createClientDir(const std::string &clientName);
        
    private:
        std::string mCVAC_DataDir;
        std::vector<ClientSandbox> mSandboxes;
        
   
    };

    /**
     * Class to manage the Ice Service functions
     */
    //class ServiceManager : public ::IceBox::Service
    class ServiceManager
    {
    public:
        typedef enum StopStateType {None, Running, Stopping, Stopped} StopState;
        /**
         * Constructor for creating a cvac Detector service.
         * Parms: The Detector instance to be served by this service.
         */
        ServiceManager();

        /**
         * Set The Service that is to be served by this service.
         * NOTE: A ServiceManager can manage either a detector or
         * detectorTrainer but not both!
         * Parms: The Algorithm instance to be served by this service and its name.
         */
        void setService(cvac::CVAlgorithmService *service, std::string serviceName);

       

        /** 
         * Returns true if a stop has been requested for this service.
         * The user needs to call this when running lengthly operations and
         * stop the operation if it returns true.  If the user supports these functions 
         * then he needs to call the setRunning call before the lengthly operation so the
         * ServiceManager will wait for the stop to be completed.  After the lengthly
         * operation the user should call clearStop.
         */
        bool stopRequested();
    
        /** 
         * Tell manager that the stop has been completed.  The user
         * should call this after an operation was stopped
         */
        void stopCompleted();
    
        /** 
         *  Poll to see if a stop is completed
         */
        bool isStopCompleted();
    
        /**
         * Tell manager to clear stop complete
         */
        void clearStop();
    
        /** 
         *  Requests that a given service be stopped.  
         */
        void stopService();

        /** 
         *  Tell ServiceManager that we are running and will listen for a stop request.  
         */
        void setStoppable();
    
        /** 
         *  Requests that the service be stopped.  And waits
         *  for the service to acknowlege the stop.
         */
        void waitForStopService();
    
        /**
         * Get the service name of this service
         */
        std::string getServiceName();

        /**
         * Get the icebox name of this service
         */
        std::string getIceName();

        /**
         * Get the icebox name of this service
         */
        void setIceName(std::string name);

        /*
         * Get the config.service defined data directory
         */
        std::string getDataDir();

        /*
         * Return the ice service
         */
        void* getIceService() { return mIceService; }

        SandboxManager  *getSandbox() { return mSandbox; }
  
        void createSandbox();

    private:
        std::string                     mServiceName;
        std::string                     mIceName;
        int                             mStopState;
        ServiceManagerIceService*       mIceService;
        SandboxManager*                 mSandbox;
    };

    // this could be an unordered_set instead
    typedef std::set<std::string> StringSet;

    /** Start the services as per config file; this won't re-start
     * the services if they are already running.  Still, avoid calling
     * this multiple times because it will access the file system
     * several times.
     * It returns the names of running services.
     */
    StringSet startServices();
}


#endif // __SERVICEMAN_H__
