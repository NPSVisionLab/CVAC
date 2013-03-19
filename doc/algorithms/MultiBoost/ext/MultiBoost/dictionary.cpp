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
#include "dictionary.h"
#include "utilities.h"
#include "CrossConvolution.h"
#ifdef HAVE_PROTOBUF
#include "trainingData.pb.h"
using namespace trainingData;
#endif // HAVE_PROTOBUF

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

#pragma warning( push )
#pragma warning( disable : 4996 )

dictionary_storage::dictionary_storage()
: patch( NULL )
, locX( NULL )
, locY( NULL )
{}


dictionary::dictionary()
: m_initialized( false )
, CrossOp( NULL )
, utilidades( NULL )
, m_nclasses( -1 )
, Params( NULL )
, dictobjsize( cvSize( 0, 0 ) )
, m_largest_patch_size( -1 )
{
  int aperture_size=7;
  locSigma = cvCreateMat(1,2*aperture_size+1, CV_32F ); //% spatial filtering of the correlation score.

  float values=0;
  for (int i=0;i<2*aperture_size+1;i++)
  { 
    values=(float)exp(-pow((double)(-aperture_size+i),2)/(pow(7.0,2.0)));
    cvSet1D( locSigma, i, cvScalar(values));
  }

  const int nFilters = 4;
  m_filters.resize( nFilters );
  m_filters[0] = cvCreateMat( 3, 3, CV_32FC1 );
  m_filters[1] = cvCreateMat( 3, 3, CV_32FC1 );
  m_filters[2] = cvCreateMat( 3, 3, CV_32FC1 );
  m_filters[3] = cvCreateMat( 1, 1, CV_32FC1 );

  CvMat tmp;
  float a[] = { .5, 1, .5, 0, 0, 0, -.5, -1, -.5 };   // y derivative
  float b[] = { .5, 0, -.5, 1, 0, -1, .5, 0, -.5 };   // x derivative
  float c[] = { -1, -1, -1, -1, 8, -1, -1, -1, -1 };  // laplacian
  float d[] = { 1 };                                  // identity - the image itself
  cvInitMatHeader( &tmp, 3, 3, CV_32FC1, a );
  cvCopy( &tmp, m_filters[0] );
  cvInitMatHeader( &tmp, 3, 3, CV_32FC1, b );
  cvCopy( &tmp, m_filters[1] );
  cvInitMatHeader( &tmp, 3, 3, CV_32FC1, c );
  cvCopy( &tmp, m_filters[2] );
  cvInitMatHeader( &tmp, 1, 1, CV_32FC1, d );
  cvCopy( &tmp, m_filters[3] );
}

void dictionary::initialize( parameters *par )
{
  assert( !m_initialized );
  Params=par;
  assert( Params );
  m_nclasses = Params->nclasses;
  assert( Params->Nfilters == m_filters.size() );
  CrossOp=new CrossConvolution(Params->Nfilters); //start cross convultion object
  utilidades = new utilities( Params );
  m_initialized = true;
}

void *dictionary::icvClearAlloc(int size)
{
    void *ptr = 0;
    if( size > 0 )
    {
        ptr = cvAlloc(size);
        memset(ptr,0,size);
    }
    return ptr;
}

void dictionary::icvFreeMatrixArray(CvMat ***matrArray,int numMatr)
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

dictionary::~dictionary(void)
{
  int Nentries = code_entry.nfilter.size();
  for (int entry=0;entry<Nentries;entry++)
  {
    cvReleaseMat( &code_entry.patch[entry] );
  }
  icvFreeMatrixArray( &code_entry.patch,1 );
  icvFreeMatrixArray( &code_entry.locX, 1 );
  icvFreeMatrixArray( &code_entry.locY ,1 );
  delete utilidades;
}


CvMat *dictionary::DeltaBlurredOffest(int Lx,int xo,float cx)
{
  CvMat *gx, *gxe;
  gx = cvCreateMat(1,Lx, CV_32F ); //% spatial filtering for x axis
  cvSetZero( gx);
  int index=(Lx+1)/2 + (xo - (int)cx)-1; // CORRECTED! it is different from the Matlab version by the + instead of the minus
  cvSetReal1D(gx,index,1);
  //cvFilter2D( gx,gxe,Params->locSigma);

  gxe=CrossConvolution::DFTConvolution(gx, locSigma); //This is slow!!! 1D convolution can be implemented faster using a simple straightforward code.
  float total=(float)cvSum(gxe).val[0];
  cvScale(gxe,gxe,1./total); //Blurred delta function at the relative offset
  cvReleaseMat( &gx );
  return (gxe);
}

void dictionary::LoadDictionary(const std::string& dict_fname)
{
  if (m_initialized)
  {
    nps::printv( 2, "MultiBoost dictionary was initialized already\n" );
    m_initialized = false;
  }
  nps::printv( 4, "loading MultiBoost dictionary from %s\n", dict_fname.c_str() );
  CvFileStorage* fs = cvOpenFileStorage( dict_fname.c_str(), 0, CV_STORAGE_READ );
  if (fs==NULL) throw std::runtime_error(std::string("can not find file ").append(dict_fname).c_str());

  // object size
  {
    dictobjsize.width = cvReadIntByName(fs, NULL, "objsize_width", 0);
    dictobjsize.height = cvReadIntByName(fs, NULL, "objsize_height", 0);
    if (dictobjsize.width==0 || dictobjsize.height==0)
    {
      throw std::runtime_error("dictionary file does not contain object width or height");
    }
    nps::printv( 5, "dictionary is for a %dx%d object\n", dictobjsize.width, dictobjsize.height );
  }

  // determine number of classes
  m_nclasses=0;
  {
    char node_name[255];
    do
    {
      sprintf( node_name, "class_%d", m_nclasses); // create a filename based in path 
      CvFileNode* listNode = cvGetFileNodeByName(fs,0,node_name);
      if (listNode==NULL) 
      {
        break;
      //code_entry.avg_width[clase] = cvReadIntByName(fs,listNode, "averageWidth", 0);
      //code_entry.avg_height[clase] = cvReadIntByName(fs,listNode, "averageHeight", 0);
      }
      m_nclasses++;
    } while (true);
    if ( m_nclasses==0 ) throw std::runtime_error("no classes found in dictionary file");
    nps::printv( 5, "found %d classes in dictionary file\n", m_nclasses );
    m_nclasses++; // account for the background class
  }

  // determine Nentries
  //number of filters * number of patches per image *number of images * number of classes
  //int Nentries=Params->Nfilters*Params->patchesFromExample*Params->sampleFromImages*(Params->nclasses-1); 
  int Nentries=0; 
  {
    char node_name[255];
    do
    {
      sprintf( node_name, "entry_%d", Nentries); // create a filename based in path 
      CvFileNode* listNode = cvGetFileNodeByName(fs,0,node_name);
      if (listNode==NULL) 
      {
        break;
      //code_entry.avg_width[clase] = cvReadIntByName(fs,listNode, "averageWidth", 0);
      //code_entry.avg_height[clase] = cvReadIntByName(fs,listNode, "averageHeight", 0);
      }
      Nentries++;
    } while (true);
    if ( Nentries==0 ) throw std::runtime_error("no entries found in dictionary file");
    nps::printv( 5, "found %d entries in dictionary file\n", Nentries );
  }

  // code_entry.locX= cvCreateMat( 1, 1, CV_32F);
  code_entry.nfilter.resize(Nentries);
  for (unsigned int i=0; i<code_entry.nfilter.size(); i++) code_entry.nfilter[i] = -1;
  code_entry.Lx.resize( Nentries );
  code_entry.Ly.resize( Nentries );
  code_entry.imagendx.resize( Nentries );
  code_entry.elemndx.resize( Nentries );
  code_entry.patch = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nentries);
  code_entry.locX = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nentries);
  code_entry.locY = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nentries);
  code_entry.filters.resize( Nentries ); 
  for (unsigned int i=0; i<code_entry.filters.size(); i++) code_entry.filters[i] = NULL;
  CvMat *patch;
  code_entry.cx.resize( Nentries );
  code_entry.cy.resize( Nentries );
  code_entry.xo.resize( Nentries );
  code_entry.yo.resize( Nentries );
  code_entry.avg_width.resize( m_nclasses );
  code_entry.avg_height.resize( m_nclasses );

  for (int entry=0;entry<Nentries;entry++)
  {
    char node_name[255];
    sprintf(node_name, "entry_%d", entry); // create a filename based in path 

    CvFileNode* ListNode = cvGetFileNodeByName(fs,0,node_name);
    code_entry.nfilter[entry] = cvReadIntByName(fs,ListNode, "filter", 0);
    code_entry.Lx[entry] = cvReadIntByName(fs,ListNode, "Lx", 0);
    code_entry.Ly[entry] = cvReadIntByName(fs,ListNode, "Ly", 0);
    code_entry.imagendx[entry] = cvReadIntByName(fs,ListNode, "imagendx", 0);
    code_entry.elemndx[entry] = cvReadIntByName(fs,ListNode, "elemndx", 0);
    patch=(CvMat*)cvReadByName( fs, ListNode, "patch" );
    code_entry.patch[entry]= cvCreateMat( patch->rows, patch->cols, CV_32F);
    cvCopy(patch,code_entry.patch[entry]);
    code_entry.cx[entry] = cvReadIntByName(fs,ListNode, "cx", 0);
    code_entry.cy[entry] = cvReadIntByName(fs,ListNode, "cy", 0);
    code_entry.xo[entry] = cvReadIntByName(fs,ListNode, "xo", 0);
    code_entry.yo[entry] = cvReadIntByName(fs,ListNode, "yo", 0);

    code_entry.locX[entry] = DeltaBlurredOffest(code_entry.Lx[entry],code_entry.xo[entry],(float)code_entry.cx[entry]);
    code_entry.locY[entry] = DeltaBlurredOffest(code_entry.Ly[entry],code_entry.yo[entry],(float)code_entry.cy[entry]);

    assert( entry<code_entry.nfilter.size() );
    assert( code_entry.nfilter[entry]<m_filters.size() );
    code_entry.filters[entry] = m_filters[code_entry.nfilter[entry]];

    //temp8U= cvCreateImage(cvSize(patch->width, patch->height), IPL_DEPTH_8U, 1);
    //cvConvert(patch,temp8U); 
    //cvSaveImage("patcho.bmp",temp8U);
    //cvReleaseImage( &temp8U );
    //codebook.push_back(code_entry);
    //remember to push the entry into the vector
  }

  for (int clase=0;clase<m_nclasses;clase++)
  {
    char node_name[255];
    sprintf(node_name, "class_%d", clase); // create a filename based in path 
    CvFileNode* ListNode = cvGetFileNodeByName(fs,0,node_name);
    code_entry.avg_width[clase] = cvReadIntByName(fs,ListNode, "averageWidth", 0);
    code_entry.avg_height[clase] = cvReadIntByName(fs,ListNode, "averageHeight", 0);
  }

  cvReleaseFileStorage( &fs ); //before exiting, close file
  nps::printv( 5, "successfully loaded dictionary\n", dict_fname.c_str() );
}

int dictionary::getLargestPatchSize()
{
  if ( m_largest_patch_size>0 )
  {
    return m_largest_patch_size; 
  }
  else if (dictobjsize.width>0 && dictobjsize.height>0 )
  {
    CvMat* patchSize; //size of patches
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
    m_largest_patch_size = cvGetReal1D( patchSize, patch_increments-1 );
    assert( m_largest_patch_size<shorter_len );
    cvReleaseMat( &patchSize );

    return m_largest_patch_size;
  }

  throw std::runtime_error( "can not calculate largest patchsize" );
};






#ifdef HAVE_PROTOBUF
//Protobase function that specifically loads a protobuff class 

void dictionary::LoadDictionary(Data &Dictdata)
{
  if (m_initialized)
  {
    nps::printv( 2, "MultiBoost dictionary was initialized already\n" );
    m_initialized = false;
  }
  nps::printv( 4, "loading MultiBoost dictionary from pb\n");
  //CvFileStorage* fs = cvOpenFileStorage( dict_fname.c_str(), 0, CV_STORAGE_READ );
  //if (fs==NULL) throw std::runtime_error(std::string("can not find file ").append(dict_fname).c_str());
  
  // object size
  {
    dictobjsize.width =Dictdata.objsizew();
    dictobjsize.height = Dictdata.objsizeh();
    if (dictobjsize.width==0 || dictobjsize.height==0)
    {
      throw std::runtime_error("dictionary file does not contain object width or height");
    }
    nps::printv( 5, "dictionary is for a %dx%d object\n", dictobjsize.width, dictobjsize.height );
  }
  
  // determine number of classes
  
  
  m_nclasses=0;
  m_nclasses=Dictdata.avgsize_size();
  if ( m_nclasses==0 ) throw std::runtime_error("no classes found in dictionary file");
  nps::printv( 5, "found %d classes in dictionary file\n", m_nclasses );
  int Nentries=0; 
  
  
  if(Dictdata.dictionary_size()==0)throw std::runtime_error("no entries found in dictionary file");
  
  Nentries=Dictdata.dictionary_size();

  
  code_entry.nfilter.resize(Nentries);
  for (unsigned int i=0; i<code_entry.nfilter.size(); i++) code_entry.nfilter[i] = -1;
  code_entry.Lx.resize( Nentries );
  code_entry.Ly.resize( Nentries );
  code_entry.imagendx.resize( Nentries );
  code_entry.elemndx.resize( Nentries );
  code_entry.patch = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nentries);
  code_entry.locX = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nentries);
  code_entry.locY = (CvMat**)icvClearAlloc(sizeof(CvMat*)*Nentries);
  code_entry.filters.resize( Nentries ); 
  for (unsigned int i=0; i<code_entry.filters.size(); i++) code_entry.filters[i] = NULL;
  CvMat *patch= NULL; 
  code_entry.cx.resize( Nentries );
  code_entry.cy.resize( Nentries );
  code_entry.xo.resize( Nentries );
  code_entry.yo.resize( Nentries );
  code_entry.avg_width.resize( m_nclasses );
  code_entry.avg_height.resize( m_nclasses );
  
  for (int entry=0;entry<Nentries;entry++)
  {
    char node_name[255];
    sprintf(node_name, "entry_%d", entry); // create a filename based in path 
      
    printf("%d out of %d\n",entry, Nentries);
    
    code_entry.nfilter[entry] = Dictdata.dictionary(entry).filterindex()-1;
    
    
    code_entry.patch[entry]= cvCreateMat( Dictdata.dictionary(entry).patchro(), Dictdata.dictionary(entry).patchco(), CV_32F);
   
    
    code_entry.locX[entry] = convertToCvMat(Dictdata.dictionary(entry).location().x().data(), Dictdata.dictionary(entry).location().x_size());
    code_entry.locY[entry] = convertToCvMat(Dictdata.dictionary(entry).location().y().data(), Dictdata.dictionary(entry).location().y_size());
    
    assert( entry<code_entry.nfilter.size() );
    assert( code_entry.nfilter[entry]<m_filters.size() );
    code_entry.filters[entry] = m_filters[code_entry.nfilter[entry]];
    
    //temp8U= cvCreateImage(cvSize(patch->width, patch->height), IPL_DEPTH_8U, 1);
    //cvConvert(patch,temp8U); 
    //cvSaveImage("patcho.bmp",temp8U);
    //cvReleaseImage( &temp8U );
    //codebook.push_back(code_entry);
    //remember to push the entry into the vector
  }
  
  for (int clase=0;clase<m_nclasses;clase++)
  {
    //char node_name[255];
   // sprintf(node_name, "class_%d", clase); // create a filename based in path 
    //CvFileNode* ListNode = cvGetFileNodeByName(fs,0,node_name);
    
    code_entry.avg_width[clase] = Dictdata.avgsize(clase).avgsizew();
    code_entry.avg_height[clase] = Dictdata.avgsize(clase).avgsizeh();
    
  }
  
  //cvReleaseFileStorage( &fs ); //before exiting, close file
  nps::printv( 5, "successfully loaded dictionary\n");
}
#endif // HAVE_PROTOBUF


CvMat *dictionary::convertToCvMat(const float *g, int size)
{
  CvMat *gx; 
  gx = cvCreateMatHeader( size, 1, CV_32F ); //% spatial filtering for x axis
  printf("%d \n",size);
  cvSetData( gx, (float*)g, size -1 );
  
  CvMat *clone = cvCloneMat( gx );
  cvReleaseMatHeader( &gx );
  return clone;
}
#pragma warning( pop )
