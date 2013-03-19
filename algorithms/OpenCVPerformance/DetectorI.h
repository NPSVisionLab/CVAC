
/****
 *CVAC Software Disclaimer
 *
 *This software was developed at the Naval Postgraduate School, Monterey, CA,
 *by employees of the Federal Government in the course of their official duties.
 *Pursuant to title 17 Section 105 of the United States Code this software
 *is not subject to copyright protection and is in the public domain. It is 
 *an experimental system.  The Naval Postgraduate School assumes no
 *responsibility whatsoever for its use by other parties, and makes
 *no guarantees, expressed or implied, about its quality, reliability, 
 *or any other characteristic.
 *We would appreciate acknowledgement and a brief notification if the software
 *is used.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above notice,
 *      this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the Naval Postgraduate School, nor the name of
 *      the U.S. Government, nor the names of its contributors may be used
 *      to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****/
#ifndef CVAC_DETECTOR_I_H
#define CVAC_DETECTOR_I_H

#include <Data.h>
#include <Services.h>
#include <opencv2/opencv.hpp>
#include <util/FileUtils.h>


namespace cvac {


/*
 * Detector Instance class for cvac Ice detector.
 * This detector is a opencv haar detector.
 */
class DetectorI :  virtual public cvac::Detector
{
protected:
    std::string _cascadeString; // The xml cascade file name
    std::string m_CVAC_DataDir; // Store an absolute path to the detector data files
    CvHaarClassifierCascade* _cascade; // The in memory cascade file
    CvMemStorage*            _storage; // The OpenCV storage
    int                      _nos;     // The number of stages initially
    int                      _verbosity; // Stored client-verbosity
    bool                     _is_initialized;
    std::string _name;   // name of the detector;
    std::string _description; //description of the detector
    cvac::DetectorData       _ddata;

public:
    static const int SCAN_WIDTH = 15;
    static const int SCAN_HEIGHT = 15;
    DetectorI(std::string name, std::string desc, std::string cascade);
    ~DetectorI();
    virtual void initialize(int verbosity, const ::cvac::DetectorData& data, const Ice::Current&);
    virtual cvac::DetectorData createCopyOfDetectorData(const ::Ice::Current& current);
    virtual void destroy(const Ice::Current &);
    virtual void process(const Ice::Identity &client,
                         const cvac::RunSet &run, const Ice::Current &);
    virtual DetectorPropertiesPrx getDetectorProperties(const Ice::Current &);
    CvMemStorage *getStorage() { return _storage; }
    CvHaarClassifierCascade *getCascade() { return _cascade; }
    int getNOS() { return _nos; }

    virtual bool isInitialized(const Ice::Current&);
    virtual std::string getName(const Ice::Current&);
    virtual std::string getDescription(const Ice::Current&);
    virtual void setVerbosity(int verbosity, const Ice::Current&);
};

typedef  void (DetectorI::*DetectFileFunc) (const char*, cvac::ResultSetV2 &);
}
#endif
