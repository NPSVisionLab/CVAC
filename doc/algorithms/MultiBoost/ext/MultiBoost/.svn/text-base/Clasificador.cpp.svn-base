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
#ifdef HAVE_PROTOBUF
#include "trainingData.pb.h"
using namespace trainingData;
#endif // HAVE_PROTOBUF

clasificador::clasificador()
 : nWeakClassifiers( 0 )
 , objsize( cvSize( 0, 0 ) )
 , featureNdx( NULL )
 , th( NULL )
 , a( NULL)
 , b( NULL )
 , bestnode( NULL )
{
}

//saves the detector in XML file
void clasificador::save(const std::string& classifier_fname)
{
  char node_name[255];
  
  CvFileStorage* fs = cvOpenFileStorage( classifier_fname.c_str(), 0, CV_STORAGE_WRITE );
  if ( !fs ) 
  {
    throw std::runtime_error( std::string("can not write to %s").append( classifier_fname ).c_str() );
  }
  assert( objsize.width>0 && objsize.height>0 );
  cvWriteInt( fs, "objsize_width", objsize.width);
  cvWriteInt( fs, "objsize_height", objsize.height);

  for (int i=0;i<nWeakClassifiers;i++)
  {   
    //SAVE ENTRIES IN DICTIONARY              
    sprintf(node_name, "round_%d", i); // create a filename based in path 
    //utilidades.SaveIMGDict(temp8U,node_name); //save patch
    cvStartWriteStruct(fs,node_name,CV_NODE_MAP);
    cvWriteReal( fs, "a", a[i] ); //save the 'a' coefficient of the regression boost
    cvWriteReal( fs, "b", b[i] ); //save the 'b' coefficient of the regression boost
    cvWriteReal( fs, "th", th[i] ); //save the 'threshold' of the regression boost
    cvWriteInt( fs, "bestnode", bestnode[i] ); //save the best node of the topology tree T
    cvWriteInt( fs, "featureNdx", featureNdx[i] ); //save the feature_id, which is the entry in the dictionary
    cvEndWriteStruct(fs);
  }
  cvReleaseFileStorage( &fs ); //before exiting, close file
}


//loads the XML detector file
void clasificador::load(const std::string& classifier_fname)
{
  //number of filters * number of patches per image *number of images * number of classes
  //CvMat *patch;
  //IplImage* temp8U;
  char node_name[255];

  nps::printv( 4, "loading MultiBoost classifier/detector from %s\n", classifier_fname.c_str() );
  CvFileStorage* fs = cvOpenFileStorage( classifier_fname.c_str(), 0, CV_STORAGE_READ );
  if ( !fs ) 
  {
    throw std::runtime_error( std::string("can not read from %s").append( classifier_fname ).c_str() );
  }

  // object size
  {
    objsize.width = cvReadIntByName(fs, NULL, "objsize_width", 0);
    objsize.height = cvReadIntByName(fs, NULL, "objsize_height", 0);
    if (objsize.width==0 || objsize.height==0)
    {
      throw std::runtime_error("detector file does not contain object width or height");
    }
  }

  // how many weak classifiers?
  int numWeak=0;
  sprintf(node_name, "round_%d", numWeak); // create a filename based in path 
  CvFileNode* ln = cvGetFileNodeByName(fs,0,node_name);
  while (ln)
  {
    numWeak++;
    sprintf(node_name, "round_%d", numWeak); // create a filename based in path 
    ln = cvGetFileNodeByName(fs,0,node_name);
  }
  nWeakClassifiers = numWeak;

  //allocate memory for the classifier
  a = new float[numWeak];
  b = new float[numWeak];
  th = new float[numWeak];
  featureNdx = new int[numWeak];
  bestnode = new int[numWeak];

  for (int entry=0; entry<nWeakClassifiers; entry++)
  {
    sprintf(node_name, "round_%d", entry); // create a filename based in path 
    CvFileNode* ListNode = cvGetFileNodeByName(fs,0,node_name);
    a[entry] = cvReadRealByName(fs,ListNode, "a", 0);
    b[entry] = cvReadRealByName(fs,ListNode, "b", 0);
    th[entry] = cvReadRealByName(fs,ListNode, "th", 0);
    bestnode[entry]= cvReadIntByName(fs,ListNode, "bestnode", 0);
    featureNdx[entry]= cvReadIntByName(fs,ListNode, "featureNdx", 0);
  }
  cvReleaseFileStorage( &fs ); //before exiting, close file
  nps::printv( 5, "loaded %dx%d detector with %d weak classifiers\n", 
        objsize.width, objsize.height, nWeakClassifiers );
}

#ifdef HAVE_PROTOBUF
//loads the pb detector file
void clasificador::load(const trainingData::Detector &detect)
{
 
  
  // object size
  
    objsize.width = detect.objsizew();
    objsize.height = detect.objsizeh();
    if (objsize.width==0 || objsize.height==0)
    {
      throw std::runtime_error("detector file does not contain object width or height");
    }

  
  // how many weak classifiers?
  int numWeak=0;
  
  numWeak=detect.detectentry_size();
  
  
  nWeakClassifiers = numWeak;
  
  //allocate memory for the classifier
  a = new float[numWeak];
  b = new float[numWeak];
  th = new float[numWeak];
  featureNdx = new int[numWeak];
  bestnode = new int[numWeak];
  
  for (int entry=0; entry<nWeakClassifiers; entry++)
  {
    
    a[entry] = detect.detectentry(nWeakClassifiers).a();
    b[entry] = detect.detectentry(nWeakClassifiers).b();
    th[entry] = detect.detectentry(nWeakClassifiers).th();
    bestnode[entry]= detect.detectentry(nWeakClassifiers).bestnode();
    featureNdx[entry]= detect.detectentry(nWeakClassifiers).featurendx();;
  }

  nps::printv( 5, "loaded %dx%d detector with %d weak classifiers\n", 
              objsize.width, objsize.height, nWeakClassifiers );
}
#endif // HAVE_PROTOBUF

