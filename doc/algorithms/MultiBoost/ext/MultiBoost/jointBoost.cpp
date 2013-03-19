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
#include "jointBoost.h"
#include "parameters.h"

using std::vector;

jointBoost::jointBoost(void)
{
}

jointBoost::~jointBoost(void)
{
}


void *jointBoost::icvClearAlloc(int size)
{
    void *ptr = 0;
    if( size > 0 )
    {
        ptr = cvAlloc(size);
        memset(ptr,0,size);
    }
    return ptr;
}

void jointBoost::icvFreeMatrixArray(CvMat ***matrArray,int numMatr)
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

void jointBoost::CreateArtificialData(void)
{
  assert(!"when is this called?");  // take care of the Parameters here.

  CvRNG rng_state = cvRNG(0xffffffff);
  int k, cluster_count = 3;
  int i, sample_count = 1000;
  int nFeat=59;
  int cont=0;

  CvMat* points = cvCreateMat( sample_count, 2, CV_32FC1 );
  CvMat* clusters = cvCreateMat( sample_count, 1, CV_32SC1 );
  CvMat* contador = cvCreateMat( cluster_count, 1, CV_32SC1 );
  CvMat* centroids = cvCreateMat( cluster_count, 2, CV_32FC1 );
  CvMat* theta = cvCreateMat( 1, nFeat, CV_32SC1 );
  CvMat* features = cvCreateMat(nFeat, sample_count,CV_32FC1 ); //59 features and 1000 samples
  CvMat* featurestest = cvCreateMat(nFeat, 101*101,CV_32FC1 ); //59 features and 10,002 samples
  CvMat* xt = cvCreateMat(10201, 1,CV_32FC1 ); //test data
  CvMat* yt = cvCreateMat(10201, 1,CV_32FC1 ); //test data

  Params=new parameters( cluster_count+1 ); //classes + background

  float acc=0;
  for( k = 0; k < nFeat; k++ )
    theta->data.fl[k]=(CV_PI/nFeat)*k;

  /* generate random sample from Uniform distribution */
  cvRandArr(&rng_state,points,CV_RAND_UNI,cvScalarAll(0),cvScalarAll(1));
  cvKMeans2( points, cluster_count, clusters, cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0 ));

  //calculates the centroids.
  cvZero(centroids);
  cvZero(contador);
  //adds all the vectors in order to average them
  for(int j = 0; j < sample_count; j++ )
  {
    int cluster=clusters->data.i[j];
    contador->data.i[cluster]++;
    i=0;
    centroids->data.fl[i+cluster*centroids->cols]=centroids->data.fl[i+cluster*centroids->cols]+points->data.fl[i+j*points->cols]; //add x
    i=1;
    centroids->data.fl[i+cluster*centroids->cols]=centroids->data.fl[i+cluster*centroids->cols]+points->data.fl[i+j*points->cols]; //add y
  }
  //Now divide by the total to find the average for centroid
  for(int j = 0; j < cluster_count; j++ )
  {
    i=0;
    centroids->data.fl[i+j*centroids->cols]=centroids->data.fl[i+j*centroids->cols]/contador->data.i[j];
    i=1;
    centroids->data.fl[i+j*centroids->cols]=centroids->data.fl[i+j*centroids->cols]/contador->data.i[j];
  }

  //relabel to be 1 based, instead of zero based
  cvAddS(clusters,cvScalar(1),clusters);
  //Now label with 0 the points that are too fat from the center of any cluster
  for(int j = 0; j < sample_count; j++ )
  {
    i=0;
    float x=points->data.fl[i+j*points->cols];
    i=1;
    float y=points->data.fl[i+j*points->cols];


    float min_dist=100000;
    for(int k = 0; k < cluster_count; k++ )
    {
      i=0;
      float cx=centroids->data.fl[i+k*centroids->cols];
      i=1;
      float cy=centroids->data.fl[i+k*centroids->cols];
      float dist=sqrt(pow(cx-x,2)+pow(cy-y,2));

      if (dist<min_dist)
        min_dist=dist;
    }
    if (min_dist>0.25)
      clusters->data.i[j]=0; //label this point as a negative class (or background)
    else
      cont++;
  }


  //creates much more features
  for(int j = 0; j < sample_count; j++ )
  {
    i=0;
    float x=points->data.fl[i+j*points->cols];
    i=1;
    float y=points->data.fl[i+j*points->cols];
    //creates the new info
    float a,b;
    for(i = 0; i < nFeat; i++ )
    {
      a=cos(theta->data.fl[i])*(x-0.5);
      b=sin(theta->data.fl[i])*(y-0.5);
      features->data.fl[j+i*features->cols]=a+b;
    }


  }

  //Save all the features in a text file
  ////now save all the features to a text file
  //FILE *file; 
  //file = fopen("test_features.txt", "w"); /* create a file for writing */
  //if(file==NULL)
  //{
  //  printf("An error has occurred.\n");
  //  exit(0);
  //}

  //for (int i=0;i<sample_count;i++)
  //{
  //  fprintf(file, "%d ", clusters->data.i[i]);
  //  for (int j=0;j<nFeat;j++)
  //    fprintf(file, "%10.6f",features->data.fl[i+j*features->cols]); //ten spaces between the first digit of avery number. 6 decimals
  //  fprintf(file, "\n");
  //}
  //fclose(file); /* now close the file */

  //NOW, FINALLY RUN THE MULTI-CLASS

  clasificador classifier;
  classifier.objsize = Params->normalizedObjectSize;

  //train the detector
  jointBoosting(features,clusters,Params->T,Params->numTrainRounds,100,cluster_count, classifier); //CALLLLLL

  //create testing data
  cvZero(xt);
  cvZero(yt);
  int jt=0; int it=0;
  float accX=0;
  for (int i=0;i<101;i++)
  {
    float acc=0;
    for (int j=0;j<101;j++)
	{ 
      yt->data.fl[jt]=acc;
	  xt->data.fl[jt]=accX;
	  acc=acc+0.01;
	  jt++;
	}
    accX=accX+0.01;
  }

  for( k = 0; k < nFeat; k++ )
	  theta->data.fl[k]=(CV_PI/nFeat)*k;
  sample_count=101*101;
  //creates much more features
  for(int j = 0; j < sample_count; j++ )
  {
	  float x=xt->data.fl[j];
	  float y=yt->data.fl[j];
	  //creates the new info
	  float a,b;
	  for(i = 0; i < nFeat; i++ )
	  {
		  a=cos(theta->data.fl[i])*(x-0.5);
		  b=sin(theta->data.fl[i])*(y-0.5);
		  featurestest->data.fl[j+i*featurestest->cols]=a+b;
	  }
  }

  //test the detector
 int Nnodes=Params->T.size(); 
 CvMat* Fstumps = cvCreateMat( sample_count, classifier.nWeakClassifiers, CV_32FC1); //% F(n) is the function in node 'n'. 
 CvMat* Fn = cvCreateMat( sample_count, Nnodes, CV_32FC1); //% F(n) is the function in node 'n'.
 CvMat* Fx = cvCreateMat(sample_count,cluster_count, CV_32FC1 );
 CvMat* Cx = cvCreateMat(sample_count,cluster_count, CV_8UC1 );

 strongJointClassifier(featurestest, classifier, Params->TT,cluster_count, Cx, Fx, Fn, Fstumps);
 //strongJointClassifier(features, classifier, Params->T,cluster_count, Cx, Fx, Fn, Fstumps);

 //Visualization only now
 for (int n=0;n<cluster_count;n++)
 {
   CvMat gray_mat;
   CvMat colo;
   double minVal, maxVal; //for maximum and minimum
   CvPoint minPt, maxPt;

   cvGetCol(Fx,&colo,n);
   CvMat* col = cvCreateMat(colo.rows,colo.cols, CV_32FC1 );
   cvCopy(&colo,col);
   cvReshape(col, &gray_mat,0, 101 );
   CvMat* gray = cvCreateMat(gray_mat.rows,gray_mat.cols, CV_8U);
   cvMinMaxLoc( &gray_mat, &minVal, &maxVal, &minPt, &maxPt );
   double scale=255./(maxVal-minVal);
   double shift=-255.*minVal/(maxVal-minVal);
   cvConvertScaleAbs(&gray_mat,gray,scale,shift); //normalize it between 0-1
   IplImage* result = cvCreateImage( cvGetSize(gray), 8, 1 );
   cvCopy(gray,result);
   //cvSaveImage("resltado.bmp",result);

   //uchar* val = &CV_IMAGE_ELEM( result, uchar, 75, 50 ); //y,x
   //val = &CV_IMAGE_ELEM( result, uchar, 70, 60 ); //y,x
   //val = &CV_IMAGE_ELEM( result, uchar, 23, 11 ); //y,x
  
    cvReleaseMat( &col);
    cvReleaseMat( &gray);
    cvReleaseImage(&result);
 }



  cvReleaseMat( &Fstumps);
  cvReleaseMat( &Fn);
  cvReleaseMat( &Fx);
  cvReleaseMat( &Cx);

  cvReleaseMat( &points );
  cvReleaseMat( &clusters );
  cvReleaseMat( &features );
  cvReleaseMat( &featurestest );
  cvReleaseMat( &centroids );
  cvReleaseMat( &theta );
  cvReleaseMat( &xt );
  cvReleaseMat( &yt );

  delete [] classifier.a;
  delete [] classifier.b;
  delete [] classifier.th;
  delete [] classifier.bestnode;
  delete [] classifier.featureNdx; 
  //do not delete Params. It is making problems
}


void jointBoost::jointBoosting(CvMat* x, CvMat* c, binmat T, int Nrounds, int Nthresholds, int Nclasses,clasificador &classifier )
{
  int Nfeatures = x->height;
  int Nsamples = x->width;
  CvMat** Fx; // % Strong classifier, one per class:    H(v,1) ... H(v,Nclasses)
  CvMat** w; // % weights vector
  int featNdx, bestnode;
  float a=0;
  float b=0;
  float th=0;

  assert( classifier.objsize.width>0 && classifier.objsize.height>0 );

  CvMat* thresholds = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1);
  CvMat* temp = cvCreateMat(Nsamples,1, CV_8UC1 );
  CvMat* yJ = cvCreateMat( Nsamples,1, CV_32FC1 );
  CvMat* res = cvCreateMat(Nsamples,1, CV_32FC1 );
  CvMat* cole = cvCreateMat(Nsamples,1, CV_32FC1 );
  CvMat* temp1 = cvCreateMat( Nsamples,1, CV_8UC1 ); 
  CvMat* temp2 = cvCreateMat( Nsamples,1, CV_32FC1 ); 
  CvMat* cd = cvCreateMat( Nclasses,1, CV_32FC1 ); 
  CvMat* fa= cvCreateMat( Nclasses,1, CV_32FC1 ); 
  CvMat rowe;

  cvZero(thresholds);
  cvZero(cd);
  cvZero(fa);

  getThresholds(x,Nthresholds,thresholds);

  Fx = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  w = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);

  int Nnodes=T.size();

  tmpa.b0 = cvCreateMat( 1, Nclasses, CV_32FC1);
  tmpa.error0 = cvCreateMat( 1, Nclasses, CV_32FC1);
  //tmpa.update = cvCreateMat( 1, Nclasses, CV_32FC1);
  //tmpa.update ?????
  CvMat* b0 = cvCreateMat( 1, Nclasses, CV_32FC1);

  tmpa.Ac = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  tmpa.Bc = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  tmpa.Wp = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  tmpa.Wn = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);

  for (int i=0;i<Nclasses;i++)
  {
    tmpa.Ac[i] = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
    tmpa.Bc[i] = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
    tmpa.Wp[i] = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
    tmpa.Wn[i] = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  }

  // allocate memory for the classifier
  classifier.nWeakClassifiers = Nrounds;
  classifier.a = new float[classifier.nWeakClassifiers];
  classifier.b = new float[classifier.nWeakClassifiers];
  classifier.th = new float[classifier.nWeakClassifiers];
  classifier.featureNdx = new int[classifier.nWeakClassifiers];
  classifier.bestnode = new int[classifier.nWeakClassifiers];

  //CvMemStorage* storage = cvCreateMemStorage(0);
  //classifier.sharing = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( std::vector<int> ), storage );


  //here you may want to be able to read an exisiting classifier and 
  // add more rounds. But we leave this "TO DO"

  //Initialize weights and classifiers
  for (int i=0;i<Nclasses;i++)
  {
    Fx[i] = cvCreateMat( Nsamples, 1, CV_32FC1 );
    w[i] = cvCreateMat( Nsamples, 1, CV_32FC1 );
  }

  for (int i=0;i<Nclasses;i++)
  {
    cvZero(Fx[i]);
    cvSet(w[i],cvScalar(1));
  }

  int init=0; //number of rounds

  for (int m=init;m<Nrounds;m++) //for very round
  //for (int m=init;m<4;m++) //for very round
  {
    fitStumpNode(x, c, w, thresholds, T, Nclasses, a , b, th, featNdx, bestnode, b0, &tmpa, m);
    classifier.a[m]=a;
    classifier.b[m]=b;
    classifier.th[m]=th;
    classifier.featureNdx[m]=featNdx;
    classifier.bestnode[m] = bestnode;
    classifier.sharing.push_back(tmpa.update);
    
    vector<int>jn_vec;
    jn_vec=classifier.sharing.at(m);

    for (int n=0;n<jn_vec.size();n++)
    {
      int jn=jn_vec.at(n);	
      cvCmpS( c, jn+1, temp, CV_CMP_EQ );
      cvConvertScale(temp,yJ,2./255.,-1);
      cvGetRow(x,&rowe,featNdx);
      cvTranspose(&rowe,cole);
      cvCmpS( cole, th, temp, CV_CMP_GT );
      cvConvertScale(temp,res,a/255.,b);
      cvAdd(Fx[jn],res,Fx[jn]);

      cvMul(yJ,Fx[jn],temp2,-1);
      cvExp(temp2,w[jn]);
      
      // true hits
      cvCmpS( yJ, 0, temp, CV_CMP_GT );
      cvConvertScale(temp,res,1./255.);
      CvScalar total=cvSum(res);
      cvCmpS( Fx[jn], 0, temp1, CV_CMP_GT );
      cvAnd(temp,temp1,temp);
      cvConvertScale(temp,res,1./255.);
      CvScalar num=cvSum(res);
      cd->data.fl[jn]=num.val[0]/total.val[0];

      //false alarms
      cvCmpS( yJ, 0, temp, CV_CMP_LT );
      cvConvertScale(temp,res,1./255.);
      total=cvSum(res);
      cvCmpS( Fx[jn], 0, temp1, CV_CMP_GT );
      cvAnd(temp,temp1,temp);
      cvConvertScale(temp,res,1./255.);
      num=cvSum(res);
      fa->data.fl[jn]=num.val[0]/total.val[0];  
    }
    printf( "Round %d - best feature = %d shared among %d classifier, with classes=",m, featNdx,jn_vec.size() );
	for (int n=0;n<jn_vec.size();n++)
    {
      int jn=jn_vec.at(n);	
	  printf( ",%d",jn);
	}

    float CDaver,CDmini,CDmaxi;
    float FAaver,FAmini,FAmaxi;
    CDaver=0;CDmini=1;CDmaxi=0;
    FAaver=0;FAmini=1;FAmaxi=0;
    for (int n=0;n<Nclasses;n++)
    {
      int jn=n;	
      CDaver=CDaver+cd->data.fl[jn];
      FAaver=FAaver+fa->data.fl[jn];
      if (cd->data.fl[jn]>CDmaxi)
        CDmaxi=cd->data.fl[jn];
      if (fa->data.fl[jn]>FAmaxi)
        FAmaxi=fa->data.fl[jn];
      if (cd->data.fl[jn]<CDmini)
        CDmini=cd->data.fl[jn];
      if (fa->data.fl[jn]<FAmini)
        FAmini=fa->data.fl[jn];
    }
    CDaver=CDaver/Nclasses;
    FAaver=FAaver/Nclasses;
    printf("\n");    
    printf( "performance on training, cd = %1.2f [%1.2f %1.2f]\n",CDaver, CDmini,CDmaxi);
    printf( "performance on training, fa = %1.2f [%1.2f %1.2f]\n",FAaver, FAmini,FAmaxi);
    printf("\n");
  }

  // remember to return values of calling function
  icvFreeMatrixArray(&Fx,Nclasses); // release strong classifiers 
  icvFreeMatrixArray(&w,Nclasses); // release weights
  icvFreeMatrixArray(&tmpa.Ac,Nclasses); // release strong coeefficients
  icvFreeMatrixArray(&tmpa.Bc,Nclasses); // release strong coeefficients
  icvFreeMatrixArray(&tmpa.Wp,Nclasses); // release strong coeefficients
  icvFreeMatrixArray(&tmpa.Wn,Nclasses); // release strong coeefficients
  cvReleaseMat(&tmpa.b0);
  cvReleaseMat(&b0);
  cvReleaseMat(&temp);
  cvReleaseMat(&temp1);
  cvReleaseMat(&temp2);
  cvReleaseMat(&yJ);
  cvReleaseMat(&cole);
  cvReleaseMat(&res);
  cvReleaseMat(&tmpa.error0);
  cvReleaseMat(&fa);
  cvReleaseMat(&cd);
  //cvReleaseMat(&tmpa.update);
  cvReleaseMat( &thresholds );
  //cvClearMemStorage( storage );
}

void jointBoost::getThresholds(CvMat* x, int Nthresholds,CvMat* thresholds)
{
//  % Looks for equally spaced thresholds. Same number of thresholds for each
//% feature. But each feature might have a different dinamic range, so the
//% thresholds might change.
//%
//% This function only is called once, at the beggining.

  double minX, maxX; //for maximum and minimum
  int Nfeatures=x->height;
  int Nsamples=x->width;
  CvMat rowe;
  CvMat* ths = cvCreateMat( 1, Nthresholds+2, CV_32FC1);
  CvMat* thres = cvCreateMat( 1, Nthresholds, CV_32FC1);
  CvMat tmp;
  CvMat ths2;
  
  for (int i=0;i<Nfeatures;i++)
  {
      cvGetRow(x, &rowe,i);
      cvMinMaxLoc(&rowe, &minX, &maxX);
      //cvRange( ths, minX, maxX );
	  linspace(ths, minX,maxX );
      CvRect recto=cvRect(1,0,ths->cols-2,1);
      cvGetSubRect( ths , &ths2, recto); 
      cvCopy(&ths2,thres);
      cvGetRow(thresholds, &tmp,i); //copy row
      cvCopy(thres,&tmp); //and modifiy it
  }
  cvReleaseMat(&thres); //deallocates memory
  cvReleaseMat( &ths );
}

void jointBoost::fitStumpNode(CvMat* x, CvMat* c, CvMat** w, CvMat* thresholds, binmat T,int Nclasses, //input parameters
                              float &aa,float &bb,float &thh,int &featNdx, int &bestnode, CvMat* b0,   //output paramaters 
                              temporary_storage *tmpa, int round)
{
  /*% All the heavy computations are only done on the leaves of the graph.
  % Then, the parameters are propagated to the other nodes.  
  % The thresholds are discretized (they can be different for each feature).*/
  int Nfeatures=x->height;
  int Nsamples=x->width;
  int Nnodes=T.size();
  int Nthresholds=thresholds->width;
  CvMat **Ac,**Bc,**Wp,**Wn;
  CvMat columna;
  double minVal, maxVal; //for maximum and minimum
  CvPoint ptMax,ptMin;
  int thNdx;

  
  CvMat* error0 = cvCreateMat( 1, Nclasses, CV_32FC1);
  //CvMat* update = cvCreateMat( 1, Nclasses, CV_32FC1);
  vector<int> update;
  CvMat* temp = cvCreateMat( Nsamples,1, CV_8UC1 );
  CvMat* yJ = cvCreateMat( Nsamples,1, CV_32FC1 );
  CvMat* temp2 = cvCreateMat( Nsamples,1, CV_32FC1 );
  CvMat* wJ = cvCreateMat( w[0]->rows,w[0]->cols, CV_32FC1 );
  CvMat* jj =cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* temp3 =cvCreateMat( Nfeatures, Nthresholds, CV_8UC1 );
  CvMat* Wns = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* Wps = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* bS = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* bS_temp = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* aS = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* aS_temp = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* SE = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* SE2 = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  CvMat* merr = cvCreateMat( 1, Nthresholds, CV_32FC1);

  CvMat* minSE = cvCreateMat( 1, Nnodes, CV_32FC1);
  CvMat* f = cvCreateMat( 1, Nnodes, CV_32FC1);

  CvMat* a = cvCreateMat( 1, Nnodes, CV_32FC1);
  CvMat* b = cvCreateMat( 1, Nnodes, CV_32FC1);
  CvMat* th = cvCreateMat( 1, Nnodes, CV_32FC1);

  //for (int i=0;i<Nsamples;i++)
  //   yJ->data.fl[i]= 1;//(int)c->data.i[i];

  //cvConvert(c,yJ);  //VERY IMPORTANT!!!!
  // Juan says: not sure, but this was probably for debugging
  //% First we compute the best parameters Ac, Bc, Wp, Wn for regression stumps for each
  //% independent class and for a set of predefined thresholds

  //Do this only if there was no trained classifier loaded
  //*******
  Ac = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  Bc = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  Wp = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);
  Wn = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nclasses);

  for (int i=0;i<Nclasses;i++)
  {
    Ac[i] = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
    Bc[i] = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
    Wp[i] = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
    Wn[i] = cvCreateMat( Nfeatures, Nthresholds, CV_32FC1 );
  }

  // First we compute the best parameters Ac, Bc, Wp, Wn for regression stumps for each
  // independent class and for a set of predefined thresholds
  if (round!=0) // means ont the first time
  {
    for (int i=0;i<Nclasses;i++)
    {
      cvCopy(tmpa->Ac[i],Ac[i]);
      cvCopy(tmpa->Bc[i],Bc[i]);
      cvCopy(tmpa->Wp[i],Wp[i]);
      cvCopy(tmpa->Wn[i],Wn[i]);
    }
    cvCopy(tmpa->b0,b0);
    cvCopy(tmpa->error0,error0);
    //cvCopy(tmpa->update,update);
    copy(tmpa->update.begin(),tmpa->update.end(),inserter(update,update.begin()));

  }
  else
  {
    //Set to zero
    for (int i=0;i<Nclasses;i++)
    {
      cvZero(Ac[i]);
      cvZero(Bc[i]);
      cvZero(Wp[i]);
      cvZero(Wn[i]);
    }
    
    for (int i=0;i<Nclasses;i++)
      update.push_back(i);  
  }
  //*****
  printf( "Update stumps for %d classes out of %d\n",update.size(),Nclasses);
  printf("Loop on leaves......\n" );
  printf("                      \n" );

  for (int i=0;i<update.size();i++)
  {
    int jn=(int)update.at(i);
    printf("\b\b\b\b\b\b\b\b\b\b\b\b class: %d\n", jn);
    // class label for class jn: -1=background, 1=data in classe jn
    // as opposed to Matlab, our classses are zero based 
    cvCmpS( c, jn+1, temp, CV_CMP_EQ );
    cvConvertScale(temp,yJ,2./255.,-1);
    cvCopy(w[jn],wJ);
    //we cannot normalize the weights per class: This is wrong => wJ = wJ / sum(wJ);

    //For each leave we fit two models: 1) a constant, 2) a stump
    // b0(jn) = sum(yJ .* wJ)/ sum(wJ);
    CvScalar total=cvSum(wJ);
    cvMul(yJ,wJ,temp2,1./total.val[0]);
    total=cvSum(temp2);
    b0->data.fl[jn]=total.val[0];
    //error0(jn) = sum(wJ.*(yJ - b0(jn)).^2);
    cvSubS(yJ,cvScalar(b0->data.fl[jn]),temp2);
    cvPow(temp2,temp2,2);
	  cvMul(wJ,temp2,temp2);
    
    total=cvSum(temp2);
    error0->data.fl[jn]=total.val[0];
    CvMat xx, thn, aC, bC, wP, wN;

    //stemp
    for (int n=0;n<Nfeatures;n++)
    {
      cvGetRow(x, &xx,n); //copy row
      cvGetRow(thresholds, &thn,n); //copy row
      cvGetRow(Ac[jn], &aC,n); //copy row
      cvGetRow(Bc[jn], &bC,n); //copy row
      cvGetRow(Wp[jn], &wP,n); //copy row
      cvGetRow(Wn[jn], &wN,n); //copy row

      getLeaveStump(xx, yJ, wJ, thn, aC, bC, wP, wN);
    }
  }

  // to avoid divided by zero
  for (int i=0;i<Nclasses;i++)
  {
      cvCmpS(Wp[i], 0, temp3, CV_CMP_EQ );
      cvConvertScale(temp3,jj,1./255.,0);
      cvAdd(Wp[i],jj,Wp[i]);
      //-----//
      cvCmpS(Wn[i], 0, temp3, CV_CMP_EQ );
      cvConvertScale(temp3,jj,1./255.,0);
      CvScalar stam=cvSum(jj);
      cvAdd(Wn[i],jj,Wn[i]);
  }

  // Now, search for best node.
 // First evaluate error at each node (for each node look for best feature
 // and best threshold to be shared by all the leaves
 for (int node=0;node<Nnodes;node++)
 {
   int *binary_vector; // the result is stored in T
   binary_vector=T.at(node);
   vector<int> myvector (binary_vector,binary_vector+Nclasses);

   vector<int> S, Sdiff;
   SearchVal(myvector,1,S,Sdiff);
   sort(S.begin(),S.end());
   sort(Sdiff.begin(),Sdiff.end());

   cvZero(Wns);
   cvZero(Wps);
   cvZero(bS);
   cvZero(aS);

   for (int i=0;i<S.size();i++)
   {
      int j=S.at(i);
      cvAdd(Wn[j],Wns,Wns);
      cvAdd(Wp[j],Wps,Wps);
      cvAdd(Bc[j],bS,bS); // divide!!!!
      cvAdd(Ac[j],aS,aS); // divide!!!!  
   }

   cvDiv(bS,Wns,bS);
   cvDiv(aS,Wps,aS);

   int sum_error0=0;
   for (int i=0;i<Sdiff.size();i++)
   {
      int j=Sdiff.at(i);
      sum_error0=error0->data.fl[j]+sum_error0; 
   }
   // Computes SE
    cvPow(bS,bS_temp,2);
    cvSubRS(bS_temp,cvScalar(1),bS_temp);
    

    cvPow(aS,aS_temp,2);
    cvSubRS(aS_temp,cvScalar(1),aS_temp);
    
   
    cvMul(Wps,aS_temp,SE);
    cvMul(Wns,bS_temp,SE2);

    cvAdd(SE,SE2,SE);
    cvAddS(SE,cvScalar(sum_error0),SE);

    // for this node, give best feature and best threshold
    cvReduce( SE, merr, -1,CV_REDUCE_MIN );
    cvMinMaxLoc(merr, &minVal, &maxVal,&ptMin,&ptMax);
    thNdx=ptMin.x;
    cvGetCol(SE,&columna,thNdx);
    cvMinMaxLoc(&columna, &minVal, &maxVal,&ptMin,&ptMax);
    featNdx=ptMin.y;

    a->data.fl[node]=aS->data.fl[featNdx*aS->cols+thNdx];
    b->data.fl[node]=bS->data.fl[featNdx*bS->cols+thNdx];
    a->data.fl[node]=a->data.fl[node]-b->data.fl[node];
    th->data.fl[node]=thresholds->data.fl[featNdx*thresholds->cols+thNdx];
    minSE->data.fl[node]=SE->data.fl[featNdx*SE->cols+thNdx];
    f->data.fl[node]=featNdx;
 }

 // Take best node and return best weak feature parameters and best feature index
 cvMinMaxLoc(minSE, &minVal, &maxVal,&ptMin,&ptMax);
 bestnode=ptMin.x;
 double minSEE=minVal;
 aa=a->data.fl[bestnode];
 bb=b->data.fl[bestnode];
 thh=th->data.fl[bestnode];
 featNdx=f->data.fl[bestnode];

// return intermediate values for efficiency. Most of them can be reused in the next iteration.
 for (int i=0;i<Nclasses;i++)
 {
   cvCopy(Ac[i],tmpa->Ac[i]);
   cvCopy(Bc[i],tmpa->Bc[i]);
   cvCopy(Wp[i],tmpa->Wp[i]);
   cvCopy(Wn[i],tmpa->Wn[i]);
 }
 
 cvCopy(b0,tmpa->b0);
 cvCopy(error0,tmpa->error0);

int *binary_vector; // the result is stored in T
 binary_vector=T.at(bestnode);
 vector<int> myvector (binary_vector,binary_vector+Nclasses);
 vector<int> S, Sdiff;
 SearchVal(myvector,1,S,Sdiff);
 sort(S.begin(),S.end());

 tmpa->update.clear();
 for (int i=0;i<S.size();i++)
    tmpa->update.push_back(S.at(i)); 

 //for (int i=0;i<S.size();i++)
 //     tmpa->update->at(i)=S.at(i);

 

  //*******  remember to return values of calling function
  //De allocate now
  cvReleaseMat( &error0 );
  //cvReleaseMat( &update );
  cvReleaseMat( &yJ );
  cvReleaseMat( &wJ );
  cvReleaseMat( &temp );
  cvReleaseMat( &temp2 );
  cvReleaseMat( &temp3 );
  cvReleaseMat( &jj );
  cvReleaseMat( &Wns );
  cvReleaseMat( &Wps );
  cvReleaseMat( &bS );
  cvReleaseMat( &bS_temp );
  cvReleaseMat( &aS );
  cvReleaseMat( &aS_temp );
  cvReleaseMat( &SE );
  cvReleaseMat( &SE2 );
  cvReleaseMat( &merr );
  cvReleaseMat( &minSE );
  cvReleaseMat( &f );
  cvReleaseMat( &a );
  cvReleaseMat( &b);
  cvReleaseMat( &th );

  icvFreeMatrixArray(&Ac,Nclasses); // release strong coeefficients
  icvFreeMatrixArray(&Bc,Nclasses); // release strong coeefficients
  icvFreeMatrixArray(&Wp,Nclasses); // release strong coeefficients
  icvFreeMatrixArray(&Wn,Nclasses); // release strong coeefficients

}

void jointBoost::getLeaveStump(CvMat x, CvMat* z, CvMat* w, CvMat thresholds,CvMat Ac, CvMat Bc,CvMat Wp,CvMat Wn)
{
  int Nfeatures=x.rows;
  int Ns=x.cols;
  int rows=thresholds.rows;
  int Nth=thresholds.cols;
  int s,thi;
  float ac,wpos;

  /* First compute total sums of weights and weighted labels */
  float Sw=0;
  float Swz=0;
  float wi, thresh;

  for (int s = 0; s < Ns; s++)
  {
    wi  = w->data.fl[s]; 
    Sw += wi;
    Swz += wi * z->data.fl[s]; 
  }


  /* Here compute regression parameters for each threshold */
  for (thi = 0; thi < Nth; thi++) 
  {
    thresh = thresholds.data.fl[thi];
    /*bc = 0; wneg = 0; */
    ac = 0; wpos = 0;
    //xs = x; ws = w; zs = z;
    for (s = 0; s < Ns; s++) 
      if (x.data.fl[s] > thresh)
      {
        wi  = w->data.fl[s];
        ac += wi * z->data.fl[s];
        wpos += wi;
      }

      Ac.data.fl[thi] = ac;
      Bc.data.fl[thi] = Swz - ac;
      Wp.data.fl[thi] = wpos;
      Wn.data.fl[thi] = Sw - wpos;
  }

  //***** remember to return values of calling function

}

void jointBoost::SearchVal(vector<int> victor,int val,vector<int> &S,vector<int> &Sdiff)
{
  for (int i=0;i<victor.size();i++)
     if (victor.at(i)==val)
       S.push_back(victor.size()-i-1); 
     else
       Sdiff.push_back(victor.size()-i-1); 
}


void jointBoost::Find(vector<float> victor,float val,vector<int> &indices)
{
  for (int i=0;i<victor.size();i++)
     if (victor.at(i)==val)
       indices.push_back(i);     
}

void jointBoost::linspace(CvMat* ths, float minX,float maxX )
{
    for( int k = 0; k <= ths->cols-2; k++ )
         ths->data.fl[k]=minX+k*(maxX-minX)/floor((float)ths->cols-1);
	  ths->data.fl[ths->cols-1]=maxX;
}

void jointBoost::strongJointClassifier(CvMat* x, clasificador &classifier, CvMat* TT, int Nclasses,  //input
                                       CvMat *Cx,CvMat *Fx,CvMat *Fn,CvMat *Fstumps) //output
{
/*% Applies multiclass classifier with function graph.
%
% Given the input samples 'x', the classifier parameters and the graph
% description T, computes the classifier for each input sample.
%
% Fx is the interesting output. Applying different thresholds to Fx we can
% move across the ROC of the classifier.
%
% Cx is just Fx with the threshold set to 0. Cx = Fx>0 but this might not
% be the best thing to do, so better work with Fx directly and decide
% later. Also, from Fx we can get probabilities.
%
% Fn is the contribution of each node to the final classifier*/

 int Nnodes=TT->cols;
 int Nfeatures=x->height;
 int Nsamples=x->width;//% Nsamples = Number of thresholds that we will consider
 CvMat* nodes = cvCreateMat( 1, classifier.nWeakClassifiers, CV_32FC1); 
 CvMat* temp = cvCreateMat( 1, classifier.nWeakClassifiers, CV_8UC1); 
 CvMat* stumpsInNode = cvCreateMat( 1, classifier.nWeakClassifiers, CV_32FC1); 
 
 cvZero(Fn);
 cvZero(Fstumps);
 CvMat rowe,cole;
 CvMat* temp1 = cvCreateMat(1,Nsamples, CV_8UC1 );
 CvMat* fm = cvCreateMat(1,Nsamples, CV_32FC1 );
 CvMat* fm_trans = cvCreateMat(Nsamples,1, CV_32FC1 );
 CvMat* temp2 = cvCreateMat(1, Nnodes,CV_8UC1 ); //for the tree
 CvMat* mascara = cvCreateMat(Nsamples, Nnodes,CV_8UC1 ); 
 CvMat* temp3 = cvCreateMat(Nsamples, Nnodes,CV_32FC1 ); 
 CvMat* fx = cvCreateMat(Nsamples, 1,CV_32FC1 );
 
 for (int i=0;i<classifier.nWeakClassifiers;i++)
    nodes->data.fl[i]=classifier.bestnode[i];

// 1) compute each node-function
 cvZero(Fn);// F(n) is the function in node 'n'. 
 int stump = 0;
 for (int n=0;n<Nnodes;n++)
 {
    cvCmpS( nodes, n, temp, CV_CMP_EQ );
    cvConvertScale(temp,stumpsInNode,1./255.,0);

	for (int m=0;m<classifier.nWeakClassifiers;m++)
		if (stumpsInNode->data.fl[m]==1.)
		{
			int featureNdx = classifier.featureNdx[m];
            float th = classifier.th[m];
            float a = classifier.a[m];
            float b = classifier.b[m];			
			cvGetRow(x,&rowe,featureNdx);
			cvCmpS( &rowe, th, temp1, CV_CMP_GT );
            cvConvertScale(temp1,fm,a/255.,b);
			cvGetCol(Fn,&cole,n);
			cvTranspose(fm,fm_trans);
			cvAdd(&cole,fm_trans,&cole);
			cvGetCol(Fstumps,&cole,stump);
			cvCopy(fm_trans,&cole);
			stump++;

		}
 }
/* 2) compute the final classifiers from the node-functions by applying the
% necessary combinations in order to get the classifiers corresponding at
% each class.
*/
 cvZero(Fx);
 for (int i=0;i<Nclasses;i++)
 {
   cvGetRow(TT,&rowe,i);
   cvCmpS( &rowe, 1., temp2, CV_CMP_EQ);
   cvRepeat(temp2,mascara);
   cvConvertScale(mascara,temp3,1./255);
   cvMul(Fn,temp3,temp3);
   cvReduce(temp3,fx,-1,CV_REDUCE_SUM);
   cvGetCol(Fx,&cole,i);
   cvCopy(fx,&cole);

 }
cvCmpS( Fx, 0, Cx, CV_CMP_GT); //remember that Cx is a 8U type matrix (0 and 255 in Cx->data.ptr )
//Cx = Fx>0;

cvReleaseMat( &nodes);
cvReleaseMat( &temp);
cvReleaseMat( &stumpsInNode);
cvReleaseMat( &temp1);
cvReleaseMat( &fm);
cvReleaseMat( &fm_trans);
cvReleaseMat( &mascara);
cvReleaseMat( &temp3);
cvReleaseMat( &fx);

}
