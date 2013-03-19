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
#include "utilities.h"
#include "parameters.h"

#pragma warning( push )
#pragma warning( disable : 4996 )

utilities::utilities( const parameters* p )
: m_params(p)
{
  counter=0;
}

utilities::~utilities(void)
{
}


void utilities::SaveIMG(IplImage *frame)
{

  strcpy(fpath,"");
  strcpy(filen,"");

  ti=(long)time(&long_time);
  i=ti+counter;
  sprintf(filen, "%d", i);
  strcat(filen,".bmp");
  strcat(fpath,m_params->MULTIBOOST_BASEDIR.c_str());
  strcat(fpath, "/Data/pics");

  strcat(fpath,filen); // create a filename based in path 
  cvSaveImage(fpath, frame );
  counter++;
  //and a big number (time in seconds).

}

void utilities::SaveIMGDict(IplImage *frame,char *fname)
{
  strcpy(fpath,"");
  strcat(fpath,m_params->MULTIBOOST_BASEDIR.c_str());
  strcat(fpath,"/Data/dict/");
  strcat(fpath,fname); // create a filename based in path 
  strcat(fpath,".bmp"); // create a filename based in path 
  cvSaveImage(fpath, frame );
}


void utilities::SaveIMGnew(IplImage *frame,int objecto, int imagen,int elemento)
{
  strcpy(filen,"");
  strcpy(fpath,"");
  strcat(fpath,m_params->MULTIBOOST_BASEDIR.c_str());
  strcat(fpath,"/Data/newimages/");
  sprintf(filen, "%d", objecto);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", imagen);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", elemento);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,".bmp"); // create a filename based in path 
  cvSaveImage(fpath, frame );
}

void utilities::SaveIMGtrain(IplImage *frame,int objecto, int imagen,int elemento)
{
  strcpy(filen,"");
  strcpy(fpath,"");
  strcat(fpath,m_params->MULTIBOOST_BASEDIR.c_str());
  strcat(fpath,"/Data/figures_train/");
  sprintf(filen, "%d", objecto);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", imagen);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", elemento);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,".bmp"); // create a filename based in path 
  cvSaveImage(fpath, frame );
}

void utilities::SaveIMGtest(IplImage *frame,int objecto, int imagen,int elemento)
{
  strcpy(filen,"");
  strcpy(fpath,"");
  strcat(fpath,m_params->MULTIBOOST_BASEDIR.c_str());
  strcat(fpath,"/Data/figures_test/");
  sprintf(filen, "%d", objecto);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", imagen);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", elemento);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,".bmp"); // create a filename based in path 
  cvSaveImage(fpath, frame );
}

void utilities::SaveIMGconv(IplImage *frame,int objecto, int imagen,int elemento)
{
  strcpy(filen,"");
  strcpy(fpath,"");
  strcat(fpath,m_params->MULTIBOOST_BASEDIR.c_str());
  strcat(fpath,"/Data/figures_train/conv_");
  sprintf(filen, "%d", objecto);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", imagen);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", elemento);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,".bmp"); // create a filename based in path 
  cvSaveImage(fpath, frame );
}

void utilities::SaveIMGmask(IplImage *frame,int objecto, int imagen,int elemento)
{
  strcpy(filen,"");
  strcpy(fpath,"");
  strcat(fpath,m_params->MULTIBOOST_BASEDIR.c_str());
  strcat(fpath,"/Data/figures_masks/");
  sprintf(filen, "%d", objecto);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", imagen);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,"_"); // create a filename based in path 
  sprintf(filen, "%d", elemento);
  strcat(fpath,filen); // create a filename based in path 
  strcat(fpath,".bmp"); // create a filename based in path 
  cvSaveImage(fpath, frame );
}

IplImage *utilities::DrawPatch(IplImage *frame,CvRect roi)
{
  CvPoint pt2;
  //pt1.x=roi.x;
  //pt1.y=roi.y;
  pt2.x=roi.x+roi.width/2;
  pt2.y=roi.y+roi.height/2;
  cvLine(frame, pt2, pt2,CV_RGB(0,255,0),3,8,0 );
  return (frame);
}

float utilities::Euclidian(CvPoint* a, CvPoint* b)
{
	assert( a && b );
    return(sqrt(pow((float)(a->x - b->x),2)+pow((float)(a->y - b->y),2))); 

}

void utilities::DistanceMatrix(CvSeq* points1, CvSeq* points2, CvMat* distance)
{
  CvPoint* pt1, *pt2;
  for (int n=0;n<points1->total;n++)
  {
    pt1 = (CvPoint*)cvGetSeqElem( points1, n );
   
    for (int m=0;m<points1->total;m++)
    {
      pt2 = (CvPoint*)cvGetSeqElem( points2, m );
      distance->data.fl[n+m*distance->cols]=Euclidian(pt1, pt2);
    }
  }
}

void utilities::print_time(time_t start_time,time_t end_time, char* message)
{
  time( &end_time );
  double total_time = difftime( end_time, start_time );
  int minutes = cvFloor(total_time/60.);
  int seconds = cvRound(total_time - minutes*60);
  int hours = minutes / 60;
  minutes %= 60;
    
  printf( message, hours, minutes, seconds );
}

#pragma warning( pop )
