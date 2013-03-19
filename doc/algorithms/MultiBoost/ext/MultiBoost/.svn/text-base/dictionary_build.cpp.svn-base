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
#ifdef HAVE_PROTOBUF
#include "trainingData.pb.h"
#endif // HAVE_PROTOBUF
#include "StdAfx.h"
#include "dictionary.h"
#include "utilities.h"
#include "fileManager.h"
#include "CrossConvolution.h"

#pragma warning( push )
#pragma warning( disable : 4996 )

void dictionary::BuildDictionary(fileManager *DB)
{
#ifdef HAVE_PROTOBUF
  trainingData::Data Dictdata;
#endif // HAVE_PROTOBUF
  int q=0;
  IplImage *gray, *temp8U,*color;
  CvRect newroi = cvRect(0,0,0,0);
  int avg_width, avg_height,ncols,nrows;
  CvSeq* points_seq = 0;
  CvSeq* random_seq = 0;
  CvMemStorage* storage=0;
  CvMemStorage* storage2=0;
  CvPoint pixel;
  CvRNG rng = cvRNG(-1);
  int Lx,Ly;
  int nd=0; //number of entry in dictionary
  float cx,cy;
  CvMat *gxe,*gye, *temp2; //spatial mask in x and y axis
  char numb[255];
  char node_name[255];
  
  assert( Params );
  std::vector<int> filtersIdx;
  filtersIdx.resize( Params->Nfilters );
  for (int i=0;i<Params->Nfilters;i++)
    filtersIdx[i]=i;

  dictobjsize = Params->normalizedObjectSize;
  CvMat* patchSize; //size of patches
  {
    int patchsize_start = 9;
    int shorter_len = min(dictobjsize.width, dictobjsize.height);
    if (shorter_len<31)
      patchsize_start = 7;
    if (shorter_len<25)
      patchsize_start = 5;
    int patch_increments = 9;
    if (shorter_len < patchsize_start+2*patch_increments)
      patch_increments = (shorter_len-patchsize_start)/2;
    patchSize = cvCreateMat(1,patch_increments, CV_32F );
    printf("using %d patch sizes from %d to %d pixels square\n",
      patch_increments, patchsize_start, patchsize_start+patch_increments*2 );

    for ( int i=0, val=patchsize_start; i<patch_increments; i++, val+=2 )
    { 
      cvSet1D( patchSize, i, cvScalar(val));
    }
    m_largest_patch_size = (int) cvGetReal1D( patchSize, patch_increments-1 );
    assert( m_largest_patch_size<shorter_len );
  }

  CvFileStorage* fs = cvOpenFileStorage( Params->GetDictionaryPath().c_str(), 0, CV_STORAGE_WRITE );
  std::vector<CvMat*> out; //result of convolutions and filters
  out.resize( Params->Nfilters ); // memory allocated for patches
  cvWriteInt( fs, "objsize_width", dictobjsize.width);
  cvWriteInt( fs, "objsize_height", dictobjsize.height);
  
#ifdef HAVE_PROTOBUF
  Dictdata.set_objsizeh(dictobjsize.height);
  Dictdata.set_objsizew(dictobjsize.width);
#endif // HAVE_PROTOBUF

  for (int obt=0;obt<Params->nclasses-1;obt++)
  {
    avg_width=0;
    avg_height=0;
    int ii=0; //counter of images
    int total_elements=0; //counter of objects total

    //Writes a comment in the XML file. with the class number
    strcpy(numb,"");
    strcpy(node_name,"Class_");
    sprintf(numb, "%d", obt);
    strcat(node_name,numb); // create a filename based in path 
    cvWriteComment(fs,node_name,0);
    // 

    while (total_elements<Params->sampleFromImages)
    {
      int Nobjects=0;
      Nobjects=DB[obt].GetNumberObjectsinImage(ii);

      //for (int elem=0;elem<Nobjects;elem++)
      for (int elem=0;elem<1;elem++) //lets pick only one element (sub-image) per image, to avoid homgenity
      {

        if (total_elements==Params->sampleFromImages)
          break;

        IplImage *img, *msk;
        CvRect roi;
        //img=DB[obt].GetObjectinImage(ii,elem);
        //msk0=DB[obt].GetMaskObjectinImage(ii,elem);
        //cvSaveImage("uno.bmp",msk0);
        img=DB[obt].GetImage(ii);
        msk=DB[obt].GetMaskImage(ii);
        roi=DB[obt].GetROIinImage(ii,elem);   
        cvSaveImage("__img.bmp",img);
        cvSaveImage("__msk.bmp",msk);
        assert( img && msk );
#ifdef _DEBUG
        {
          double min, max;
          cvMinMaxLoc( msk, &min, &max );
          assert( min<max );
        }
#endif

        storage = cvCreateMemStorage();
        storage2 = cvCreateMemStorage();
        points_seq = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( CvPoint ), storage );
        random_seq = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( CvPoint ), storage2 );

        gray= cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1);
        cvCvtColor(img,gray,CV_RGB2GRAY); //convert to grayscale
        // Get tight crop of the centered object to extract patches:
        // size of the images used for dicitonary (slighlty bigger than the silhouette)
        // todo: ask Juan: why bigger than objsize? why off-ratio? 
        CvSize dictionaryImageSize =
          cvSize(cvRound(dictobjsize.width*1.2),
                 cvRound(dictobjsize.height+dictobjsize.width*0.2));
        IplImage *newimg, *newmsk;
        newimg=DB[obt].Resize( gray, dictionaryImageSize, dictobjsize, LM_CENTERED, roi, newroi);
        newmsk=DB[obt].Resize( msk, dictionaryImageSize, dictobjsize, LM_CENTERED, roi, newroi); 
#ifdef _DEBUG
        {
          double min, max;
          cvMinMaxLoc( newmsk, &min, &max );
          assert( min<max );
          // this probably means the wrong annotation (mask) was found for this image
        }
#endif
        ncols=newimg->width;
        nrows=newimg->height;
        color= cvCreateImage(cvSize(newimg->width, newimg->height), IPL_DEPTH_8U, 3);
        cvCvtColor(newimg,color,CV_GRAY2RGB);
        //cvSaveImage("newimg.bmp",newimg);

        //if (min(nrows,ncols)<min(dictobjsize.width,dictobjsize.height))
        //{
        //Object center
        cx=newroi.x+(float)newroi.width/2;
        cy=newroi.y+(float)newroi.height/2;

        //Average Bounding box
        avg_width=newroi.width+avg_width;
        avg_height=newroi.height+avg_height;
        // Sample points on the edges
        IplImage *edgemap, *edgemapmask;
        edgemap = cvCreateImage( cvGetSize(newimg), 8, 1 );
        edgemapmask = cvCreateImage( cvGetSize(newimg), 8, 1 );

        double start_new_edge = 200;
        double link_edges = 50;
        do 
        {
          cvCanny( newimg, edgemap, link_edges, start_new_edge, 3 ); //VERY IMPORTANT! SENSIBLE TO PARAMETERS
          //cvSaveImage("edgemap.bmp",edgemap);
          cvAnd( edgemap, newmsk, edgemapmask);
          cvThreshold( edgemapmask, edgemapmask,2,255,CV_THRESH_BINARY);
          //cvSaveImage("edgemapmask.bmp",edgemapmask);

          // find pixels with '1's
          for(int  i = 0; i < edgemapmask->height; i++ )
          {
            for(int j = 0; j < edgemapmask->width; j++ )
            {
              uchar* val = &CV_IMAGE_ELEM( edgemapmask, uchar, i, j );
              if (val[0]>0)
              {  
                pixel.y=i;
                pixel.x=j;
                cvSeqPush( points_seq, &pixel );
              }
            }
          }
          start_new_edge *= 0.9; // try to find more edges if we don't have enough
          if (start_new_edge<link_edges)
          {
            link_edges = start_new_edge-1;
          }
        }
        while (start_new_edge>=1 && link_edges>=1 && points_seq->total < Params->patchesFromExample);
        if (start_new_edge<1 || link_edges<1)
        {
          throw std::runtime_error("can not find enough edges");
        }
        assert( points_seq->total >= Params->patchesFromExample );

          int count = points_seq->total;

          // creates in new list with all the points in random order
          for(int entries=0;entries<Params->patchesFromExample;entries++)
          {
            // choose random point out of the remaining ones
            assert( count>0 ); // if not, there are less than Params->patchesFromExample edge pixels 
            int idx = cvRandInt(&rng) % count;
            CvPoint *pt = (CvPoint*)cvGetSeqElem( points_seq, idx );
            int xo=pt->x;
            int yo=pt->y;    
            // keep coordinates within image size: 
            xo = max(xo, (int)max(patchSize->data.fl[patchSize->cols-1]+1,0)/2+1);
            yo = max(yo, (int)max(patchSize->data.fl[patchSize->cols-1]+1,0)/2+1);             
            xo = min(xo, ncols - (int)max(patchSize->data.fl[patchSize->cols-1]+1,0)/2+1);
            yo = min(yo, nrows - (int)max(patchSize->data.fl[patchSize->cols-1]+1,0)/2+1);
            pt->x=xo;
            pt->y=yo;
            cvSeqPush( random_seq, pt );
            // "remove" it by overriding it with the last element  
            *pt = *(CvPoint*)cvGetSeqElem( points_seq, count-1 );
            count--;
          }

          for (int i=0;i<Params->Nfilters;i++)
               out[i] = cvCreateMat( newimg->height, newimg->width, CV_32FC1 ); //creates headers for convolutions

          //Applies the bank of filters to the sub-image
          CrossOp->convCrossConv( out, newimg, m_filters, filtersIdx );

          // Crop patches: all filter outputs from each location
          for (int lp=0;lp<Params->patchesFromExample;lp++)
          {
            CvPoint *pt = (CvPoint*)cvGetSeqElem( random_seq, lp );
            int xo=pt->x;
            Lx = 2*abs(xo - (int)cx)+1;
            gxe=DeltaBlurredOffest(Lx,xo,cx);

            int yo=pt->y;
            Ly = 2*abs(yo - (int)cy)+1;
            gye=DeltaBlurredOffest(Ly,yo,cy);
            
            //The part below crops patches from the filtered images, patches from different sizes
            for (int lf=0;lf<Params->Nfilters;lf++)
            {
              int ra= (int) cvRandReal(&rng)* patchSize->cols;
              int p = cvRound(patchSize->data.fl[ra])-1; // random patch size
              CvRect pat = cvRect(xo-(int)p/2,yo-(int)p/2,p,p);  //roi of patch
              temp2= cvCreateMat(pat.width, pat.height, CV_32F);
              temp8U= cvCreateImage(cvSize(pat.width, pat.height), IPL_DEPTH_8U, 1);
              cvGetSubRect(out[lf],temp2,pat);
              cvConvert(temp2,temp8U); 

              //SAVE ENTRIES IN DICTIONARY              
              strcpy(numb,"");
              strcpy(node_name,"entry_");
              sprintf(numb, "%d", nd);
              strcat(node_name,numb); // create a filename based in path             
              //utilidades->SaveIMGDict(temp8U,node_name); //save patch
              cvStartWriteStruct(fs,node_name,CV_NODE_MAP);
              cvWriteInt( fs, "filter", lf ); //instead of saving the whole filter 3x3, we only save a reference index to the filter number (1,2,3,4)
              cvWrite( fs, "patch", temp2 ); //this has floats and iis large, so the dictionary file is large. May be later we can round this.
              //cvWrite( fs, "gxe", gxe); This creates a huge file, lets save instead only the offset, and later we will create the Gaussian vector
              //cvWrite( fs, "gye", gye); This creates a huge file, lets save instead only the offset, and later we will create the Gaussian vector
              cvWriteInt(fs,"cx",(int)cx); //stores the center of the newimage
              cvWriteInt(fs,"cy",(int)cy); // stores the center of the newimage
              cvWriteInt(fs,"xo",(int)xo); //stores the center of the patch
              cvWriteInt(fs,"yo",(int)yo); // stores the center of the patch
              cvWriteInt( fs, "Lx", Lx);
              cvWriteInt( fs, "Ly", Ly);
              cvWriteInt( fs, "imagendx", ii);
              cvWriteInt( fs, "elemndx",elem);
              cvEndWriteStruct(fs);
              
#ifdef HAVE_PROTOBUF
              std::cout<<std::endl;
              std::cout<<"Writing to protobuff"<<std::endl;
              std::cout<<std::endl;
              Dictdata.add_dictionary();
              Dictdata.mutable_dictionary(q);
              //Start saving in protobuff 
              Dictdata.mutable_dictionary(q)->set_filterindex(lf);
              //Dictdata.mutable_dictionary(lf)->add_patch(<#float value#>); This is where the data of the patch would go after linearization 
              //Dictdata.mutable_dictionary(lf).mutable_location()->;
              //Dictdata.mutable_dictionary(q)->set_cx((int)cx);
              //Dictdata.mutable_dictionary(q)->set_cy((int)cy);
              //Dictdata.mutable_dictionary(q)->set_xo((int)xo);
              //Dictdata.mutable_dictionary(q)->set_yo((int)yo);
              //Dictdata.mutable_dictionary(q)->set_lx(Lx);
              //Dictdata.mutable_dictionary(q)->set_ly(Ly);
              Dictdata.mutable_dictionary(q)->set_patchro(pat.width);
              Dictdata.mutable_dictionary(q)->set_patchco(pat.height);
#endif // HAVE_PROTOBUF
              q++;
              

              //Draw the patch in color
              color=utilidades->DrawPatch(color,pat);
              cvReleaseImage(&temp8U);
              nd++;
            }
            //            }
            //almost the end of the inner loop;
            cvReleaseImage( &edgemap );
            cvReleaseImage( &edgemapmask );
          }

          cvLine(color, cvPoint((int)cx,(int)cy), cvPoint((int)cx,(int)cy),CV_RGB(255,0,0),3,8,0 );
          utilidades->SaveIMGnew(color,obt, ii,elem);     //saves the image
          total_elements++; //incrmenet the number of object

          utilidades->SaveIMGmask(newmsk,obt, ii,elem); //saves the mask
          cvClearMemStorage( storage );
          cvClearMemStorage( storage2 );
          cvReleaseImage( &gray );
          cvReleaseImage( &color );
      } 

      ii++; //incrmenet the image number
    }
    //Caculates the average bounding box per class and writes it in the XML file
    avg_height=cvRound(avg_height/Params->sampleFromImages);
    avg_width=cvRound(avg_width/Params->sampleFromImages);
    strcpy(numb,"");
    strcpy(node_name,"class_");
    sprintf(numb, "%d", obt);
    strcat(node_name,numb); // create a filename based in path 

    cvStartWriteStruct(fs,node_name,CV_NODE_MAP);
    cvWriteInt( fs, "averageWidth", avg_width ); //i
    cvWriteInt( fs, "averageHeight", avg_height ); //i
    cvEndWriteStruct(fs);
    
  
    
#ifdef HAVE_PROTOBUF
    Dictdata.add_avgsize()->set_avgsizeh(avg_height);
    Dictdata.add_avgsize()->set_avgsizew(avg_width);
#endif // HAVE_PROTOBUF

  }
  
  
#ifdef HAVE_PROTOBUF
  std::fstream output("Dictionary.pb", std::ios::out | std::ios::binary| std::ios::trunc);
  Dictdata.SerializeToOstream(&output);
  output.close();
#endif // HAVE_PROTOBUF
  
  cvReleaseFileStorage( &fs ); //before exiting, close file
  for (int i=0;i<Params->Nfilters;i++)
    cvReleaseMat( &out[i] );
}

#pragma warning( pop )
