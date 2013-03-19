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
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()
#include <vector>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>

#ifndef PATH_MAX
#define PATH_MAX 512
#endif /* PATH_MAX */

class parameters
{
public:
  // main settings
  std::string MULTIBOOST_BASEDIR;
  std::string dictionary_fname;
  std::string features_fname;
  std::string detector_fname;
  CvSize normalizedObjectSize; // size of object in a normalized frame (= [max height, max width])
  double min_IP_distance;

   //Variables
   typedef std::vector <int*> binmat; //matrix of binary numbers
   binmat T; //creation of the matrix of binarys T (topology of tree multi-class)
   CvMat *TT; //same tree of topology, but using CVMat type (later will find very usefull)
   int Nthresholds; //thresholds for the boosting algorithm
   int sampleFromImages; // Number of images used to build the dictionary of patches. The images selected will not be used for training or test.
   int patchesFromExample; // Number of patches to be extracted from every image.

   CvSize trainingImageSize( const CvSize& objsize ) const;// size of the images used for training
   int negativeSamplesPerImage;    // number of background samples extracted from each image

   int numTrainImages;        // number of object training instances, per category
   int numTrainRounds;  // Rounds of the boosting
   int numTestImages;
   
   int Nfilters; //number of filters
   int Npatches; //number of patches
   char *dictFln; //Filenames including the images filenames with the object to be recognized
   char *flnMasks;  //Filename of the directory containing all the image masks
   char *query;  
   typedef std::vector<std::string> queries; //vector of filenames and bounding boxes
	 queries objects_name;
   int aperture_size; //aperture size of the kernel for the correlation score
   int nclasses; //  number of classes to recognize including background   
   int *binary_vector; //the '1' and '0' in each vector

   //Methods
   parameters( int nClasses=-1 ); // nClasses is determined automatically for training
   void push_paths(char qry[PATH_MAX]);
   void *icvClearAlloc(int size);
   void icvFreeMatrixArray(CvMat ***matrArray,int numMatr);
  ~parameters(void);

  std::string GetDictionaryPath() const;
  std::string GetFeaturesPath() const;
  std::string GetDetectorPath() const;
 
protected:
	
	
private:
   void convert_bin(int nnodes,int Nclasses); //convert decimals to binary numbers
   float a[9],b[9],c[9],d[1]; //size of filters

};
