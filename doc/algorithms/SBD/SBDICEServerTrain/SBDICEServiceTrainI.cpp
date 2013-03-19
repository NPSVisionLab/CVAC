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
#include "SBDICEServiceTrainI.h"
#include <util/ServiceMan.h>
using namespace cvac;

#ifdef WIN32
static HANDLE pthread = NULL;
#endif
extern "C"
{
	//
	// Factory function
	//
	ICE_DECLSPEC_EXPORT IceBox::Service* create(Ice::CommunicatorPtr communicator)
	{
		return new SBDICEServiceTrainI;
	}
}

///////////////////////////////////////////////////////////////////////////////
SBDICEServiceTrainI::SBDICEServiceTrainI()
: mDetectorTrainer(NULL)
{
    _adapter = NULL;
}


///////////////////////////////////////////////////////////////////////////////
SBDICEServiceTrainI::~SBDICEServiceTrainI()
{

}

///////////////////////////////////////////////////////////////////////////////
void SBDICEServiceTrainI::start(const ::std::string& name,const ::Ice::CommunicatorPtr& communicator,const ::Ice::StringSeq&)
{
  localAndClientMsg(VLogger::DEBUG_3, NULL, "SBD_Trainer: starting\n"); // Echo immediately
   // Since we don't have the ability to control when the trainer shuts down (SVM OpenCV train) then just
   // kill the thread.
   setServiceName(name);
#ifdef WIN32
   pthread = GetCurrentThread();
#endif
	_adapter = communicator->createObjectAdapter(name);
	mDetectorTrainer = new SBDICETrainI();
	_adapter->add(mDetectorTrainer, communicator->stringToIdentity("SBDTrain"));
	_adapter->activate();
  localAndClientMsg(VLogger::INFO, NULL, "Service started: SBDTrain \n");
}

///////////////////////////////////////////////////////////////////////////////
void SBDICEServiceTrainI::stop()
{
    _adapter->deactivate();
    localAndClientMsg(VLogger::INFO, NULL, "Stopping Service: SBDTrain \n");
    // Since we don't have the ability to control when the trainer shuts down (SVM OpenCV train) then just
    // kill the thread.
#ifdef WIN32
    //if (pthread != NULL)
     //   TerminateThread(pthread, 0);
#endif
    if (mDetectorTrainer != NULL)
	    delete mDetectorTrainer; 
    mDetectorTrainer = NULL;
    /*if (_adapter != NULL)
	    _adapter->destroy();*/
	localAndClientMsg(VLogger::INFO, NULL, "Service stopped: SBDTrain \n");
}
