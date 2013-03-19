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

#include "dictionary.h"

class parameters;
class fileManager;
class utilities;
class CrossConvolution;

typedef struct feature_storage
  {
  	  std::vector<CvMat*> features; //feature vector
      std::vector<int> clase; //class number
      std::vector<int> cx, cy; //center voted for the feature vector
      std::vector<int> imagendx; //number of image
      std::vector<int> elemndx; //object within the image number
  }
  feature_storage;

class ComputeFeatures
{
public:
	ComputeFeatures( const parameters *par );
	//typedef vector<dictionary_storage> Dictionar; //vector of all the entries inside the dictionary
	void CalculateFeatures(dictionary& codebook,fileManager *DB, const std::string& features_fname); //todo: check argument passing
  void SaveEntries(struct feature_storage &feature_entry, int negativeSamplesPerImage, int total, CvSeq* points_seq2, int elem, int ii, int nd, std::vector<CvMat*>& out  ); //Save Entries to the Database
  void WriteFeatures(const feature_storage& feature_entry, 
                                    const std::string& features_fname, int nd, int Nentries);
  void CornerReduce(IplImage* corner_mask, CvMat* score);
  void ProximityOrder(IplImage* corner_mask, IplImage* masked,IplImage *newmsk, CvSeq* points_seq, double min_distance, CvPoint& pt, CvMat* score );
	utilities *utilidades; //call class of utilies functions
	const parameters *Params;
	CrossConvolution *CrossOp;//start cross convultion object

	//Dictionar codebook;  //creates the object dict
	//dictionary_storage code_entry; //info of a single entry of the codebook
  int Nentries; //total entries in the dictionary

private:
  void *icvClearAlloc(int size); //allocates a big cuhnck of memory
  void icvFreeMatrixArray(CvMat ***matrArray,int numMatr); //deallocates memory
  void CalcAverage(CvMat *Qk_1, std::vector<CvMat*>& out); //compute averages on Z direction
  void LocalMaximums(CvMat *score, IplImage* corner_mask, CvSeq* points_seq, double min_distance); //find multiple local maxima
  void FindNotZero(IplImage*  image, CvSeq* points_seq); // retreives all the pixels coordinates for which the intensity is higher than '0'
  // like [x,y]=find(image>0) in Matlab
  void FindNotZeroRnd(IplImage* mask,CvSeq* points_seq, int num,
    int largest_patch_size ); //finds random points (in case that we don't have enough maximums) within the image, 
  // and outside the sillhoute
public:
	~ComputeFeatures(void);
};
