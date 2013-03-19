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
#include "ComputeFeatures.h"
#include "utilities.h"
#include "parameters.h"
#include "fileManager.h"
#include "dictionary.h"
#include "CrossConvolution.h"

int CV_CDECL CompareScorePoints(const void* _a, const void* _b, void* userdata);
int CV_CDECL ComparePoints(const void* _a, const void* _b, void* userdata);
int CV_CDECL CompareDistanceCenter(const void* _a, const void* _b, void* userdata);


ComputeFeatures::ComputeFeatures( const parameters* par )
{
  Params=par;
  Nentries=Params->Nfilters*Params->patchesFromExample*Params->sampleFromImages*(Params->nclasses-1); 
  CrossOp=new CrossConvolution(Nentries); //start cross convultion object
  utilidades = new utilities( Params );
}

ComputeFeatures::~ComputeFeatures(void)
{
  delete CrossOp;
  delete utilidades;
}

void *ComputeFeatures::icvClearAlloc(int size)
{
  void *ptr = 0;
  if( size > 0 )
  {
    ptr = cvAlloc(size);
    memset(ptr,0,size);
  }
  return ptr;
}

void ComputeFeatures::icvFreeMatrixArray(CvMat ***matrArray,int numMatr)
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

void ComputeFeatures::CalcAverage(CvMat *Qk_1,std::vector<CvMat*>& out)
{
  for (int k=0;k<Nentries;k++)
    cvAddWeighted( Qk_1, (float)k/(k+1), out[k], 1./(k+1), 0, Qk_1 );
}

int CV_CDECL CompareScorePoints(const void* _a, const void* _b, void* userdata)
{
  CvMat *score = (CvMat*)userdata;
  CvPoint* a = (CvPoint*)_a;
  CvPoint* b = (CvPoint*)_b;
  float s1=score->data.fl[a->x+a->y*score->cols];
  float s2=score->data.fl[b->x+b->y*score->cols];
  return (s1<s2 ? -1: s1>s2 ? 1: 0);
}

int CV_CDECL ComparePoints(const void* _a, const void* _b, void* userdata)
{

  CvPoint* a = (CvPoint*)_a;
  CvPoint* b = (CvPoint*)_b;
  int y_diff = a->y - b->y;
  int x_diff = a->x - b->x;
  return y_diff ? y_diff : x_diff;

}

int CV_CDECL CompareDistanceCenter(const void* _a, const void* _b, void* userdata)
{

  CvPoint* a = (CvPoint*)_a;
  CvPoint* b = (CvPoint*)_b;
  CvPoint* ce = (CvPoint*)userdata;
  float s1 = pow((float)(a->x - ce->x),2)+pow((float)(a->y - ce->y),2);
  float s2 = pow((float)(b->x - ce->x),2)+pow((float)(b->y - ce->y),2);
  return (s1<s2 ? -1: s1>s2 ? 1: 0);

}

void ComputeFeatures::FindNotZero(IplImage* image, CvSeq* points_seq)
{
  CvPoint pixel;
  //// find pixels with '1's
  for(int  i = 0; i < image->height; i++ )
    for(int j = 0; j < image->width; j++ )
    {
      uchar* val = &CV_IMAGE_ELEM( image, uchar, i, j );
      if (val[0]>0)
      {  
        pixel.y=i;
        pixel.x=j;
        cvSeqPush( points_seq, &pixel );
      }
    }
}

void ComputeFeatures::FindNotZeroRnd(IplImage* mask, CvSeq* points_seq, int num,
                                     int largest_patch_size )
{
  CvPoint pixel;
  CvRNG rng = cvRNG(0xffffffff);
  int counter=0;
  int x,y;

  int mod_width = mask->width-largest_patch_size;
  int mod_height = mask->height-largest_patch_size;
  assert( mod_width>0 && mod_height>0 );

  while (counter<num)
  {
    //random coordiantes not in border
    x = cvRandInt(&rng)%(mod_width)+largest_patch_size/2;
    y = cvRandInt(&rng)%(mod_height)+largest_patch_size/2;
    // check that they do not belong to points in the mask
    uchar* val_msk = &CV_IMAGE_ELEM( mask, uchar, y, x );
    pixel.y=y;
    pixel.x=x;
    //cvSaveImage("2.bmp",mask);
    int elem_idx=0;
    cvSeqSearch(points_seq, &pixel,ComparePoints,0, &elem_idx, 0);
    // if the pixel is not '1' (part of the sillhoute, and doesn't exist on the sequence, add it)
    if  ((val_msk[0]==0)&&(elem_idx==points_seq->total))
    {  

      cvSeqPush( points_seq, &pixel );
      counter++;
    }

  }
}

#if 0
void ComputeFeatures::LocalMaximums(CvMat *score, IplImage* corner_mask, CvSeq* points_seq)
{
  //// From the OpenCV help HTML
  //// assume that the image is floating-point
  //CvPoint pixel;
  //IplImage* image = cvCreateImage( cvGetSize(score), IPL_DEPTH_32F, 1 );
  //cvConvert(score,image);
  //IplImage* corners = cvCloneImage(image);
  //IplImage* dilated_corners = cvCloneImage(image);
  //cvPreCornerDetect( image, corners, 7 );
  //cvDilate( corners, dilated_corners, 0, 1 );
  //cvSub( corners, dilated_corners, corners );
  //cvCmpS( corners, 0, corner_mask, CV_CMP_GE );  
  ////// find pixels with '1's
  //for(int  i = 0; i < corner_mask->height; i++ )
  //  for(int j = 0; j < corner_mask->width; j++ )
  //  {
  //    uchar* val = &CV_IMAGE_ELEM( corner_mask, uchar, i, j );
  //    if (val[0]>0)
  //    {  
  //      pixel.y=i;
  //      pixel.x=j;
  //      cvSeqPush( points_seq, &pixel );
  //    }
  //  }
  //  cvReleaseImage( &corners );
  //  cvReleaseImage( &dilated_corners );

  double quality = 0.01;
  double min_distance = 10; //this may work better with 5
  const int MAX_COUNT = 200;
  CvPoint2D32f *cornerPoints = 0;
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
        x=(int)cornerPoints[i].x;
        y=(int)cornerPoints[i].y;
        cvSeqPush( points_seq, &cornerPoints[i] );
        CV_IMAGE_ELEM(corner_mask,uchar,y,x) = (uchar)255;
    }
  
  cvFree(&cornerPoints);
  cvReleaseImage( &eig );
  cvReleaseImage( &temp );
  cvReleaseImage( &grey );
   
}
#endif // 0
void ComputeFeatures::LocalMaximums(CvMat *score, IplImage* corner_mask, CvSeq* points_seq, double min_distance)
{
  // IMPLEMENTATION FLAVOR 'A'
  //// From the OpenCV help HTML
  //// assume that the image is floating-point
  CvPoint pixel;
  IplImage* image = cvCreateImage( cvGetSize(score), IPL_DEPTH_32F, 1 );
  cvConvert(score,image);
  IplImage* corners = cvCloneImage(image);
  IplImage* dilated_corners = cvCloneImage(image);
  cvPreCornerDetect( image, corners, 7 );
  cvDilate( corners, dilated_corners, 0, 1 );
  cvSub( corners, dilated_corners, corners );
  cvCmpS( corners, 0, corner_mask, CV_CMP_GE );  
  // find pixels with '1's

  cvReleaseImage( &image );  //added by deborah 3:23pm

  for(int  i = 0; i < corner_mask->height; i++ )
    for(int j = 0; j < corner_mask->width; j++ )
    {
      uchar* val = &CV_IMAGE_ELEM( corner_mask, uchar, i, j );
      if (val[0]>0)
      {  
        pixel.y=i;
        pixel.x=j;
        cvSeqPush( points_seq, &pixel );
      }
    }
    cvReleaseImage( &corners );
    cvReleaseImage( &dilated_corners );

   cvSeqSort( points_seq, CompareScorePoints, score); //Order the points from strongest to hte weakest

   int k=points_seq->total;
   int x,y;
   int max_count = 200;
   CvPoint *ptr;
   ptr = (CvPoint*)cvAlloc( sizeof(CvPoint) * max_count) ;
   int count = 0;

   int min_dist = cvRound( min_distance * min_distance );
    /* select the strongest features */
    for(int i = 0; i < k; i++ )
    {
        int j = count;
        CvPoint* pt;
        pt = (CvPoint*)cvGetSeqElem( points_seq, k-i-1);

        y = pt->y;
        x = pt->x;

        if( min_dist != 0 )
        {
            for( j = 0; j < count; j++ )
            {
                int dx = x - ptr[j].x;
                int dy = y - ptr[j].y;
                int dist = dx * dx + dy * dy;

                if( dist < min_dist )
                    break;
            }
        }

        if( j == count )
        {
            ptr[count].x = x;
            ptr[count].y = y;
            if( ++count >= max_count )
                break;
        }
    }

    //Copy from ptr to sequence. This points are stronger and separated
    cvClearSeq(points_seq);
    cvZero(corner_mask);
    for(int i = 0; i < count; i++ )
    {
      x=ptr[i].x;
      y=ptr[i].y;

      pixel.y=y;
      pixel.x=x;
      //clear the sequence first
      cvSeqPush( points_seq, &pixel );
      CV_IMAGE_ELEM(corner_mask,uchar,y,x) = (uchar)255;
    }

    cvFree(&ptr);

  // IMPLEMENTATION FLAVOR 'B'
  //double quality = 0.01;
  //double min_distance = 10; //this may work better with 5
  //const int MAX_COUNT = 200;
  //CvPoint2D32f *cornerPoints = 0;
  //int count = MAX_COUNT;

  //cornerPoints = (CvPoint2D32f*)cvAlloc( sizeof(CvPoint2D32f) * MAX_COUNT) ;

  //IplImage* grey = cvCreateImage( cvGetSize(score), IPL_DEPTH_32F, 1 );
  //cvConvert(score,grey);
  //IplImage* eig = cvCreateImage( cvGetSize(grey), 32, 1 );
  //IplImage* temp = cvCreateImage( cvGetSize(grey), 32, 1 );
  //cvZero(corner_mask);

  //cvGoodFeaturesToTrack( grey, eig, temp, cornerPoints, &count,
  //                       quality, min_distance, 0, 3, 0, 0.04 );

  //int x,y;
  // for( int i = 0; i < count; i++ )
  //  {
  //      x=(int)cornerPoints[i].x;
  //      y=(int)cornerPoints[i].y;
  //      cvSeqPush( points_seq, &cornerPoints[i] );
  //      CV_IMAGE_ELEM(corner_mask,uchar,y,x) = (uchar)255;
  //  }
  //
  //cvFree(&cornerPoints);
  //cvReleaseImage( &eig );
  //cvReleaseImage( &temp );
  //cvReleaseImage( &grey );
  
}

/** calculate features for positive and negative samples. 
 * store them in the features_fname file (in binary format).
 */

//
void ComputeFeatures::CalculateFeatures(dictionary& dict,fileManager *DB, const std::string& features_fname)
{
  dictionary_storage* codebook = &dict.code_entry;

  CvRect newroi = cvRect(0,0,0,0);;  //
  int ncols,nrows;
  //char numb[255]; //for file writing
  //char node_name[255]; //for file writing
  int nd=0; //number of entries in the feature file
  int flag_inverted=0;

  //allocate memort for all the feature vectors
  int Nvecs = Params->numTrainImages*(Params->nclasses-1)*(Params->negativeSamplesPerImage+1);
  feature_storage feature_entry; //info of a single entry of the feature dataset;
  feature_entry.clase.resize( Nvecs );
  feature_entry.imagendx.resize( Nvecs );
  feature_entry.elemndx.resize( Nvecs );
  feature_entry.cx.resize( Nvecs );
  feature_entry.cy.resize( Nvecs );
  feature_entry.features.resize( Nvecs );
  for (int i=0;i<Nvecs;i++)
    feature_entry.features[i]= cvCreateMat( 1, Nentries, CV_32F);

  //for every object, scan every element in every image and extracts feature vectors for positive and negatives tranining vectors
  for (int obt=0;obt<Params->nclasses-1;obt++)
  {
    int ii=Params->sampleFromImages; //counter of images. start from images not used in the dictionary
    int total_elements=0; //counter of objects total

    while (total_elements<Params->numTrainImages)
	 //while (total_elements<1)
    {
	    printf("calculating features for class %d, object %d\n", obt, total_elements );
      int Nobjects=0;
      Nobjects=DB[obt].GetNumberObjectsinImage(ii);
        
      if(NULL==Nobjects)
      {
          printf("Nobjects==NULL - no objects in image or an error?");
          break;
      }

      for (int elem=0;elem<Nobjects;elem++)
        //for (int elem=0;elem<1;elem++) //lets pick only one element (sub-image) per image, to avoid homgenity
      {

        if (total_elements==Params->numTrainImages)
          break;

        //img=DB[obt].GetObjectinImage(ii,elem);

        //IplImage *msk0=DB[obt].GetMaskObjectinImage(ii,elem);
        //cvSaveImage("uno.bmp",msk0);
        IplImage *img=DB[obt].GetImage(ii);
        //cvSaveImage("imagen.bmp",img);
          
        IplImage *msk=DB[obt].GetMaskImage(ii);
        if(NULL==msk)
        {
            break;
        }
        //cvSaveImage("maska.bmp",msk);
        CvRect roi=DB[obt].GetROIinImage(ii,elem); 
        IplImage *gray= cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1);
        cvCvtColor(img,gray,CV_RGB2GRAY); //convert to grayscale

        // Get tight crop of the centered object to extract patches:
        //cvSaveImage("image.bmp",img);
        CvSize tis = Params->trainingImageSize( dict.dictobjsize );
        IplImage *newimg=DB[obt].Resize( gray, tis, dict.dictobjsize, LM_ORIGINAL, roi, newroi);
        IplImage *newmsk=DB[obt].Resize( msk, tis, dict.dictobjsize, LM_ORIGINAL, roi, newroi); 
        ncols=newimg->width;
        nrows=newimg->height;
        IplImage *color= cvCreateImage(cvSize(newimg->width, newimg->height), IPL_DEPTH_8U, 3);
        cvCvtColor(newimg,color,CV_GRAY2RGB);

        //cvSaveImage("newimage.bmp",newimg); 
        // You should check here if there cropped image is out of the boundaries of the main image
        //color= cvCreateImage(cvSize(newimg->width, newimg->height), IPL_DEPTH_8U, 3);
        //cvCvtColor(newimg,color,CV_GRAY2RGB);
        // Compute features
        std::vector<CvMat*> out; //result of convolutions and filters
        std::vector<CvMat*> abs_out; //result of convolutions and filters
        {
          time_t start_time,end_time; //for time performance
          start_time= 0; end_time= 0; time( &start_time );

          out.resize( Nentries );
          abs_out.resize( Nentries );

          for (int i=0;i<Nentries;i++)
          {
            out[i] = cvCreateMat( newimg->height, newimg->width, CV_32FC1 );
            abs_out[i] = cvCreateMat( newimg->height, newimg->width, CV_32FC1 );
            cvZero(abs_out[i]);
          }

          CrossOp->convCrossConv( out, newimg, codebook->filters, codebook->nfilter, codebook->patch, codebook->locX, codebook->locY);
          //utilidades.print_time(start_time,end_time,"Running time for convolution of all entries: %02d:%02d:%02d\n");
        }

        //temp= cvCreateImage(cvSize(newimg->width, newimg->height), IPL_DEPTH_32F, 1);
        //cvConvertScaleAbs(out[0],temp,255.);  //VERY IMPORTANT!!!!
        //cvSaveImage("matching.bmp",temp); 
        //absolute value of all the multi-dim matrix out[]
        int abs_mask = 0x7fffffff;
        for (int i=0;i<Nentries;i++)
          cvAndS( out[i], cvRealScalar(*(float*)&abs_mask), abs_out[i], 0 ); 

        ////calculates the average on the Z dimension.
        CvMat *score=cvCreateMat( newimg->height, newimg->width, CV_32FC1 );
        cvReleaseImage( &newimg );
        cvSetZero(score);
        time_t start_time,end_time; //for time performance
        start_time= 0; end_time= 0; time( &start_time );
        CalcAverage(score,abs_out);       
        //utilidades.print_time(start_time,end_time,"Running time for computing average: %02d:%02d:%02d\n");    

        //Saves the averaged image
        /*
        {
          IplImage* out_img = cvCreateImage( cvGetSize(score), 8, 1 );
          double minVal, maxVal; //for maximum and minimum
          cvMinMaxLoc(score, &minVal, &maxVal);
          cvConvertScaleAbs(score,out_img,255./maxVal);  
          utilidades.SaveIMGconv(out_img,obt, ii,elem);   
          cvReleaseImage( &out_img );
        }
        */
        
        //Finds local maxima
        IplImage* corner_mask = cvCreateImage( cvGetSize(score), 8, 1 );
        CvSeq* points_seq = 0;CvMemStorage* storage=0; //to store maximums peaks
        CvSeq* points_seq2 = 0;CvMemStorage* storage2=0; //to store maximums peaks outside the silhoute
        storage = cvCreateMemStorage(0);
        storage2 = cvCreateMemStorage(0);
        points_seq = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( CvPoint ), storage );
        points_seq2 = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( CvPoint ), storage2 );
        double min_distance = Params->min_IP_distance;
	      do {
              LocalMaximums(score,corner_mask,points_seq,min_distance);
		      min_distance = min_distance-1;
	      } while (points_seq->total==0 && min_distance>0);
	      assert( points_seq->total>0 );
        //cvSaveImage("maximums.bmp",corner_mask);
        //cvReleaseMat( &score );

        CornerReduce(corner_mask, score);

          //mask the objects so the maximum points are outside of the mask
          IplImage* masked = cvCreateImage( cvGetSize(score), 8, 1 );
          cvSetZero(masked);
          cvThreshold( newmsk, newmsk,128,255,CV_THRESH_BINARY);
          cvNot(newmsk,newmsk);
          cvCopy(corner_mask,masked,newmsk);
          // now copy the background points to a sequence.
          FindNotZero(masked, points_seq2);
          //cvSaveImage("masked.bmp",masked);

          //sort the interesting points (maximums) according to their score (high->low)
          cvSeqSort( points_seq2, CompareScorePoints, score);
        

        //removes unnecesary elements from sequence (if we have more points than necesary)
        int count=0;
        count=min(points_seq2->total, Params->negativeSamplesPerImage); 
        count=points_seq2->total-count;
        if (count<0) count=0;
        cvSeqPopMulti(points_seq2, 0, count, CV_FRONT);

        //if I do not have enough maximums, add points randomly (withuot repeating)
        if (Params->negativeSamplesPerImage-points_seq2->total>0)
        {   
          flag_inverted=1;
          cvNot(newmsk,newmsk);
          FindNotZeroRnd(newmsk,points_seq2,Params->negativeSamplesPerImage-points_seq2->total, dict.getLargestPatchSize());
        }
        if (Params->negativeSamplesPerImage-points_seq2->total>0)
        {
          throw std::runtime_error("can not find enough points for samplesPerImage");
        }

        // Store features in background image regions
        //SAVE ENTRIES IN DICTIONARY
        SaveEntries(feature_entry, Params->negativeSamplesPerImage, points_seq2->total, points_seq2, elem, ii, nd, out);

        // Store features at the center of the object 
        // Object center (we could be a bit tolerant with this and search of an object center that is one local maximum of the features score).
        CvPoint pt;
        pt.x = (int)(newroi.x+(float)newroi.width/2);
        pt.y = (int)(newroi.y+(float)newroi.height/2);
        if (flag_inverted==0)
          cvNot(newmsk,newmsk);

        //extract points maximum only inside the sillhouete;
		    // reduce the minimum distance if necessary 
		    // until at least one point was found
        /*
        {
		      CvPoint* closer_pt;
		      do 
		      {
			      cvSetZero(masked);
			      cvCopy(corner_mask,masked,newmsk);
			      cvClearSeq( points_seq);
			      FindNotZero(masked, points_seq);
			      min_distance = min_distance-1;
			      if (points_seq->total==0 && min_distance>0)
			      {
		              LocalMaximums(score,corner_mask,points_seq,min_distance);
			      }
		      } while (points_seq->total==0 && min_distance>0);
		      assert( points_seq->total>0 );

		      cvSeqSort( points_seq, CompareDistanceCenter, &pt); //Order according to proximity to the center of the sillohuete
		      closer_pt = (CvPoint*)cvGetSeqElem( points_seq, 0 );
		      assert( closer_pt );
          float dist=utilidades->Euclidian(closer_pt,&pt);
          if (dist<9)
          {
            pt.x=closer_pt->x;
            pt.y=closer_pt->y;
          }
        }
        */

        ProximityOrder(corner_mask, masked, newmsk, points_seq, min_distance, pt, score);
        cvReleaseImage( &newmsk );
  
        //copy the maximum inside the silhuette to the buffer
        for (int i=0;i<Nentries;i++)
          feature_entry.features[nd]->data.fl[i]=out[i]->data.fl[pt.y*out[i]->width+pt.x]; //corrected ->height changed to ->width
        feature_entry.clase[nd]=obt;
        feature_entry.cx[nd]=pt.x;
        feature_entry.cy[nd]=pt.y;
        feature_entry.imagendx[nd]=ii;
        feature_entry.elemndx[nd]=elem;
        nd++;

        // Visualize what is done
        cvLine(color, pt, pt,CV_RGB(255,0,0),3,8,0 ); //plot center
        for (int n=0;n<Params->negativeSamplesPerImage;n++)
        {
          CvPoint* pt = (CvPoint*)cvGetSeqElem( points_seq2, n );
          cvLine(color, *pt, *pt,CV_RGB(0,255,0),3,8,0 ); //plot center
        }
        utilidades->SaveIMGtrain(color,obt, ii,elem);     //saves the image

        total_elements++; //increment the total elements used

        //deallocate memory     
        cvReleaseMat(&score);
        for (int i=0;i<Nentries;i++)
        {
          cvReleaseMat( &out[i] );
          cvReleaseMat( &abs_out[i] );
        }
        cvReleaseImage( &img );
        cvReleaseImage( &msk );
        //cvReleaseImage( &msk0 );
        cvReleaseImage( &gray );
        cvReleaseImage( &corner_mask );
        cvReleaseImage( &masked );
        cvReleaseImage( &color );
        cvReleaseMemStorage( &storage );
        cvReleaseMemStorage( &storage2 );
      }
      ii++; //increment the number of images used

//      _CrtDumpMemoryLeaks();
    }
  }

  ////now save all the features to a text file
  //FILE *file; 
  //file = fopen("features.txt", "w"); /* create a file for writing */
  //if(file==NULL)
  //{
  //  printf("An error has occurred.\n");
  //  exit;
  //}

  //for (int i=0;i<Nvecs;i++)
  //{
  //  fprintf(file, "%d ", feature_entry.clase[i]);
  //  for (int j=0;j<Nentries;j++)
  //    fprintf(file, "%10.6f",feature_entry.features[i]->data.fl[j]); //ten spaces between the first digit of avery number. 6 decimals
  //  fprintf(file, "\n");
  //}
  //fclose(file); /* now close the file */

  WriteFeatures(feature_entry, features_fname, nd, Nentries);

  //deallocate memory
  for (int i=0;i<Nvecs;i++)
    cvReleaseMat( &feature_entry.features[i] );
}

void ComputeFeatures::WriteFeatures(const feature_storage& feature_entry, 
                                    const std::string& features_fname, int nd, int Nentries)
{
  time_t start_time= 0, end_time= 0; 
  time( &start_time );
  FILE *file; 
  file = fopen(features_fname.c_str(), "wb"); // create a file for writing 
  if(file==NULL)
  {
    printf("An error has occurred - can't write file %s.\n", features_fname.c_str());
	assert(0);
    exit(0);
  }

  for (int i=0;i<nd;i++) //if all the vectors are saved, nd=nVecs
  {
    fwrite(&feature_entry.clase[i], sizeof(int), 1,file );
    fwrite(feature_entry.features[i]->data.fl, sizeof(float), Nentries,file );
  }
  fclose(file); //now close the file 
  utilidades->print_time(start_time,end_time,"Running time for writing the data into the file: %02d:%02d:%02d\n");     
}




void ComputeFeatures::SaveEntries(struct feature_storage &feature_entry, int negativeSamplesPerImage, int total, CvSeq* points_seq2, int elem, int ii, int nd, std::vector<CvMat*>& out )
  {
        
        assert( total >= negativeSamplesPerImage );
     //   time_t start_time= 0; time_t end_time= 0;  time( &start_time );   
        for (int n=0;n<negativeSamplesPerImage;n++)
        {
          CvPoint* pt;
          pt = (CvPoint*)cvGetSeqElem( points_seq2, n );
          assert( pt );
          for (int i=0;i<Nentries;i++)
            feature_entry.features[nd]->data.fl[i]=out[i]->data.fl[pt->y*out[i]->width+pt->x]; //CHECK IF IS CORRECT!!!!!!
          feature_entry.clase[nd]=-1;
          feature_entry.cx[nd]=pt->x;
          feature_entry.cy[nd]=pt->y;
          feature_entry.imagendx[nd]=ii;
          feature_entry.elemndx[nd]=elem;
          nd++;
        }

    //  utilities * utilidades = null;
    //   utilidades->print_time(start_time,end_time,"Running time for copying feature vector to buffer for all 15 patches: %02d:%02d:%02d\n"); 
  };


void ComputeFeatures::CornerReduce(IplImage* corner_mask, CvMat* score)
{         //do not consider maximums close to the border (10 pixels from it)
          CvRect border;border.x=10;border.y=10; 
          border.width=corner_mask->width-2*border.x; border.height=corner_mask->height-2*border.y; 
          cvSetImageROI(corner_mask,border);
          IplImage* corner_reduc = cvCreateImage( cvGetSize(score), 8, 1 );
          cvSetZero(corner_reduc);
          cvSetImageROI(corner_reduc,border);
          cvCopy(corner_mask,corner_reduc);
          cvResetImageROI(corner_reduc);
          cvResetImageROI(corner_mask);
          cvCopy(corner_reduc,corner_mask);
          cvReleaseImage( &corner_reduc );
          //cvSaveImage("masked.bmp",corner_mask);
}

void ComputeFeatures::ProximityOrder(IplImage* corner_mask, IplImage* masked,IplImage *newmsk, CvSeq* points_seq, double min_distance, CvPoint& pt, CvMat* score )
{  
  do 
  {
    cvSetZero(masked);
    cvCopy(corner_mask,masked,newmsk);
    cvClearSeq( points_seq);
    FindNotZero(masked, points_seq);
    min_distance = min_distance-1;
    if (points_seq->total==0 && min_distance>0)
    {
    LocalMaximums(score,corner_mask,points_seq,min_distance);
    }
  } while (points_seq->total==0 && min_distance>0);
 
  assert( points_seq->total>0 );

  cvSeqSort( points_seq, CompareDistanceCenter, &pt); //Order according to proximity to the center of the sillohuete
  CvPoint* closer_pt
    = (CvPoint*)cvGetSeqElem( points_seq, 0 );
  assert( closer_pt );
  float dist=utilidades->Euclidian(closer_pt,&pt);
  if (dist<9)
  {
     pt.x=closer_pt->x;
     pt.y=closer_pt->y;
  }        
}
