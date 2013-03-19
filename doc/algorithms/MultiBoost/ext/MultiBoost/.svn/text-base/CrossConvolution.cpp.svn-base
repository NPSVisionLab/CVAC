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
#include "CrossConvolution.h"
#include <time.h>


CrossConvolution::CrossConvolution(int nfilters)
{
   Nfilters=nfilters;
  //a[0]=1./16;a[1]=2./16;a[2]=1./16;a[3]=2./16;a[4]=4./16;a[5]=2./16;a[6]=1./16;a[7]=2./16;a[8]=1./16;     
   a[0]=1./16;a[1]=2./16;a[2]=1./16;a[3]=2./16;a[4]=4./16;a[5]=2./16;a[6]=1./16;a[7]=2./16;a[8]=1./16;     
  filtro=cvCreateMat( 3, 3, CV_32F);
  filtro->data.fl=a;

}


CrossConvolution::~CrossConvolution(void)
{
}

void *CrossConvolution::icvClearAlloc(int size)
{
    void *ptr = 0;
    if( size > 0 )
    {
        ptr = cvAlloc(size);
        memset(ptr,0,size);
    }
    return ptr;
}

void CrossConvolution::icvFreeMatrixArray(CvMat ***matrArray,int numMatr)
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

void CrossConvolution::convCrossConv(std::vector<CvMat*>& out, const IplImage* img, 
                                     const std::vector<CvMat*>& filters, const std::vector<int>& filtersIdx, 
                                     const CvMat *const *patches, const CvMat *const *locX, const CvMat *const *locY)
{
  // filtersIdx: an # that identifies the filter. We have from 0-3 always (four filters)
  
  CvMat *temp2=NULL, **outF=NULL;
  CvMat tmp;
#if 0
  time_t start_time,end_time; //for time performance
#endif

  //double minVal, maxVal;
  //CvPoint minPt, maxPt;

  //outT = cvCreateMat( img->height, img->width, CV_32FC1 );
  IplImage *temp=cvCreateImage( cvGetSize( img ), IPL_DEPTH_8U, 1 );
  IplImage *tempo=cvCreateImage( cvGetSize( img ),  IPL_DEPTH_32F, 1 );
  IplImage *imgcnv=cvCreateImage( cvGetSize( img ), IPL_DEPTH_32F, 1 );
 
  //patcho=cvCreateImage( cvSize( patches[0]->width, patches[0]->height ), IPL_DEPTH_8U, 1 );
  //cvConvertScaleAbs(patches[0],patcho);
  //cvSaveImage("patcho.bmp",patcho);   
  cvConvert(img,imgcnv);
 
#if 0
  if (nps::g_verbose>5) { start_time= 0; end_time= 0; time( &start_time ); }
#endif
     
  // 1) Convolution (features) with filters and smooth afterwards
  for (unsigned int i=0;i<filters.size();i++)
  {  
    cvFilter2D( imgcnv,tempo,filters[i]);
    cvFilter2D( tempo,out[i],filtro); //Apply Gaussian
    //cvConvertScaleAbs(out[i],temp);  //VERY IMPORTANT!!!!
  }

  //cvConvert(out[0],temp);
  // cvSaveImage("current.bmp",temp);   

  //More code according to the "nargin"
  if (!patches)
  {
    cvReleaseImage( &temp );
    cvReleaseImage( &tempo );
    cvReleaseImage( &imgcnv );
    icvFreeMatrixArray(&outF,filters.size()); // release patches
    return;
  }

   outF = (CvMat**)icvClearAlloc(sizeof(CvMat*)*filters.size()); //cleans memory allocated for filters
  for (unsigned int i=0;i<filters.size();i++)
    outF[i] = cvCreateMat( img->height, img->width, CV_32FC1 ); //creates headers for convolutions of filters
  for (unsigned int i=0;i<filters.size();i++) //creates a copy of the image after the 4 kernels filters are applied to those only. we will use them later
   cvCopy(out[i],outF[i]);  

#if 0
  if (nps::g_verbose>5) 
  {
    utilidades.print_time(start_time,end_time,"Running time for applying filters: %02d:%02d:%02d\n");
    start_time= 0; end_time= 0; time( &start_time ); 
  }
#endif
  // 2) Normalized correlation (template matching) with each patch
  for (int i=0;i<Nfilters;i++)
  {
    int resrows=out[i]->rows-patches[i]->rows+1;
    int rescols=out[i]->cols-patches[i]->cols+1;
    temp2= cvCreateMat(resrows, rescols, CV_32F);
    //temp=cvCreateImage( cvSize( rescols, resrows ), IPL_DEPTH_8U, 1 );
    if (Nfilters==1)  //this is true only on the detection step
        cvMatchTemplate( outF[i], patches[i], temp2, CV_TM_CCOEFF_NORMED );
    else   // more than one filter, for example in the dictionary creation or feature computation
        cvMatchTemplate( outF[filtersIdx[i]], patches[i], temp2, CV_TM_CCOEFF_NORMED );
        
    cvZero( out[i]);
    //cvConvertScaleAbs(temp2,temp,255.);  //VERY IMPORTANT!!!!
    //cvSaveImage("filter_matched.bmp",temp);   
    int subrows=cvCeil((patches[i]->rows-1)/2);
    int subcols=cvCeil((patches[i]->cols-1)/2);
    CvRect recto=cvRect(subcols,subrows,temp2->cols,temp2->rows); //for padding
    cvGetSubRect( out[i] , &tmp, recto); //the result is smaller than the original, we need to padd
    cvCopy(temp2,&tmp); //now padded
    //cvMinMaxLoc( out[i], &minVal, &maxVal, &minPt, &maxPt );
    cvReleaseMat(&temp2); //deallocates memory
  }
#if 0
  if (nps::g_verbose>5) 
  { 
   utilidades.print_time(start_time,end_time,"Running time for Matching Template: %02d:%02d:%02d\n");
   start_time= 0; end_time= 0; time( &start_time );
  }
#endif
  // 3) Convolution (location): separable filters
  // We exponentiate the correlation output to make the contrast of local maximum locations
  // to be enhanced: The convolution approximates a local max.
//  BTW: There is a clear way to optimize the location convolution: instead of creating very long vectors with mostly zeros
   // you can just extract a subwindow from the image to be processed, determined by the number of zeros, and apply only a 15x15 (or 15x1, or 1x15, but not 1x150)
  for (int i=0;i<Nfilters;i++)
  {
    cvPow( out[i], out[i],  3. );
    cvFilter2D( out[i],out[i],locX[i]);
    if (out[i]->rows!=out[i]->cols) //cannot use in-place operations! :(
    {
      temp2= cvCreateMat(out[i]->cols, out[i]->rows, CV_32F);
      cvTranspose( out[i], temp2);
      cvFilter2D( temp2,temp2,locY[i]);
      cvTranspose( temp2, out[i]);
      cvReleaseMat(&temp2); //deallocates memory
    }
    else
    { //much faster if is a square matrix
    cvTranspose( out[i], out[i]);
    cvFilter2D( out[i],out[i],locY[i]);
    cvTranspose( out[i], out[i]);
    }
  } 
#if 0
  if (nps::g_verbose>5) 
  { 
    utilidades.print_time(start_time,end_time,"Running time for Gaussian Location filters: %02d:%02d:%02d\n");
  }
#endif
  cvReleaseImage( &temp );
  cvReleaseImage( &tempo );
  cvReleaseImage( &imgcnv );
  icvFreeMatrixArray(&outF,filters.size()); // release patches
}

CvMat* CrossConvolution::DFTConvolution(CvMat* A, CvMat* B)
{ 
  // it is also possible to have only abs(M2-M1)+1�abs(N2-N1)+1
  // part of the full convolution result
  CvMat* conv = cvCreateMat( A->rows + B->rows - 1, A->cols + B->cols - 1, A->type );


  int dft_M = cvGetOptimalDFTSize( A->rows + B->rows - 1 );
  int dft_N = cvGetOptimalDFTSize( A->cols + B->cols - 1 );

  CvMat* dft_A = cvCreateMat( dft_M, dft_N, A->type );
  CvMat* dft_B = cvCreateMat( dft_M, dft_N, B->type );
  CvMat tmp;

  if (A->cols==1)
  {
    cvGetSubRect( dft_B, &tmp, cvRect(0,0,B->cols,B->rows));
    cvCopy( B, &tmp );
    cvCopy( &tmp, conv );
    return (conv);
  }

  // copy A to dft_A and pad dft_A with zeros
  cvGetSubRect( dft_A, &tmp, cvRect(0,0,A->cols,A->rows));
  cvCopy( A, &tmp );
  cvGetSubRect( dft_A, &tmp, cvRect(A->cols,0,dft_A->cols - A->cols,A->rows));
  cvZero( &tmp );
  // no need to pad bottom part of dft_A with zeros because of
  // use nonzero_rows parameter in cvDFT() call below

  cvDFT( dft_A, dft_A, CV_DXT_FORWARD, A->rows );

  // repeat the same with the second array
  cvGetSubRect( dft_B, &tmp, cvRect(0,0,B->cols,B->rows));
  cvCopy( B, &tmp );
  cvGetSubRect( dft_B, &tmp, cvRect(B->cols,0,dft_B->cols - B->cols,B->rows));
  cvZero( &tmp );
  // no need to pad bottom part of dft_B with zeros because of
  // use nonzero_rows parameter in cvDFT() call below

  cvDFT( dft_B, dft_B, CV_DXT_FORWARD, B->rows );

  cvMulSpectrums( dft_A, dft_B, dft_A, 0 /* or CV_DXT_MUL_CONJ to get correlation
                                         rather than convolution */ );

  cvDFT( dft_A, dft_A, CV_DXT_INV_SCALE, conv->rows ); // calculate only the top part
  cvGetSubRect( dft_A, &tmp, cvRect(0,0,conv->cols,conv->rows) );

  cvCopy( &tmp, conv );
  cvReleaseMat( &dft_A );
  cvReleaseMat( &dft_B );

  return (conv);
}


