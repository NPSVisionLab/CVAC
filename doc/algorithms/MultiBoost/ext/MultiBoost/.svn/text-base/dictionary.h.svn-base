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
#ifndef __MULTIBOOST__PRECOMPILED_HEADER_INCLUDED__
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <vector>
#endif //  __MULTIBOOST__PRECOMPILED_HEADER_INCLUDED__

#ifdef HAVE_PROTOBUF
  #include "trainingData.pb.h"
  using namespace trainingData;
#endif //HAVE_PROTOBUF

#include "parameters.h"


class fileManager;
class CrossConvolution;
class utilities;

class dictionary_storage
  {
  public:
      dictionary_storage();
      std::vector<int> Lx; //offset position relative to the center in X direction
      std::vector<int> Ly; //offset position relative to the center in Y direction
      std::vector<int> nfilter; //filter number
      std::vector<CvMat*> filters; //patch entry
      CvMat **patch; //patch entry
      CvMat **locX; //the 1D filter for the X axis for location.
      CvMat **locY; //the 1D filter for the Y axis for location.
      std::vector<int> imagendx; //number of image
      std::vector<int> elemndx; //number of object within the image
      std::vector<int> cx, cy; //center of newimg
      std::vector<int> xo, yo; //center of the patch
      std::vector<int> avg_width, avg_height; //average window size PER CLASS
  };

class dictionary
{
  //variables
public:
  dictionary_storage code_entry; //info of a single entry of the code book;
  CvSize dictobjsize;
protected:
  utilities *utilidades; //call class of utilies functions
  parameters *Params;
  CrossConvolution *CrossOp;//start cross convultion object
  std::vector<CvMat*> m_filters;
  int m_largest_patch_size;

public:
  //methods
  dictionary();
  void initialize( parameters *par );
  void BuildDictionary(fileManager *DB);
  void LoadDictionary(const std::string& dict_fname); 
#ifdef HAVE_PROTOBUF
  void LoadDictionary(Data &Dictentry);
#endif // HAVE_PROTOBUF
  CvMat *DeltaBlurredOffest(int Lx,int xo,float cx); //blurred the position offeset using a delta gaussian blurred function
  CvMat *convertToCvMat(const float *g, int size);
  ~dictionary(void);
  int getNumClasses() { return m_nclasses; }
  int getLargestPatchSize();

private:
  bool m_initialized;
  int m_nclasses;
  void *icvClearAlloc(int size); //cleans a bunch of memory
    void icvFreeMatrixArray(CvMat ***matrArray,int numMatr); //deallocates memory  
  CvMat* locSigma; //% spatial filtering of the correlation score.

};
