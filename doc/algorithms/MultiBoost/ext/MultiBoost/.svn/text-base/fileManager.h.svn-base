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
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <vector>

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef PATH_MAX
#define PATH_MAX 512
#endif /* PATH_MAX */

#define LM_CENTERED 0
#define LM_ORIGINAL 1

typedef struct db_images
  {
      std::vector<CvRect> rects; //bounding boxes of objects
      int anno; //number of boxes
      char filename[PATH_MAX]; //iimage filename
      char maskFilename[PATH_MAX]; //iimage filename

  }
  db_images;

class fileManager
{
public:
 
	fileManager();
	~fileManager(void);
  //vars
  IplImage* GetImage(int nImage); //return a pointer to the image "index" from the dataset
  IplImage* GetMaskImage( int nImage); //return a pointer to the image "index" from the dataset
  IplImage* GetObjectinImage( int nImage, int nElem); //return a pointer to the image of a object number "anno" in the place "index" from the dataset.
  //The above function does not check if the subimage is totally inside the whole image. This should be done
  IplImage* GetMaskObjectinImage( int nImage,int nElem); //return a pointer to the image of a object mask number "anno" in the place "index" from the dataset
  
  //methods
  void CreateTrainingSamplesFromInfo( const char* infoname, const char* flnMasks); //populate the list of image information in the database
  int GetNumberObjectsinImage(int nImage); //return the number of annotated objects in the image
  int GetNumberofImages(void); //returns number of images in DB
  int GetNumberofObjectsinDB(void); //returns the number of objects in DB (in image we may have more than one marine, for example)
  int GetNumberofImagesinFile(const char* infoname);
  int GetNumberofObjectsinFile(const char* infoname);
  CvRect GetROIinImage( int nImage,int nElem); //return a pointer to the ROI of the object in image
  IplImage* Resize(IplImage* src, CvSize maximagesize,CvSize normalizedObjectSize, int flag,
CvRect annotation,CvRect &newannotation); //resize image to the dictionary image size, but constraining to minimum size
  //however the above function does not check if the image extracted is out of the large image
  IplImage* CropIm(IplImage* src,int *crop,CvRect annotation,CvRect &newannotation); //crops image checking that is still in the boundary


protected:
  typedef std::vector<db_images> VecImg; //vector of filenames and bounding boxes
  //db_images imInfo; //info of a single image
  VecImg vector_dataset;  //keep all the info regarding each image filename and bounding boxes on that image.
private: 
  //variables
  
  //methods
  
};
