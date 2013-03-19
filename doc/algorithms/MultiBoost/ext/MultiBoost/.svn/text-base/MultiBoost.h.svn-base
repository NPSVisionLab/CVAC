/**
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
* This class is the external interface of the MultiBoost detector.
* If the detector is to be used in a runtime implementation, this
* is the proper class to use.
*
* created: 9 August 2009
* author: Mathias
*/

#ifndef __MULTIBOOST_H__INCLUDED__
#define __MULTIBOOST_H__INCLUDED__

class MultiBoostMatch
{
public:
  MultiBoostMatch( int classid, float score, int x, int y, int w, int h )
    : m_classid(classid)
    , m_score(score)
    , m_x(x)
    , m_y(y)
    , m_w(w)
    , m_h(h)
  {}
public:
  int m_classid;
  float m_score;
  int m_x, m_y, m_w, m_h;
};

class MultiBoost
{
public:
  // instead of a factory, use this method to get an implementation
  // of MultiBoost.  Implemented in runDetector.
  static MultiBoost* CreateMultiBoost();

  // the number of classes is extracted from the dictionary file.
  virtual void Init(const std::string& dictionary_fname, 
    const std::string& detector_fname) = 0;
  virtual void GetMinImageSize( int *pWidth, int *pHeight ) = 0;
  virtual void Detect( IplImage* gray_img, IplImage* output_img,
    std::vector<MultiBoostMatch> &matches) = 0;
  virtual void Detect( IplImage* gray_img, IplImage* output_img,
    std::vector<MultiBoostMatch> &matches, 
    double start_scale, double stop_scale, double scale_inc_factor ) = 0;
  virtual void UnInit() = 0;
};

#endif // __MULTIBOOST_H__INCLUDED__
