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
#include <MultiBoost/MultiBoostTrainerApp.h>
#include <MultiBoost/parameters.h>
#include <MultiBoost/dictionary.h>
#include <MultiBoost/ComputeFeatures.h>
#include <MultiBoost/trainDetector.h>
#include <MultiBoost/utilities.h>
#include <MultiBoost/fileManager.h>
#include <sys/stat.h>
///////////////////////////////////////////////////////////////////////////////
MultiBoostTrainerApp::MultiBoostTrainerApp()
{

}

///////////////////////////////////////////////////////////////////////////////
MultiBoostTrainerApp::~MultiBoostTrainerApp()
{

}

///////////////////////////////////////////////////////////////////////////////
int MultiBoostTrainerApp::Run()
{
   // _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
   // cvSetMemoryManager( (CvAllocFunc) malloc, (CvFreeFunc) free );

   parameters Params;  //Object of parameters

   //jointBoost JBoost;
   //JBoost.CreateArtificialData();

   dictionary Dict; //Dictionary object
   ComputeFeatures ExtractFeatures( &Params ); //Compute Features object
   trainDetector trainDet( &Params ); //train detector class  

   time_t start_time,end_time; //for time performance
   int key=-1;
   utilities utilidades( &Params ); //call class of utilies functions

   fileManager *DB; 
   DB=new fileManager[Params.nclasses-1]; //creates 4 objects of Databse

   for (int i=0;i<Params.nclasses-1;i++) //only 4 objects
   {
      DB[i].CreateTrainingSamplesFromInfo(Params.objects_name.at(i).c_str(), Params.flnMasks);  //read the DB and loads images filenames
      if (DB[i].GetNumberofImages() < Params.sampleFromImages+Params.numTrainImages)
      {
         printf("not enough training examples in %s (got %d, need %d for patches, %d for training)\n",
            Params.objects_name[i].c_str(), DB[i].GetNumberofImages(), Params.sampleFromImages, Params.numTrainImages );
         return -1;
      } else 
      {
         printf("found %d training examples in %s\n", DB[i].GetNumberofImages(),
            Params.objects_name[i].c_str() );
      }
   }

   struct stat finfo;
   if ( stat( Params.GetDictionaryPath().c_str(), &finfo )==0 )
   {
      printf("dictionary file %s exists, won't re-build\n", Params.GetDictionaryPath().c_str());
   }
   else
   {
      start_time= 0; end_time= 0; time( &start_time );
      Dict.initialize( &Params );
      Dict.BuildDictionary(DB); // Builds the dictionary
      utilidades.print_time(start_time,end_time,"Running time to build dictionary: %02d:%02d:%02d\n");
   }

   start_time= 0; end_time= 0; time( &start_time );
   Dict.LoadDictionary( Params.GetDictionaryPath() );  //loads the dictionary
   utilidades.print_time(start_time,end_time,"Running time to load dictionary: %02d:%02d:%02d\n");

   if ( stat( Params.GetFeaturesPath().c_str(), &finfo )==0 )
   {
      printf("features file %s exists, won't re-build\n", Params.GetFeaturesPath().c_str());
   }
   else
   {
      //calculate the feature vectors, positives and negatives, and saves the data
      ExtractFeatures.CalculateFeatures(Dict,DB, Params.GetFeaturesPath() ); 
   }

   //_CrtDumpMemoryLeaks();

   trainDet.train( Params.GetFeaturesPath(), Params.GetDetectorPath() );

   // trainDet.test(Params.GetFeaturesPath(), Params.GetDetectorPath().c_str());

   delete [] DB;
}

