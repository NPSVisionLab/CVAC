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
#include "trainDetector.h"
#include "utilities.h"
#include "parameters.h"
#include "fileManager.h"
#include "CrossConvolution.h"
#include "dictionary.h"
#include "ComputeFeatures.h"

#pragma warning( push )
#pragma warning( disable : 4996 )

trainDetector::trainDetector( const parameters* par )
{ 
  Params = par;
  Nentries=Params->Nfilters*Params->patchesFromExample*Params->sampleFromImages*(Params->nclasses-1); //number of dimensions
  Nvecs = Params->numTrainImages*(Params->nclasses-1)*(Params->negativeSamplesPerImage+1); //number of samples
  utilidades = new utilities( Params );
}

trainDetector::~trainDetector()
{ 
  delete utilidades;
}

void *trainDetector::icvClearAlloc(int size)
{
  void *ptr = 0;
  if( size > 0 )
  {
    ptr = cvAlloc(size);
    memset(ptr,0,size);
  }
  return ptr;
}

void trainDetector::icvFreeMatrixArray(CvMat ***matrArray,int numMatr)
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


void trainDetector::loadData( const std::string& features_fname )
{
  time_t start_time,end_time; //for time performance
  start_time= 0; end_time= 0; time( &start_time );
  FILE *file; 
  file = fopen(features_fname.c_str(), "rb");
  if(file==NULL)
  {
    printf("An error has occurred.\n");
    exit(0);
  }

  int clase;
  int nd=0; //number of elements readed (entries)
  while (fread(&clase, sizeof(int), 1,file )==1)//this is to make sure that we don't read more data that in the file
  {
    feature_entry.clase[nd]=clase;
    fread(feature_entry.features[nd]->data.fl, sizeof(float), Nentries,file );
    nd++;
  }
  fclose(file); //now close the file 
  utilidades->print_time(start_time,end_time,"Running time for reading the data from the file: %02d:%02d:%02d\n");      
  Nvecs=nd; //we hope that we have enough entries to match the real value of nVecs defined in the constructor
  
}


void trainDetector::train(const std::string& features_fname, const std::string& classifier_fname)
{ 
  //allocate memory to load the feature vectors
  feature_entry.clase.resize( Nvecs );
  feature_entry.features.resize( Nvecs );

  for (int i=0;i<Nvecs;i++)
    feature_entry.features[i]= cvCreateMat( 1, Nentries, CV_32F);
  
  float *temp=new float[Nentries]; //auxiliar for random

  CvRNG rng = cvRNG(0xffffffff); //random generator
  //CvMat *shuffle_index=cvCreateMat( 1, Nvecs, CV_32F);

  loadData( features_fname ); // Load precomputed features for training file.

  /* shuffle samples */
  /* LEAVE THIS OUT ONLY FOR TESTING!!! REMEMBER TO PUT THIS BACK AS IT WAS
  for(int i = 0; i < Nvecs/2; i++ )
  {
      int index1=cvRandInt(&rng)%Nvecs;
      int index2=cvRandInt(&rng)%Nvecs;
      CV_SWAP( feature_entry.features[index1]->data.fl, feature_entry.features[index2]->data.fl, temp );
      int tmp_clase; //auxiliar for random
      CV_SWAP( feature_entry.clase[index1], feature_entry.clase[index2], tmp_clase );
  }
  */


  //copy all the data of the features and classes into 2D matrices
  CvMat* classes = cvCreateMat( Nvecs, 1, CV_32SC1 );
  CvMat* features = cvCreateMat(Nentries, Nvecs,CV_32FC1 ); //59 features and 1000 samples
  
  CvMat cole;
  for(int i = 0; i < Nvecs; i++ )
  {
     cvGetCol(features,&cole,i);
     cvTranspose(feature_entry.features[i],&cole);
     classes->data.i[i]=feature_entry.clase[i]+1;
  }

  /*Call here the Boost training!!!*/
  clasificador classifier;
  classifier.objsize = Params->normalizedObjectSize;

  //train the detector
  JBoost.jointBoosting(features,classes,Params->T,Params->numTrainRounds,Params->Nthresholds,Params->nclasses-1, classifier);

  classifier.save(classifier_fname);

  //release the memory of the classifier (you should save it before that)
  delete [] classifier.a;
  delete [] classifier.b;
  delete [] classifier.th;
  delete [] classifier.bestnode;
  delete [] classifier.featureNdx; 


  cvReleaseMat( &classes);
  cvReleaseMat( &features);
  for (int i=0;i<Nvecs;i++)
    cvReleaseMat( &feature_entry.features[i] );
}

void trainDetector::test(const std::string& features_fname, const std::string& classifier_fname)
{ 
  //allocate memory to load the feature vectors
  feature_entry.clase.resize( Nvecs );
  feature_entry.features.resize( Nvecs );

  for (int i=0;i<Nvecs;i++)
    feature_entry.features[i]= cvCreateMat( 1, Nentries, CV_32F);

  loadData( features_fname ); // Load precomputed features for training file.

  //copy all the data of the features and classes into 2D matrices
  CvMat* classes = cvCreateMat( Nvecs, 1, CV_32SC1 );
  CvMat* features = cvCreateMat(Nentries, Nvecs,CV_32FC1 ); //2560 features and 6400 samples
  CvMat* featuretest = cvCreateMat(Nentries, 1,CV_32FC1 ); //2560 features and 6400 samples
  CvMat temp;

  CvMat cole;
  for(int i = 0; i < Nvecs; i++ )
  {
     cvGetCol(features,&cole,i);
     cvTranspose(feature_entry.features[i],&cole);
     classes->data.i[i]=feature_entry.clase[i]+1;
  }
  
  cvGetCol(features,&temp,15); //test the first positive sample
  cvCopy(&temp,featuretest);
  /*Call here the Boost training!!!*/
  clasificador classifier;

  classifier.load(classifier_fname);

  //train the detector
 //JBoost.jointBoosting(features,classes,Params->T,Params->NweakClassifiers,Params->Nthresholds,Params->nclasses-1, classifier);

 int Nnodes=Params->T.size(); 
 int cluster_count=Params->nclasses-1;
 int sample_count=1; // test only one feature vector 

 CvMat* Fstumps = cvCreateMat( sample_count, classifier.nWeakClassifiers, CV_32FC1); //% F(n) is the function in node 'n'. 
 CvMat* Fn = cvCreateMat( sample_count, Nnodes, CV_32FC1); //% F(n) is the function in node 'n'.
 CvMat* Fx = cvCreateMat(sample_count,cluster_count, CV_32FC1 );
 CvMat* Cx = cvCreateMat(sample_count,cluster_count, CV_8UC1 );

 JBoost.strongJointClassifier(featuretest, classifier, Params->TT,cluster_count, Cx, Fx, Fn, Fstumps);

  //release the memory of the classifier (you should save it before that)
  delete [] classifier.a;
  delete [] classifier.b;
  delete [] classifier.th;
  delete [] classifier.bestnode;
  delete [] classifier.featureNdx; 

  cvReleaseMat( &featuretest);
  cvReleaseMat( &classes);
  cvReleaseMat( &features);
  cvReleaseMat( &Fstumps);
  cvReleaseMat( &Fn);
  cvReleaseMat( &Fx);
  cvReleaseMat( &Cx);

  for (int i=0;i<Nvecs;i++)
    cvReleaseMat( &feature_entry.features[i] );
}

#pragma warning( pop )
