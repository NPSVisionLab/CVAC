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
#include "fileManager.h"


fileManager::fileManager()
{
}

fileManager::~fileManager(void)
{
}


void fileManager::CreateTrainingSamplesFromInfo( const char* infoname, const char* flnMasks )
{
  //Populates the structure used as DB
  char fullname[PATH_MAX];
  char maskFullname[PATH_MAX];
  char* filename;
  char *fln, *fln2, *fln3;

  FILE* info;
  //FILE* vec;
  //IplImage* src;
  //IplImage* sample;
  int total = 0;

  assert( infoname != NULL );

  info = fopen( infoname, "r" );
  if( info == NULL )
    fprintf( stderr, "Unable to open file: %s\n", infoname );

  strcpy( fullname, infoname );
  filename = strrchr( fullname, '\\' );
  if( filename == NULL )
    filename = strrchr( fullname, '/' );
  if( filename == NULL )
    filename = fullname;
  else
    filename++;

  int line = 0;
  bool eof = false;
  do
  {
    int count;

    if( fscanf( info, "%s %d", filename, &count ) != 2 )
    {
      if (feof( info ))
        eof = true;
      else
      {
        fprintf( stderr, "%s(%d) : parse error", infoname, line );
      }
      break;
    }
    db_images imInfo;
    imInfo.rects.resize( count );
    imInfo.anno=count;
    strcpy(imInfo.filename,"");
    strcat(imInfo.filename,filename);


    fln = strtok (filename,"/");
    fln2 = strtok (NULL, "/");
    while (fln != NULL)
    {
      fln3 = strtok (NULL, "/");
      if (fln3==NULL)
        break;
      fln = fln2;
      fln2 = fln3;
    }
    strcpy(maskFullname,"");
    strcat(maskFullname,flnMasks);
    strcat(maskFullname,"/");
    strcat(maskFullname,fln);
    strcat(maskFullname,"_");
    strcat(maskFullname,fln2);
    strcat(maskFullname,"_mask.jpg");

    strcpy(imInfo.maskFilename,maskFullname);
    //strcpy(tmp,"");
    //strcat(tmp,flnMasks);


    //src = cvLoadImage( filename, 0 );
    //cvSaveImage( "debugimg.bmp", src);

    for( int i = 0; i < count; i++ )
    {
      int x, y, width, height;
      if ( fscanf( info, "%d %d %d %d", &x, &y, &width, &height ) != 4 )
      {
        if (feof( info ))
          eof = true;
        else
        {
          fprintf( stderr, "%s(%d) : parse error", infoname, line );
        }
        break;
      }
      imInfo.rects[i].x=x;
      imInfo.rects[i].y=y;
      imInfo.rects[i].width=width;
      imInfo.rects[i].height=height;
    }
    vector_dataset.push_back(imInfo);
    line++;
  } while (!eof);
  fclose( info );
}

IplImage* fileManager::GetImage( int nImage) //return a pointer to the image "index" from the dataset
{
   IplImage* src;
   src = cvLoadImage(vector_dataset[nImage].filename);
   assert( src );
   return (src);
}

IplImage* fileManager::GetObjectinImage( int nImage,int nElem) //return a pointer to the image "index" from the dataset
{
   IplImage* src;
   IplImage* subImg;
   CvRect roi;
   src = cvLoadImage(vector_dataset[nImage].filename);
   roi=vector_dataset[nImage].rects[nElem]; 
   subImg = cvCreateImage( cvSize( roi.width, roi.height ), IPL_DEPTH_8U, 3 );
   cvSetImageROI( src, roi );
   cvCopy( src, subImg );
   cvResetImageROI( src);
   cvReleaseImage(&src);
   return (subImg);
}

IplImage* fileManager::GetMaskObjectinImage( int nImage,int nElem) //return a pointer to the image "index" from the dataset
{
   
  
   CvRect roi;
   IplImage* src = cvLoadImage(vector_dataset[nImage].maskFilename);
    if(NULL==src)
    {
        printf("could not find file %s\n", vector_dataset[nImage].maskFilename);
        return NULL;
    }
  // assert( src );
   roi=vector_dataset[nImage].rects[nElem];
   IplImage *subImg = cvCreateImage( cvSize( roi.width, roi.height ), IPL_DEPTH_8U, 3 );
   IplImage *subMsk = cvCreateImage( cvSize( roi.width, roi.height ), IPL_DEPTH_8U, 1 );
   cvSetImageROI( src, roi );
   cvCopy( src, subImg );
   cvResetImageROI( src);
   cvCvtColor(subImg,subMsk,CV_RGB2GRAY); //convert to grayscale
   cvThreshold( subMsk, subMsk,128,255,CV_THRESH_BINARY);
   cvReleaseImage(&src);
   cvReleaseImage(&subImg);
   return (subMsk);
}

CvRect fileManager::GetROIinImage( int nImage,int nElem) //return a pointer to the ROI of the object in image
{
   CvRect roi;
   roi=vector_dataset[nImage].rects[nElem];
   return (roi);
}

IplImage* fileManager::GetMaskImage( int nImage) //return a pointer to the mask image "index" from the dataset
{
   IplImage *src = cvLoadImage(vector_dataset[nImage].maskFilename);
   if (NULL==src)
   {
       printf( "cannot find file: %s\n", vector_dataset[nImage].maskFilename );
       return NULL;
   }
   IplImage *msk = cvCreateImage( cvSize( src->width, src->height ), IPL_DEPTH_8U, 1 );
   cvCvtColor(src,msk,CV_RGB2GRAY); //convert to grayscale
   cvReleaseImage(&src);
   cvThreshold( msk, msk,128,255,CV_THRESH_BINARY);
   return (msk);
}

int fileManager::GetNumberObjectsinImage(int nImage) //return the number of annotated objects in the image
{
    return (vector_dataset[nImage].anno);
}

int fileManager::GetNumberofImages(void)
{
  return ((int)vector_dataset.size());
}

int fileManager::GetNumberofObjectsinDB()
{
  int total=0;
  for (unsigned int i=0;i<vector_dataset.size();i++)
  {
   total=total+vector_dataset[i].anno; 
  }
return (total);
}

int fileManager::GetNumberofImagesinFile(const char* infoname)
{
  char fullname[PATH_MAX];
  char* filename;
  FILE* info;
  int error;
  int total,i,line;
  int x, y, width, height;

  assert( infoname != NULL );
  total = 0;

  info = fopen( infoname, "r" );
  if( info == NULL )
    fprintf( stderr, "Unable to open file: %s\n", infoname );

  strcpy( fullname, infoname );
  filename = strrchr( fullname, '\\' );
  if( filename == NULL )
    filename = strrchr( fullname, '/' );
  if( filename == NULL )
    filename = fullname;
  else
    filename++;

  int count=1;
  line=0;
  while (count != 0)
  {
    error = ( fscanf( info, "%s %d", filename, &count ) != 2 );
    if (error)
      break;
    for( i = 0; i < count; i++, total++ )
      error = ( fscanf( info, "%d %d %d %d", &x, &y, &width, &height ) != 4 );
    line++;
  }
  fclose( info );
  return (line);
}

int fileManager::GetNumberofObjectsinFile(const char* infoname)
{
  char fullname[PATH_MAX];
  char* filename;
  FILE* info;
  int error;
  int total,i,line;
  int x, y, width, height;

  assert( infoname != NULL );
  total = 0;

  info = fopen( infoname, "r" );
  if( info == NULL )
    fprintf( stderr, "Unable to open file: %s\n", infoname );

  strcpy( fullname, infoname );
  filename = strrchr( fullname, '\\' );
  if( filename == NULL )
    filename = strrchr( fullname, '/' );
  if( filename == NULL )
    filename = fullname;
  else
    filename++;

  int count=1;
  line=0;
  while (count != 0)
  {
    error = ( fscanf( info, "%s %d", filename, &count ) != 2 );
    if (error)
      break;
    for( i = 0; i < count; i++, total++ )
      error = ( fscanf( info, "%d %d %d %d", &x, &y, &width, &height ) != 4 );
    line++;
  }
  fclose( info );
  return (total);
}

/* resize image to the dictionary image size, but constraining to minimum size
 however this function does not check if the image extracted is out of the large image
 */
IplImage* fileManager::Resize(IplImage* src, CvSize maximagesize,CvSize normalizedObjectSize,int flag,CvRect roiObject,CvRect &newannotation)
{
   float scaling,cx,cy,nx,ny,Dx,Dy;
   IplImage *newimg, *dst;
   int *crop;
   crop=new int[4];
   int nrows,ncols,ncx,ncy;
   CvSize ms;

   ncols=src->width;
   nrows=src->height;

   // center and size of object ROI
   cx=(float)roiObject.x+roiObject.width/2;
   cy=(float)roiObject.y+roiObject.height/2;
   nx=(float)roiObject.width;
   ny=(float)roiObject.height;

   // scaling factor: detector/ROI, i.e. upscale ROI if <1 and downscale if >1
   scaling=min((float)normalizedObjectSize.height/ny,(float)normalizedObjectSize.width/nx); 

   if (flag==LM_CENTERED)
   {
   Dx = floor(maximagesize.width/2/scaling); Dy = floor(maximagesize.height/2/scaling);
   Dx=min(min(cx,Dx),src->width);Dy=min(min(cy,Dy),src->height);
   crop[0]=(int)cvRound(cx-floor(Dx)); //x1
   crop[1]=(int)cvRound(cx+ceil(Dx));  // x2
   crop[2]=(int)cvRound(cy-floor(Dy)); // y1
   crop[3]=(int)cvRound(cy+ceil(Dy));  // y2
   }

   if (flag==LM_ORIGINAL)
   {
   float px=cx/ncols;
   float py=cy/nrows;
   ms.height = cvRound(maximagesize.height/scaling);
   ms.width =  cvRound(maximagesize.width/scaling);
   ncx = cvRound(ms.width*px); ncy = cvRound(ms.height*py);
   crop[0]=cvRound(max(max(min(cx-ncx,cx-nx/2),1),cx+nx/2-ms.width));
   crop[1]=cvRound(min(crop[0]+ms.width, ncols));
   crop[2]=cvRound(max(max(min(cy-ncy,cy-ny/2),1),cy+ny/2-ms.height));  
   crop[3]=cvRound(min(crop[2]+ms.height, nrows));
   }

   newimg=CropIm(src,crop,roiObject,newannotation);
   //cvSaveImage("dos.bmp",newimg);
   dst=cvCreateImage(cvSize(cvRound(newimg->width*scaling), cvRound(newimg->height*scaling)), IPL_DEPTH_8U, 1);
   cvResize(newimg,dst,CV_INTER_AREA);
   //cvSaveImage("dos.bmp",dst);
   newannotation.x=cvRound(scaling*(roiObject.x-crop[0]));
   newannotation.y=cvRound(scaling*(roiObject.y-crop[2]));
   newannotation.width=cvRound(scaling*(roiObject.width));
   newannotation.height=cvRound(scaling*(roiObject.height));
   delete [] crop;
   cvReleaseImage( &newimg );
   return (dst);
}

IplImage* fileManager::CropIm(IplImage* src,int *crop,CvRect annotation,CvRect &newannotation)
{
  IplImage* img; 
  int rows=src->height;
   int cols=src->width;
   CvRect roi;

   crop[0] = max(crop[0],0);
  crop[1] = min(crop[1],cols-1);
  crop[2] = max(crop[2],0);
  crop[3] = min(crop[3],rows-1);

  roi.x=(int)crop[0];
  roi.y=(int)crop[2];
  roi.width=(int)crop[1]-(int)crop[0];
  roi.height=(int)crop[3]-(int)crop[2];

  img = cvCreateImage( cvSize( roi.width, roi.height ), IPL_DEPTH_8U, 1 );
  cvSetImageROI( src, roi );
  cvCopy( src, img );
  cvResetImageROI( src);
  return(img);
}
