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
#include "testDetector.h"
#include "dictionary.h"
#include "parameters.h"
#include "utilities.h"
#include "CrossConvolution.h"
#include "ComputeFeatures.h"
#include "jointBoost.h"
#include "trainDetector.h"
#include "fileManager.h"

void testDetector::ComputeDetectionsInDB(const std::string& dict_fname, int nClasses, 
                                         fileManager *DB, const std::string &classifier_fname)
{
  assert( Params );
  assert( m_dictionary==NULL );

  m_dictionary = new dictionary();
  m_dictionary->LoadDictionary(dict_fname.c_str());  //loads the dictionary
  m_dictionary->initialize( Params );
  dictionary_storage codebook = m_dictionary->code_entry;
  //IplImage* img, *gray,*msk, *newimg, *newmsk, *msk0,*color;  //dg commented
  
  CvRect roi;
  CvRect newroi = cvRect(0,0,0,0);;
  int ncols,nrows;
  time_t start_time,end_time; //for time performance
  //detector results
  CvSeq* det = 0;CvSeq* score = 0; CvSeq* ids = 0; CvMemStorage* storage=0;CvMemStorage* storage1=0;CvMemStorage* storage2=0; 
   CvScalar *colores=new CvScalar[4]; colores[0]=CV_RGB(255,0,0);colores[1]=CV_RGB(0,255,0);colores[2]=CV_RGB(255,255,0);colores[3]=CV_RGB(0,0,255);
  //load the XML of the classifier
     /*Call here the detector!!!*/
  //allocate memory for the resuylt of it
  clasificador classifier;

  classifier.load(classifier_fname);


  //for every object, scan every element in every image and extracts feature vectors for positive and negatives tranining vectors
  for (int obt=0;obt<m_dictionary->getNumClasses()-1;obt++)
  {
    //int ii=Params->sampleFromImages+Params->numTrainImages; //counter of images. start from images not used in the dictionary and in the training part
    int ii=0; //counter of images. start from images not used in the dictionary and in the training part
    //int ii=Params->sampleFromImages; //DELETE THIS!!!! THIS IS ONLY FOR TESTING!!!!
    int total_elements=0; //counter of objects total

    while (total_elements<Params->numTestImages)
    {
      int Nobjects=0;
      Nobjects=DB[obt].GetNumberObjectsinImage(ii);

      for (int elem=0;elem<Nobjects;elem++)
      {
        if (total_elements==Params->numTestImages)
          break;
         //for detector responses
         storage = cvCreateMemStorage(0);
         storage1 = cvCreateMemStorage(0);
         storage2 = cvCreateMemStorage(0);

        det = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( CvRect ), storage );
        score = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( float ), storage1 );
        ids = cvCreateSeq( 0, sizeof( CvSeq ), sizeof( int ), storage2 );

            //
        IplImage *msk0=DB[obt].GetMaskObjectinImage(ii,elem);
        if(NULL==msk0)
        {
            printf("couldn't find the mask for object %d\n", ii);
            break;
        }
        IplImage *img=DB[obt].GetImage(ii);
		assert( img );
        IplImage *msk=DB[obt].GetMaskImage(ii);
        roi=DB[obt].GetROIinImage(ii,elem); 
        IplImage *gray= cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1);
        cvCvtColor(img,gray,CV_RGB2GRAY); //convert to grayscale
        //cvSaveImage("testcolorimage.bmp",img); //save the input image

        // Get tight crop of the centered object to extract patches:
        CvSize tis = Params->trainingImageSize( m_dictionary->dictobjsize );
        IplImage *newimg=DB[obt].Resize( gray, tis, m_dictionary->dictobjsize, LM_ORIGINAL, roi, newroi);
        IplImage *newmsk=DB[obt].Resize( msk, tis, m_dictionary->dictobjsize, LM_ORIGINAL, roi, newroi); 
        ncols=newimg->width;
        nrows=newimg->height;
        IplImage *color= cvCreateImage(cvSize(newimg->width, newimg->height), IPL_DEPTH_8U, 3);
        cvCvtColor(newimg,color,CV_GRAY2RGB);
        
       
        //cvSaveImage("testimage.bmp",newimg); //save the input image
        start_time= 0; end_time= 0; time( &start_time );
        // run the detector
        singleScaleJointDetector(newimg,classifier,Params->TT,codebook,det,score,ids); // HERE CALL THE CLASSIFIER WITH THE IMAGE!!!

        utilidades->print_time(start_time,end_time,"Running time for detection in a 200x200 image: %02d:%02d:%02d\n");  

        // retreive the detections per class and plot them
       
        for (int k=0;k<m_dictionary->getNumClasses()-1;k++)
        {
          int total=0;
          for (int n=0;n<ids->total;n++)
          {
            int* kt = (int*)cvGetSeqElem( ids, n );
            if (*kt==k)
            {
              CvRect* rect = (CvRect*)cvGetSeqElem( det, n );
              float* sco = (float*)cvGetSeqElem( score, n );
              int st=(int)cvCeil(*sco/50); //strength of the detection, normalized to 50
              if (*sco>50.0) // only plot the bounding box if the score is higher than
                  cvRectangle(color, cvPoint(rect->x,rect->y), cvPoint(rect->x+rect->width,rect->y+rect->height), 
                      colores[k],st, 8, 0);
              total=total++;
              
            }
          }
          printf( "Class %d - number of detections = %d \n", k,total );
        }
        utilidades->SaveIMGtest(color,obt, ii,elem);     //saves the image

   
        total_elements++; //increment the total elements used

        cvReleaseImage( &newimg );
        cvReleaseImage( &newmsk );
        cvReleaseImage( &img );
        cvReleaseImage( &msk );
        cvReleaseImage( &msk0 );
        cvReleaseImage( &gray );
        cvReleaseImage( &color );
        cvClearMemStorage( storage );
        cvClearMemStorage( storage1 );
        cvClearMemStorage( storage2 );
    }
       ii++; //increment the number of images used
    }
  }

  delete [] classifier.a;
  delete [] classifier.b;
  delete [] classifier.th;
  delete [] classifier.bestnode;
  delete [] classifier.featureNdx; 
  delete [] colores;

}

