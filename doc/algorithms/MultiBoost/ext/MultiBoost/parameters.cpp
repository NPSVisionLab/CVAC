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
#include "StdAfx.h"
#include "parameters.h"

#ifndef PATH_MAX
#define PATH_MAX 512
#endif /* PATH_MAX */

#pragma warning( push )
#pragma warning( disable : 4996 )

void *parameters::icvClearAlloc(int size)
{
    void *ptr = 0;
    if( size > 0 )
    {
        ptr = cvAlloc(size);
        memset(ptr,0,size);
    }
    return ptr;
}

void parameters::icvFreeMatrixArray(CvMat ***matrArray,int numMatr)
{
    /* Free each matrix */
    int currMatr;
    
    if( *matrArray != 0 )
    {/* Need delete */
        for( currMatr = 0; currMatr < numMatr; currMatr++ )
        {
            cvReleaseMat((*matrArray)+currMatr);
        }
        cvFree( matrArray);
    }
    return;
}

parameters::parameters( int _nClasses )
: nclasses( _nClasses ) // defaults to -1
{
  int nnodes=0;
  flnMasks=new char[PATH_MAX];

  // main settings, dependent on what object we'd like to detect
  // ------------------
  // MULTIBOOST_BASEDIR = "C:\\home\\matz\\PROJECTS\\MultiBoost";
  // MULTIBOOST_BASEDIR = "C:\\POSTDOC\\Base_It\\MultiBoost";
  //MULTIBOOST_BASEDIR = "C:\\Documents and Settings\\nmlloyde\\My Documents\\MultiBoost";
	MULTIBOOST_BASEDIR = "../MultiBoostData";
  //sprintf( flnMasks, "%s\\Data\\rifle_masks", MULTIBOOST_BASEDIR.c_str() );
  //sprintf( flnMasks, "%s\\Data\\marine8_masks", MULTIBOOST_BASEDIR.c_str() );
	sprintf( flnMasks, "%s/Data/marine_masks", MULTIBOOST_BASEDIR.c_str() );
  //sprintf( flnMasks, "%s/Data/skull_masks", MULTIBOOST_BASEDIR.c_str() );
  //dictionary_fname = "rifle_dictionary_25x25.xml";  // .xml file
  //features_fname = "rifle_features_25x25.bin";      // .bin file
  //detector_fname = "rifle_detector_25x25.xml";      // .xml file format
  dictionary_fname = "marine8_dictionary_25x50.xml";  // .xml file
  features_fname = "marine8_features_25x50.bin";      // .bin file
  detector_fname = "marine8_detector_25x50.xml";      // .xml file format
  //dictionary_fname = "head_dictionary_11x11.xml";  // .xml file
  //features_fname = "head_features_11x11.bin";      // .bin file
  //detector_fname = "head_detector_11x11.xml";      // .xml file format

  //push_paths("rifle+az225deg.txt");
  //push_paths("rifle+az270deg.txt");
  //push_paths("rifle+az315deg.txt");

  push_paths("marine+standing+az0deg,marine+walking+az0deg.txt");
  push_paths("marine+standing+az90deg,marine+walking+az90deg.txt");
  push_paths("marine+standing+az180deg,marine+walking+az180deg.txt");
  push_paths("marine+standing+az270deg,marine+walking+az270deg.txt");
  push_paths("marine+kneeling+az0deg.txt");
  push_paths("marine+kneeling+az90deg.txt");
  push_paths("marine+kneeling+az180deg.txt");
  push_paths("marine+kneeling+az270deg.txt");

  //push_paths("skull+az0deg.txt");
  //push_paths("skull+az90deg.txt");
  //push_paths("skull+az180deg.txt");
  //push_paths("skull+az270deg.txt");

  // size of object in a normalized frame (= [max height, max width])
  normalizedObjectSize = cvSize(25, 50); // for example, 25x25 for heads, 48x128 for full body
  min_IP_distance = 6.0; // 5.0 for 25x25 heads, 10.0 for 48x128 full body
  printf("parameters for training: %d classes (+1), obj size %dx%d, min_IP_dist %f\n",
    objects_name.size(),
    normalizedObjectSize.width, normalizedObjectSize.height, min_IP_distance );

  if (nclasses==-1)
  {
    // nclasses is set automatically when we do training. for detection and
    // simulated data, the number can be set explicitly
    nclasses=objects_name.size()+1;//  number of classes to recognize including background   
  }

  //int ii=0;
  nnodes=(int)pow((double)nclasses-1,2)-1;
  // PARAMETERS BOOSTING
  TT=cvCreateMat( nclasses-1, nnodes, CV_32F); //same but with CvMat
  cvZero(TT);
  convert_bin(nnodes,nclasses-1); //creates topology of classifier tree 0001, 0011, 01001 , etc
  // PARAMETERS CLASSIFIER
  Nthresholds = 105;
  Nfilters=4;

  sampleFromImages = 8; // Number of images used to build the dictionary of patches. The images selected will not be used for training or test.
  patchesFromExample = 20; // Number of patches to be extracted from every image.

  // parameters for precomputed features
  negativeSamplesPerImage = 15;    // number of background samples extracted from each image

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // Training parameters
  numTrainImages  = 60;        // number of object training instances, per category
  numTrainRounds = 200;  // Rounds of the boosting
  numTestImages = 50;
  printf( "using %d images to extract patches, %d images to train\n",
    sampleFromImages, numTrainImages );

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // Compute some common variables:
  Npatches = patchesFromExample * Nfilters;
  //strcat( message, "\nStack\n{" );     
}

parameters::~parameters(void)
{
  for (std::vector<int*>::iterator it = T.begin(); it!=T.end(); it++)
  {
    delete *it;
  }
  delete[] flnMasks;
}

/* size of the images used for training
*/
CvSize parameters::trainingImageSize( const CvSize& objsize ) const
{
//  int len = max(objsize.width, objsize.height) * 1.8;
//  CvSize tis = cvSize( len, len );
  CvSize tis = cvSize( 200, 200 );
  return tis;
}


void parameters::push_paths(char qry[PATH_MAX])
{
	query=new char[PATH_MAX];
	dictFln=new char[PATH_MAX];
	strcpy(dictFln,"");
	strcpy(query,"");
	strcpy(query,qry);
  strcat(dictFln,MULTIBOOST_BASEDIR.c_str());
	strcat(dictFln,"/Data/");
	strcat(dictFln,query);
	objects_name.push_back(dictFln);
  delete[] query;
  delete[] dictFln;
}

void parameters::convert_bin(int nnodes,int Nclasses)
{
  double val;
  int tot;
  
  for (int j=1;j<nnodes+1;j++)
  {
    binary_vector=new int[Nclasses];
    for (int i=Nclasses;i>0;i--)
    {
      val=pow((double)2,1-i);
      tot=(int)floor((double)j*pow((double)2,1-i));
      binary_vector[Nclasses-i]=tot%2;
	    TT->data.fl[j-1+(i-1)*TT->cols]=tot%2;
    }
    T.push_back(binary_vector); // the result is stored in T
  }
}

std::string parameters::GetDictionaryPath() const
{
  std::string fname = MULTIBOOST_BASEDIR;
  fname.append( "/Data/" );
  fname.append( dictionary_fname );
  return fname;
}

std::string parameters::GetFeaturesPath() const
{
  std::string fname = MULTIBOOST_BASEDIR;
  fname.append( "/Data/" );
  fname.append( features_fname );
  return fname;
}

std::string parameters::GetDetectorPath() const
{
  std::string fname = MULTIBOOST_BASEDIR;
  fname.append( "/Data/" );
  fname.append( detector_fname );
  return fname;
}

#pragma warning(pop)
