#ifndef _BowICETrainI_H__
/*****************************************************************************
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
 **************************************************************************/
#define _BowICETrainI_H__

#include <map>
#include <string>

#include <Data.h>
#include <Services.h>
#include <bowCV.h>

#include <Ice/Ice.h>
#include <IceBox/IceBox.h>
#include <IceUtil/UUID.h>
#include <util/processRunSet.h>
#include <util/ServiceManI.h>
#include <util/MsgLogger.h>

typedef std::map<cvac::Purpose, std::string> LabelMap;
namespace{  // Need an anonymous namespace to resolve issues with classes of the same name
class TrainerPropertiesI : public cvac::TrainerProperties
{
public:
  /**
   * Initialize fields for this trainer.
   */
  TrainerPropertiesI();
  /**
   * Read the string properties and convert them to member data values.
   */
  bool readProps();
  /**
   * Convert member data values into string properties.
   */
  bool writeProps();
  /**
   * Load the struct's values into our class ignoring uninitialized values
   */
  void load(const TrainerProperties &p);
public:
  /**
   * Detector: SURF, SIFT, FAST, STAR, MSER, GFTT, HARRIS, ORB
   */
  string	keyptName_Detector;
  /**
   * Descriptor: SURF, SIFT, OpponentSIFT, OpponentSURF, ORB, FREAK
   */
  string	keyptName_Descriptor;
  /**
   * Matcher: BruteForce-L1, BruteForce, FlannBased, BruteForce-Hamming 
   */
  string	keyptName_Matcher;
  /**
   * The number of visual words (or clusters)
   */
  int       countWords;
  /**
   * How to handle negative samples: Ignore, Multiclass, FirstStage
   */
  string    rejectClassStrategy;
  /**
   * Whether different class weights are applied for or not   
   * according to the number of samples in each class
   */
  bool      flagClassWeight;
};

class BowICETrainI : public cvac::DetectorTrainer, public cvac::StartStop, public MsgLogger
{
public:
  BowICETrainI();
  ~BowICETrainI();
	
  virtual void process(const Ice::Identity &client, const ::cvac::RunSet&, 
                       const ::cvac::TrainerProperties &props,
                       const ::Ice::Current& = ::Ice::Current() );
  virtual void destroy(const ::Ice::Current& = ::Ice::Current() );
  virtual bool cancel( const Ice::Identity &client,
                       const ::Ice::Current& = ::Ice::Current() );
  virtual ::std::string getName(const ::Ice::Current& = ::Ice::Current() );
  virtual ::std::string getDescription(const ::Ice::Current& = ::Ice::Current() );
  virtual void starting();
  virtual void stopping();
  void setServiceManager(cvac::ServiceManagerI *sman);
  virtual ::cvac::TrainerProperties
    getTrainerProperties(const ::Ice::Current& = ::Ice::Current());
  
 private:  
  int                    m_cvacVerbosity;
  cvac::ServiceManager  *mServiceMan;
  int                    maxClassId;
  TrainerPropertiesI    *mTrainProps;

 private:
  bowCV* initialize( cvac::TrainerCallbackHandlerPrx& _callback,
                     const ::cvac::TrainerProperties &props,
                     ::cvac::DetectorDataArchive& dda,
                     const ::Ice::Current& = ::Ice::Current() );
  void processSingleImg(std::string _filepath, std::string _filename,int _classID,
                        const ::cvac::LocationPtr& _ploc, 
                        bowCV* pBowCV,
                        cvac::TrainerCallbackHandlerPrx& _callback);
  void processPurposedList( ::cvac::PurposedListPtr purList,
                            bowCV* pBowCV,
                            ::cvac::TrainerCallbackHandlerPrx& _callback,
                            const std::string& CVAC_DataDir,
                            LabelMap& labelmap, bool* labelsMatch);
  ::cvac::FilePath createArchive( ::cvac::DetectorDataArchive& dda,
                                  bowCV* pBowCV,
                                  const LabelMap& labelmap,
                                  const std::string& clientName,
                                  const std::string& CVAC_DataDir,
                                  const std::string& tempDir );
  int getPurposeId( const cvac::Purpose& pur,
                    cvac::TrainerCallbackHandlerPrx& _callback );
  bool checkPurposedLists( const cvac::PurposedListSequence& purposedLists,
                           cvac::TrainerCallbackHandlerPrx& _callback );
private:
  cvac::TrainerCallbackHandlerPrx callbackPtr;
  virtual void message(MsgLogger::Levels msgLevel, const string& _msgStr);
};
}
#endif //_BowICETrainI_H__
