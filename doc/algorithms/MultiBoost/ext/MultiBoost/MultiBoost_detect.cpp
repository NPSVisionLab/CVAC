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
// MultiBoost.cpp : Defines the entry point for the console application.
//
#include "StdAfx.h"
#include "parameters.h"
#include "fileManager.h"
#include "ComputeFeatures.h"
#include "jointBoost.h"
#include "trainDetector.h"
#include "testDetector.h"
#include "MultiBoost.h"
#include "utilities.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <cstdlib> 
#include <stdarg.h>
#include <algorithm>


namespace nps {
int g_verbose           = 4;      
FILE* g_ostream         = stderr; 
const char* leveltxt[]  = {"silent", "error", "warn ",
                                "info ", "info1", "info2",
                                "info3", "info4", "info5"};
void printv(int level, const char* fmt, ...)
{
  if (nps::g_verbose >= level) 
  {
    va_list args;
    va_start(args, fmt);
    fprintf(nps::g_ostream, "%s: ", leveltxt[level]);
    vfprintf(nps::g_ostream, fmt, args);
    va_end(args);
    fflush(nps::g_ostream);
  }
}
}; // namespace

int TestNewInterface(char * line, FILE * dataOut, double start_scale, double stop_scale, double scale_inc_factor );
int ReadingText(char* argv, double start_scale, double stop_scale, double scale_inc_factor );
//int FindHighscoreClass( const std::vector<MultiBoostMatch>& matches, IplImage* out_img);
bool order(const MultiBoostMatch& a, const MultiBoostMatch& b);

int main(int argc, char** argv)
{
  if (argc>1)
  {
    return ReadingText(argv[1], atof(argv[2]), atof(argv[3]), atof(argv[4]));
  }
  else
  {
  int const nClasses=5; //number of classes to recognize including background
  parameters Params( nClasses );

  //jointBoost JBoost;
  //JBoost.CreateArtificialData();

  time_t start_time,end_time; time( &start_time );//for time performance
  int key=-1;
  utilities utilidades( &Params ); //call class of utilies functions
 
  testDetector testDect; //class that runs the detector
  testDect.Params = &Params;
  testDect.utilidades = &utilidades;

  fileManager *DB; 
  DB=new fileManager[nClasses-1]; //creates 4 objects of Databse
  
  for (int i=0;i<nClasses-1;i++) //only 4 objects
  {
    //read the DB and loads images filenames
    DB[i].CreateTrainingSamplesFromInfo(Params.objects_name.at(i).c_str(), 
      Params.flnMasks);
  }

  testDect.ComputeDetectionsInDB( Params.GetDictionaryPath(), nClasses,
    DB,Params.GetDetectorPath());  //runs the detector over the test images
  time( &end_time );
  utilidades.print_time(start_time,end_time,"Running time for entire testing: %02d:%02d:%02d\n");     
  
  delete [] DB;
  }
	return 0;
}

int TestNewInterface(char * line, FILE * dataOut, double start_scale, double stop_scale, double scale_inc_factor )
{
  try {
    IplImage *img, *out_img;
    img = cvLoadImage(line,0); // load as grayscale
    if (!img) { printf("can't load %s\n", line ); return -1; }
    out_img = cvCloneImage( img );

    MultiBoost* mboost = MultiBoost::CreateMultiBoost();
    //parameters params;
    mboost->Init( "trainingData.pb", "detector.pb" );
    //mboost->Init( "../MultiBoostData/Data/marine8_dictionary_25x50.xml", "../MultiBoostData/Data/marine8_detector_25x50.xml" );
    //mboost->Init( "head_dictionary_25x25.xml", "head_detector_25x25.xml" );
    
    std::vector<MultiBoostMatch> matches;
    mboost->Detect( img, out_img, matches, start_scale, stop_scale, scale_inc_factor);
    //mboost->Detect(img, out_img, matches);
      
    cvNamedWindow( "MultiBoost detections" );
    cvShowImage( "MultiBoost detections", out_img );
    printf( "found %d detections\n", matches.size() );

    std::sort(matches.begin(),matches.end(),order);
    
    fprintf(dataOut,"%d\n", matches.size());
    
    for(int i=0; i<matches.size();i++)
    {
    
      fprintf(dataOut,"%d %d %d %d\n", matches[i].m_x, matches[i].m_y, matches[i].m_w, matches[i].m_h);
      
    }
    cvWaitKey( 0 );
    
    mboost->UnInit();
    cvReleaseImage( &out_img );
    return 0;
  }
  catch ( const std::exception &ex )
  {
    printf( "caught exception: %s\n", ex.what() );
    return -1;
  }
}
  
int ReadingText(char* arg, double start_scale, double stop_scale, double scale_inc_factor )
{
  int rnum;
  
  FILE * dataIn;
  FILE * dataOut;
  
  char * fileOut;
  char * fileIn;
  
  
    fileIn=arg;
      
  dataOut=fopen("imgDetections.txt", "w");
  dataIn=fopen(fileIn,"r");
  
  if(!dataIn)
  {
    printf("File coud not be opened, Make sure you have the correct permissions or file path");
    exit(1);}
  
  char line[1024];
  int num;
  
  while(!feof(dataIn))
  {
    num=fscanf(dataIn,"%s",line);
    if(num==1)
      rnum=TestNewInterface(line, dataOut, start_scale, stop_scale, scale_inc_factor );
  } 
  fclose(dataOut);
  fclose(dataIn);
  
  return rnum;}

bool order (const MultiBoostMatch& a, const MultiBoostMatch& b)
{
  if(a.m_score==b.m_score)
  {
    return a.m_score <  b.m_score;
  }
  
  return a.m_score > b.m_score;
  
}


