#pragma once
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

#include <Data.h>
#include <Services.h>
#include <ctime>  //for seeding the function "rand()"

#ifdef WIN32
#include <util/wdirent.h>
#else
#include <dirent.h>
#endif

#include <util/RunSetWrapper.h>
#include <util/MediaConverter.h>
#include <map>

namespace cvac
{
  using namespace std; 

  typedef pair<string,MediaConverter*> rsMediaPair;
//---------------------------------------------------------------------------
  struct RunSetConstraint
  {
    cvac::Purpose compatiblePurpose;
    string substrateType ;
    vector<rsMediaType> mimeTypes;  //new string[3] { "hi", "there"};
    bool spacesInFilenamesPermitted;
    void addType(rsMediaType _type)  //how to handle duplication
    {
      mimeTypes.push_back(_type);
    };
    void clear()
    {
      mimeTypes.clear();
    }
  };

//--------------------------------------------------------------------------- 

  class RunSetIterator
  {		
  public:
    RunSetIterator(RunSetWrapper* _rsw,RunSetConstraint& _cons,
                   ServiceManager *_sman,
                   const CallbackHandlerPrx& _callback,
                   int _nSkipFrames = 100);
    ~RunSetIterator();		

  private:
    CallbackHandlerPrx mCallback2Client;
    bool mFlagInitialize;
    ServiceManager* mServiceMan;
    string mMediaRootDirectory;
    string mMediaTempDirectory;
    ResultSet mResultSet;
    cvac::Purpose mConstraintPurpose;
    vector<rsMediaType>  mResultSetType;
    vector<int>          mListOrginalIdx;
    vector<LabelablePtr> mList;    
    vector<LabelablePtr>::iterator mListItr;    
    vector<int>::iterator mListOrginalIdxItr;
    vector<rsMediaType> mConstraintType;
    vector<rsMediaType>::iterator mConstraintTypeItr;    
    map< string,MediaConverter* > mConvertible; //a list of pairs to be convertible, the sequence may indicate priority
    map< string,MediaConverter* >::iterator mConvertibleItr; //a list of pairs to be convertible, the sequence may indicate priority

  private:  //list converter
    void makeConversionList();
    MediaConverter_openCV_i2i* mConv_openCV_i2i;
    MediaConverter_openCV_v2i* mConv_openCV_v2i;

  private:
    bool convert();
    void clear();
    void initIterator(){  mListItr = mList.begin(); mListOrginalIdxItr = mListOrginalIdx.begin(); };    
    bool makeList(ResultSet& _resultSet,vector<rsMediaType>& _resultSetType);    
    bool makeList(ResultSet& _resultSet);
    bool isInConstraintType(const rsMediaType& _type);
    void addToList(const LabelablePtr _pla,int _originalIdx);
    bool convertAndAddToList(const LabelablePtr& _pla,
                             const rsMediaType& _targerType,
                             MediaConverter* _pConv,
                             const string& _rDirTemp,int _originalIdx);
    rsMediaType getType(const LabelablePtr _pla);
    bool matchPurpose(int origIdx);
    LabelablePtr cloneLabelablePtr(const LabelablePtr _pla, int frameNum);

  public:    
    bool hasNext();
    LabelablePtr getNext();
    bool isInitialized(){ return mFlagInitialize; };
    Result& getCurrentResult();
    ResultSet& getResultSet();
    void showList();
  };
}

