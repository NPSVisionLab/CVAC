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
#include <fstream>
#include <iostream>
#include "StdAfx.h"
#include "runDetector.h"
#include "dictionary.h"
#include "utilities.h"
#include "CrossConvolution.h"
#ifdef HAVE_PROTOBUF
#include "trainingData.pb.h"
using namespace trainingData;
#endif // HAVE_PROTOBUF

using namespace std;

#pragma warning( push )
#pragma warning( disable : 4996 )

using nps::printv;

MultiBoost* MultiBoost::CreateMultiBoost()
{
  return new runDetector();
}

runDetector::runDetector()
: m_dictionary( NULL )
, Params( NULL )
, m_det( 0 )
, m_score( 0 )
, m_ids( 0 )
{
  nps::printv( 5, "runDetector constructor\n" );
  CrossOp=new CrossConvolution(1); //start cross convultion object
}

void *runDetector::icvClearAlloc(int size)
{
  assert( Params );
  void *ptr = 0;
  if( size > 0 )
  {
    ptr = cvAlloc(size);
    memset(ptr,0,size);
  }
  return ptr;
}

void runDetector::icvFreeMatrixArray(CvMat ***matrArray,int numMatr)
{
  assert( Params );
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

void runDetector::calc_window(int m, int n, int flag,CvMat *w)
{
  /*
   %CALC_WINDOW   Calculate the generalized cosine window samples.
  %   CALC_WINDOW Calculates and returns the first M points of an N point
  %   generalized cosine window determined by the 'window' string.

  % For the hamming and blackman windows we force rounding in order to achieve
  % better numerical properties.  For example, the end points of the hamming 
  % window should be exactly 0.08.
  */

  assert( w );
  float a0,a1,a2,a3,a4;
  switch (flag)
  {
  case HANN:
     // Hann window
     //    w = 0.5 * (1 - cos(2*pi*(0:m-1)'/(n-1))); 
      a0 = 0.5f;
      a1 = 0.5f;
      a2 = 0.f;
      a3 = 0.f;
      a4 = 0.f;
      break;
  case HAMMING:
     // Hamming window
     //    w = (54 - 46*cos(2*pi*(0:m-1)'/(n-1)))/100;
      a0 = 0.54f;
      a1 = 0.46f;
      a2 = 0.f;
      a3 = 0.f;
      a4 = 0.f;
      break;
  case BLACKMAN:
      // Blackman window
      //    w = (42 - 50*cos(2*pi*(0:m-1)/(n-1)) + 8*cos(4*pi*(0:m-1)/(n-1)))'/100;
      a0 = 0.42f;
      a1 = 0.5f;
      a2 = 0.08f;
      a3 = 0.f;
      a4 = 0.f;
      break;
  case FLATTOPWIN:
      // Flattop window
      // Coefficients as defined in the reference [1] (see flattopwin.m)
      a0 = 0.21557895f;
      a1 = 0.41663158f;
      a2 = 0.277263158f;
      a3 = 0.083578947f;
      a4 = 0.006947368f;
      break;
  }

  for (int i=0;i<m;i++)
  {
    float x=(float)i/(float)(n-1);
    w->data.fl[i]= a0 - a1*cos(2*CV_PI*x) + a2*cos(4*CV_PI*x) - a3*cos(6*CV_PI*x) + a4*cos(8*CV_PI*x);
  } 
}

void runDetector::hammin(int n,CvMat *w)
{
  assert( w );

  if  (n%2==0)
  {
    // Even length window
    int half=cvRound(n/2); 
    calc_window(half,n,HAMMING,w);
    int i=half; 
    for (int j=half-1;j>=0;j--)
    {
      w->data.fl[i]=w->data.fl[j];
      i++;
    } 

  }
  else
  {
    // Odd length window
    int half=cvRound((n+1)/2);
    calc_window(half,n,HAMMING,w);
    int i=half; 
    for (int j=half-2;j>=0;j--)
    {
      w->data.fl[i]=w->data.fl[j];
      i++;
    } 
  }
}


void runDetector::LocalMaximums(CvMat *score, IplImage* corner_mask, CvSeq* points_seq)
{
  assert( Params );

  double quality = 0.05;
  double min_distance = 5; //this may work better with 5
  const int MAX_COUNT = 200;
  CvPoint2D32f *cornerPoints = 0;
  CvPoint pt;
  int count = MAX_COUNT;

  cornerPoints = (CvPoint2D32f*)cvAlloc( sizeof(CvPoint2D32f) * MAX_COUNT) ;

  IplImage* grey = cvCreateImage( cvGetSize(score), IPL_DEPTH_32F, 1 );
  cvConvert(score,grey);
  IplImage* eig = cvCreateImage( cvGetSize(grey), 32, 1 );
  IplImage* temp = cvCreateImage( cvGetSize(grey), 32, 1 );
  cvZero(corner_mask);

  cvGoodFeaturesToTrack( grey, eig, temp, cornerPoints, &count,
                         quality, min_distance, 0, 3, 0, 0.04 );

  int x,y;
   for( int i = 0; i < count; i++ )
    {
        pt.x=(int)cornerPoints[i].x;
        pt.y=(int)cornerPoints[i].y;
        x=pt.x;
        y=pt.y;
        cvSeqPush( points_seq, &pt );
        CV_IMAGE_ELEM(corner_mask,uchar,y,x) = (uchar)255;
    }

  cvFree(&cornerPoints);
  cvReleaseImage( &eig );
  cvReleaseImage( &temp );
  cvReleaseImage( &grey );
   
}

float runDetector::minDistance(CvSeq *points_seq,double &minVal,double &maxVal,CvPoint &minPt, CvPoint &maxPt)
{
  double minD; 
  //CvPoint minPt, maxPt; //for maximum and minimum
  CvMat* distance = cvCreateMat(points_seq->total, points_seq->total, CV_32FC1 );
  CvMat* Di = cvCreateMat(points_seq->total, points_seq->total, CV_32FC1 );
  utilidades->DistanceMatrix(points_seq,points_seq,distance);
  cvSetIdentity(Di);
  cvScaleAdd( Di, cvScalar(1000), distance, distance );
  cvMinMaxLoc( distance, &minVal, &maxVal, &minPt, &maxPt );
  minD=minVal;
  cvReleaseMat( &distance);
  cvReleaseMat( &Di);
  return (minD);
}


void runDetector::singleScaleJointDetector(IplImage* newimg, clasificador &classifier,CvMat *TT,
                                           dictionary_storage codebook, CvSeq* det, CvSeq* score, CvSeq* ids)
{
  /*
  % This runs the detector at single scale.
  %
  % In order to build a multiscale detectors, you need to loop on scales.
  % Something like this:
  % for scale = 1:Nscales
  %    img = imresize(img, .8, 'bilinear');
  %    Score{scale} = singleScaleBoostedDetector(img, data);
  % end*/
  assert( Params );
  nps::printv( 5, "runDetector singleScaleJointDetector\n" );

  // xxx
#if 0
  for (int i=0; i<10; i++)
  {
    CvRect rect;
    float f = i;
    int cls = i%4;
    rect.x = i*25;
    rect.y = i*50;
    rect.width = 50;
    rect.height = 80;
    cvSeqPush( det, &rect );
    cvSeqPush( score, &f );
    cvSeqPush( ids, &cls );
  }
  return;
#endif 
   
  int *nodes; 
  int Nclasses=TT->rows;
  int Nnodes=TT->cols;
  nodes=classifier.bestnode;
  int rows=newimg->height;
  int cols=newimg->width;
  CvMat **Fn, **Fx, **s; 
  double minVal, maxVal; //for maximum and minimum
  CvPoint minPt, maxPt; //for maximum and minimum

  Fn = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nnodes);
  Fx = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  s = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  CvMat* nodeS = cvCreateMat( 1, classifier.nWeakClassifiers, CV_32FC1); 
  CvMat* temp = cvCreateMat( 1, classifier.nWeakClassifiers, CV_8UC1);
  CvMat* stumpsInNode = cvCreateMat( 1, classifier.nWeakClassifiers, CV_32FC1); 
  CvMat *Score;
  CvMat rowe,cole;
  CvMat* tmp2;

  for (int i=0;i<classifier.nWeakClassifiers;i++)
    nodeS->data.fl[i]=1.0*nodes[i]; //convert to float

  for (int i=0;i<Nnodes;i++)
  {
    Fn[i]= cvCreateMat( rows, cols, CV_32F);
    cvZero(Fn[i]);
  }

  std::vector<int> nfilter; nfilter.resize(1); nfilter[0] = -1;
  std::vector<CvMat*> feature; feature.resize(1);
  feature[0] = cvCreateMat( newimg->height, newimg->width, CV_32FC1 );
  CvMat* temp1 = cvCreateMat(newimg->height, newimg->width, CV_8UC1 );
  CvMat* fm = cvCreateMat(newimg->height, newimg->width, CV_32FC1 );
  CvMat* temp2 = cvCreateMat(1, Nnodes,CV_8UC1 ); //for the tree
  CvMat* temp3 = cvCreateMat(1, Nnodes,CV_32FC1 ); 
  //CvMat* temp4 = cvCreateMat(newimg->height, newimg->width, CV_32FC1 );
  int stumps;
  CvSeq* points_seq = 0;CvSeq* kt_seq = 0;CvMemStorage* storage=0;CvMemStorage* storage2=0 ; //to store maximums peaks

  //Each function in Fn is computed by adding the corresponding boosting rounds
  for (int n=0;n<Nnodes;n++)
  {
    cvCmpS( nodeS, n, temp, CV_CMP_EQ );
    cvConvertScale(temp,stumpsInNode,1./255.,0);

    stumps=0;
    for (int m=0;m<classifier.nWeakClassifiers;m++)
      // todo: this loop and if statement seem very inefficient
      if (stumpsInNode->data.fl[m]==1.)
      {
        int k = classifier.featureNdx[m];
        assert( k<codebook.filters.size() );
        //printf("calculating node %d, weak %d, feature %d\n", n, m, k );

        std::vector<CvMat*> filters;
        filters.push_back( codebook.filters[k] );
        CrossOp->convCrossConv( feature, newimg, filters, nfilter, 
          &codebook.patch[k], &codebook.locX[k], &codebook.locY[k]);

        float th = classifier.th[m];
        float a = classifier.a[m];
        float b = classifier.b[m];	

        cvCmpS( feature[0], th, temp1, CV_CMP_GT );   
        cvConvertScale(temp1,fm,a/255.,b);
        cvAdd(Fn[n],fm,Fn[n]);

        stumps++;
      }
  }

  cvMinMaxLoc( Fn[0], &minVal, &maxVal, &minPt, &maxPt );

  /*% 2) compute the final classifiers from the node-functions by applying the
  % necessary combinations in order to get the classifiers corresponding at
  % each class. */
  for (int i=0;i<Nclasses;i++)
  {
    Fx[i]= cvCreateMat( rows, cols, CV_32F);
    s[i]= cvCreateMat( rows, cols, CV_32F);
    cvZero(Fx[i]);
    cvZero(s[i]);
  }

  for (int i=0;i<Nclasses;i++)
  {
    cvGetRow(Params->TT,&rowe,i);
    cvCmpS( &rowe, 1., temp2, CV_CMP_EQ);
    cvConvertScale(temp2,temp3,1./255);

    for (int j=0;j<Nnodes;j++)
      if (temp3->data.fl[j]==1.0)
         cvAdd(Fn[j],Fx[i],Fx[i]);//  + data.detector(1).b0(i); %Calculate H(v,1), H(v,2) and H(v,3). Remember H(v,1)=G123(v)+G12(v)+G2(v).
  }

  /*
  % The follow part was modified from the original. The reason is that we
  % will present all the detections suggested by each detector, however if
  % there are overlaping detections from different detector, we are going to
  % pick the detection with the highest score only.
  */
  storage = cvCreateMemStorage(0);
  storage2 = cvCreateMemStorage(0);

  points_seq = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( CvPoint ), storage );
  kt_seq = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( int ), storage2 );

  int old_count=0;
  int count=0;
  CvMat* hammX = cvCreateMat(1,classifier.objsize.width, CV_32FC1 );
  CvMat* hammY = cvCreateMat(1,classifier.objsize.height, CV_32FC1 );

  for (int k=0;k<Nclasses;k++)
  {
    Score=Fx[k];
    /*% Look at local maximum of output score and output a set of detected object
    % bounding boxes.*/
    cvCmpS( Score, 0., temp1, CV_CMP_GT);
    cvConvertScale(temp1,s[k],1./255);

    hammin(classifier.objsize.width,hammX);
    hammin(classifier.objsize.height,hammY);

    cvFilter2D( s[k],s[k],hammX);

    if (s[k]->rows!=s[k]->cols) //cannot use in-place operations! :(
    {
      tmp2= cvCreateMat(s[k]->cols, s[k]->rows, CV_32F);
      cvTranspose( s[k], tmp2);
      cvFilter2D( tmp2,tmp2,hammY);
      cvTranspose( tmp2, s[k]);
      cvReleaseMat(&tmp2); //deallocates memory
    }
    else
    { //much faster if is a square matrix
      cvTranspose( s[k], s[k]);
      cvFilter2D( s[k],s[k],hammY);
      cvTranspose( s[k], s[k]);
    }

    cvMinMaxLoc( s[k], &minVal, &maxVal, &minPt, &maxPt );
    //imregionalmax
    //Finds local maxima
    IplImage* corner_mask = cvCreateImage( cvGetSize(s[k]), 8, 1 );
    cvZero(corner_mask);
    LocalMaximums(s[k],corner_mask,points_seq); 
    //cvSaveImage("maxi_detection.bmp",corner_mask);
    cvReleaseImage( &corner_mask);

    count=points_seq->total;
    int added=count-old_count;

    for( int i = 0; i < added; i++ )
      cvSeqPush( kt_seq, &k);

    old_count=count;
  }

  float minD=0; 
  
  if (points_seq->total==0) goto cleanup;

  // Do maximal supression now of detections
  // todo: look into the creation of these min/max values and points
  minD=minDistance(points_seq, minVal,maxVal,minPt, maxPt);

  // This number (15) is arbitrary. However represents the distance threshold of close pixels.
  // It is clear that this number depends on the scale used.  todo: make dependent on object size
  while (minD<15) 
  {

    int N=count;
    int p1=minPt.x;
    int p2=minPt.y;
    int d=p2; //drop p2

    int *kt_p1 = (int*)cvGetSeqElem( kt_seq, p1 );
    int *kt_p2 = (int*)cvGetSeqElem( kt_seq, p2 );

    CvPoint* pt1 = (CvPoint*)cvGetSeqElem( points_seq, p1 );
    CvPoint* pt2 = (CvPoint*)cvGetSeqElem( points_seq, p2 );

    if (*kt_p1==*kt_p2)
    {    
      pt1->x=(float)(pt1->x+pt2->x)/2;
      pt1->y=(float)(pt1->y+pt2->y)/2;
    }
    else
    {
      float ptScore1=s[*kt_p1]->data.fl[pt1->x+pt1->y*s[*kt_p1]->cols];
      float ptScore2=s[*kt_p2]->data.fl[pt2->x+pt2->y*s[*kt_p2]->cols];

      if (ptScore1<ptScore2)
        d=p1;
    }

    //setdiff now
    cvSeqRemove( points_seq, d );
    cvSeqRemove( kt_seq, d );  
    minD=minDistance(points_seq, minVal,maxVal,minPt,maxPt);
  }

  // final detections
  CvPoint* pt;
  CvRect rect;
  float sc;

  {
    //move out
    int *nDetections=new int[Nclasses];
    //

    for (int k=0;k<Nclasses;k++)
    {
      nDetections[k]=0;
      for (int n=0;n<points_seq->total;n++)
      {
        int *kt = (int*)cvGetSeqElem( kt_seq, n );
        if (*kt==k)
        {
          nDetections[k]=nDetections[k]+1;
          CvPoint* pt = (CvPoint*)cvGetSeqElem( points_seq, n );

          rect.width=codebook.avg_width[k];
          rect.height=codebook.avg_height[k];
      
          rect.x = pt->x-cvRound((float)rect.width/2);
          rect.y = pt->y-cvRound((float)rect.height/2);
          //assert(rect.x>=0 && rect.y>=0);
          /*
          if(rect.x<0 || rect.y<0)
          {
            printf("Nclasses:%d \n points_seq:%d \n rectx: %f \n recty: %f \n",
                     Nclasses,points_seq->total,rect.x,rect.y);
          }
          if(rect.<0 || rect.y<0)
          {
            printf("stop\n");
          }
          */
          
          cvSeqPush( det, &rect );
          sc=s[k]->data.fl[pt->x+pt->y*s[k]->cols];
          cvSeqPush( score, &sc );
          cvSeqPush( ids, &k );
        }

      }
    }
    //need to move out
    delete [] nDetections;
    // move out
  }

cleanup:

  // Clean all now
  cvReleaseMat( &feature[0] );
  cvClearMemStorage( storage );
  cvClearMemStorage( storage2 );

  cvReleaseMat( &nodeS);
  cvReleaseMat( &temp);
  cvReleaseMat( &stumpsInNode);
  cvReleaseMat( &temp1);
  cvReleaseMat( &fm);
  cvReleaseMat( &temp2);
  cvReleaseMat( &temp3);
  //cvReleaseMat( &temp4);
  cvReleaseMat( &hammX);
  cvReleaseMat( &hammY);
  icvFreeMatrixArray(&Fn,Nnodes); //free memory for multi-array
  icvFreeMatrixArray(&Fx,Nclasses); //free memory for multi-array
  icvFreeMatrixArray(&s,Nclasses); //free memory for multi-array
}

void runDetector::Init(const std::string& dictionary_fname, 
    const std::string& detector_fname)
{
  assert( Params==NULL );
  if ( Params ) throw std::runtime_error("runDetector initialized already");
  nps::printv( 5, "runDetector Initializing\n" );
  
  m_dictionary = new dictionary(); //Dictionary object
  
  
  if(dictionary_fname==("trainingData.pb"))
  {
#ifdef HAVE_PROTOBUF
    Data DictEntry;
    fstream in("trainingData4.pb", ios::in | ios::binary);
    if (!DictEntry.ParseFromIstream(&in)) {
      cerr << "Failed to parse trainingData.pb." << endl;
      exit(1);

    }
    
    m_dictionary->LoadDictionary(trainData);
 
    Params=new parameters( m_dictionary->getNumClasses() );
    m_dictionary->initialize( Params );
    utilidades = new utilities( Params );

    m_classifier.load(trainData);
#endif // HAVE_PROTOBUF  
  }
  //If it is not a pb file default to xml file
  else
  {
  
    m_dictionary->LoadDictionary(dictionary_fname.c_str());  //loads the dictionary
    
    Params=new parameters( m_dictionary->getNumClasses() );
    m_dictionary->initialize( Params );
    utilidades = new utilities( Params );


    m_classifier.load(detector_fname.c_str());
    
  }
  
  //detector results
    CvMemStorage* storage=0;CvMemStorage* storage1=0;CvMemStorage* storage2=0; 
    m_colores=new CvScalar[4]; m_colores[0]=CV_RGB(255,0,0);m_colores[1]=CV_RGB(0,255,0);m_colores[2]=CV_RGB(255,255,0);m_colores[3]=CV_RGB(0,0,255);
  
  
  
  if (m_classifier.objsize.width!=m_dictionary->dictobjsize.width
    || m_classifier.objsize.height!=m_dictionary->dictobjsize.height)
  {
    throw std::runtime_error( "incompatible dictionary and detector" );
  }

  //for detector responses
  storage = cvCreateMemStorage(0);
  storage1 = cvCreateMemStorage(0);
  storage2 = cvCreateMemStorage(0);

  m_det = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( CvRect ), storage );
  m_score = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( float ), storage1 );
  m_ids = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( int ), storage2 );
  nps::printv( 5, "runDetector Initialized successfully\n" );
}

void runDetector::GetMinImageSize( int *pWidth, int *pHeight )
{
  assert( m_classifier.objsize.width>0 && m_classifier.objsize.height>0 );
  *pWidth = m_classifier.objsize.width;
  *pHeight = m_classifier.objsize.height;
}

void runDetector::UnInit()
{
  if (NULL==Params) return;
  //cvClearMemStorage( m_storage );
  //cvClearMemStorage( m_storage1 );
  //cvClearMemStorage( m_storage2 );

  if (m_dictionary) delete m_dictionary; m_dictionary=NULL;
  delete [] m_classifier.a;
  delete [] m_classifier.b;
  delete [] m_classifier.th;
  delete [] m_classifier.bestnode;
  delete [] m_classifier.featureNdx; 
  delete [] m_colores;


  printv(1, "not deleting Params\n");
//  delete Params;
  Params = NULL;
}

/** cheap multi-scale detector: resizes the image
*/
void runDetector::Detect( IplImage* gray_img, IplImage* output_img,
    std::vector<MultiBoostMatch> &matches, 
    double start_scale, double stop_scale, double scale_inc_factor )
{
  printv( 5, "scale: %f, %f, %f\n", start_scale, stop_scale, scale_inc_factor );
  printv( 5, "H:%d W:%d\n", gray_img->height, gray_img->width);
  if (start_scale<1.0) 
  {
    static bool warned = false;
    if (!warned)
    {
      printv( 2, "upscaling is not recommended (requested upscale factor is %f)\n", 
        1.0/start_scale );
      warned = true;
    }
  }
  if (stop_scale<start_scale)
  {
    printv( 2, "stop scale must be ge than start scale\n" );
    stop_scale = start_scale;
  }
  if (scale_inc_factor<1.1)
  {
    printv( 2, "scale_inc_factor must be ge than 1.1\n" );
    scale_inc_factor = 1.1;
  }

  // first upscale the image if necessary; start with the smallest
  // scale larger than 1.0 and increase the image size from there
  double scale = 1.0/scale_inc_factor;
  IplImage *src = gray_img;
  IplImage *deleteme = NULL;
  while (scale>start_scale)
  {
    CvSize larger = cvSize( gray_img->width/scale, gray_img->height/scale );
    IplImage *resized = cvCreateImage( larger, src->depth, src->nChannels );
    cvResize( src, resized );
    if (deleteme) cvReleaseImage( &deleteme );
    unsigned int num_previous_matches = matches.size();
    Detect( resized, output_img, matches );

    // scale the new matches
    for (unsigned mcnt=num_previous_matches; mcnt<matches.size(); mcnt++)
    {
      matches[mcnt].m_x *= scale;
      matches[mcnt].m_y *= scale;
      matches[mcnt].m_w *= scale;
      matches[mcnt].m_h *= scale;
    }

    // for debugging, draw rectangle of current size into output_img
    if ( output_img && nps::g_verbose>4 )
    {
      int w = m_classifier.objsize.width*scale;
      int h = m_classifier.objsize.height*scale;
      cvRectangle( output_img, cvPoint( 0, 0 ), cvPoint( w, h ), cvScalar( 255, 0, 0 ) );
    }

    src = resized;
    deleteme = resized;
    scale /= scale_inc_factor;
  }
  if (deleteme) cvReleaseImage( &deleteme );

  // downsize the image; the size is given through calculation
  // based off the original (gray_img), but the shrinking in size
  // is done iteratively based off the previously shrunk image
  scale = start_scale;
  src = gray_img;
  deleteme = NULL;
  while (scale<=stop_scale)
  {
    CvSize smaller = cvSize( gray_img->width/scale, gray_img->height/scale );
    IplImage *resized = cvCreateImage( smaller, src->depth, src->nChannels );
    cvResize( src, resized );
    if (deleteme) cvReleaseImage( &deleteme );
    unsigned int num_previous_matches = matches.size();
    Detect( resized, output_img, matches );
    
    // scale the new matches
    for (unsigned mcnt=num_previous_matches; mcnt<matches.size(); mcnt++)
    {
      matches[mcnt].m_x *= scale;
      matches[mcnt].m_y *= scale;
      matches[mcnt].m_w *= scale;
      matches[mcnt].m_h *= scale;
      // draw rectangles at the proper scale
      if ( output_img )
      {
        cvRectangle( output_img, cvPoint( matches[mcnt].m_x, matches[mcnt].m_y), cvPoint( matches[mcnt].m_w +matches[mcnt].m_x, matches[mcnt].m_h+ matches[mcnt].m_y ), CV_RGB( 255, 0, 0 ) );
      }
    }
    
    // for debugging, draw rectangle of current size into output_img
    if ( output_img && nps::g_verbose>4 )
    {
      int w = m_classifier.objsize.width*scale;
      int h = m_classifier.objsize.height*scale;
      cvRectangle( output_img, cvPoint( 0, 0 ), cvPoint( w, h ), CV_RGB( 255, 0, 0 ) );
    }

    src = resized;
    deleteme = resized;
    scale *= scale_inc_factor;
  }
  if (deleteme) cvReleaseImage( &deleteme );
}

void runDetector::Detect( IplImage* gray_img, IplImage* output_img,
    std::vector<MultiBoostMatch> &matches )
{
  assert( Params );
	// run the detector
  cvClearSeq( m_det );
  cvClearSeq( m_score );
  cvClearSeq( m_ids );
	singleScaleJointDetector(gray_img,m_classifier,Params->TT,
    m_dictionary->code_entry,m_det,m_score,m_ids); // HERE CALL THE CLASSIFIER WITH THE IMAGE!!!

	// retreive the detections per class and plot them

  const double thresh = 0.0;
	for (int k=0;k<Params->nclasses-1;k++)
	{
		int total=0;
    int high=0;
		for (int n=0;n<m_ids->total;n++)
		{
			int* kt = (int*)cvGetSeqElem( m_ids, n );
			if (*kt==k)
			{
				float* sco = (float*)cvGetSeqElem( m_score, n );

        // only record the bounding box if the score is higher than thresh
        if (*sco>thresh) 
        {
				  CvRect* rect = (CvRect*)cvGetSeqElem( m_det, n );
				  int st=(int)cvCeil(*sco/50); //strength of the detection, normalized to 50
          MultiBoostMatch match( k, *sco, rect->x, rect->y, rect->width, rect->height);
          matches.push_back( match );
          high++;
          if (output_img) 
          {
            cvRectangle( output_img, cvPoint(rect->x,rect->y), 
                         cvPoint(rect->x+rect->width,rect->y+rect->height), 
                         m_colores[k],st, 8, 0);
          }
        }
				total=total++;
			}
		}
		printv( 5, "Class %d - number of detections (higher than %.0f) = %d (%d)\n", k, thresh, total, high );
	}
}

runDetector::~runDetector(void)
{
  UnInit();
}

#pragma warning(pop)
