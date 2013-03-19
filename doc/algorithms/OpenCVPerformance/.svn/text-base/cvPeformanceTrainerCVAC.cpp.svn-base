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
#include "CvPerfICETrainI.h"
#include "cvPerformanceTrainerCVAC.h"
#include <util/processLabels.h>
#include <opencv2/opencv.hpp>

CvPerfTrainer::CvPerfTrainer(){}
CvPerfTrainer::~CvPerfTrainer() {}

/*
  'prepareTrainingSamples(infoname, vecname, num, width, height)'

      OpenCv code to create training samples from images collection.  Used in cvPerformance training.
      char* infoname: Database file name with marked up image descriptions of positive source images.
      char* vecname: Output (.vec) file containing the generated positive samples for training.
  return the number of samples (not images but number of rectangles in images);
*/
int CvPerfTrainer::prepareTrainingSamples(const char* infoname, const char* vecname, int numSamples, int width = 24, int height = 24) {
  int numSam;
  int showsamples = 0;  // no user-visualization
  //cvCreateTrainingSamplesFromInfo(infoname, vecname, numSamples, showsamples, width, height);
  // changed create samples to compute number of samples if we passed zero in.
  // number of samples must include not just images but rectangles.
  numSam = cvCreateTrainingSamplesFromInfo(infoname, vecname, 0, showsamples, width, height);
  //debug
  //cvShowVecSamples( vecname, width, height, 1.0);
  return numSam;   
}

void CvPerfTrainer::trainDataset(const ::cvac::TrainerCallbackHandlerPrx& callback,
                                 const char* dirname,
                                 const char* vecname,
                                 const char* bgfilename,
                                 int winwidth, int winheight,
                                 int nPosImgs, int nNegImgs,
                                 int numTrainStages = 14     )  // Default to a modest number
{
  char* nullname = (char*)"(NULL)";

  // Constants from ViolaJones.properties
  int mode = 2; //"ALL"
  bool bg_vecfile = false;
  int mem = 512;
  float minhitrate     = 0.95f; //0.995F; lowering standards for testing
  float maxfalsealarm  = 0.5F;
  float weightfraction = 0.95F;

  // from original 'haartraining.cpp' wrapper where omitteed from ViolaJones
  int boosttype = 3;
  int equalweights = 0;
  int nsplits = 1;
  int isSymmetric  = 0; // non-symmetric
  int stumperror = 0;
  int maxtreesplits = 0;
  int minpos = nPosImgs;
  const char* boosttypes[] = { "DAB", "RAB", "LB", "GAB" };
  const char* stumperrors[] = { "misclass", "gini", "entropy" };


  printf("Launching training with 'cvCreateTreeCascadeClassifier(..' \n");
  printf( "Data dir name: %s\n", ((dirname == NULL) ? nullname : dirname ) );
  printf( "Vec file name: %s\n", ((vecname == NULL) ? nullname : vecname ) );
  printf( "BG  file name: %s, is a vecfile: %s\n", ((bgfilename == NULL) ? nullname : bgfilename ), bg_vecfile ? "yes" : "no" );
  printf("\n Calling Library \n\n");
  cvCreateTreeCascadeClassifier( dirname, vecname, bgfilename,
                               nPosImgs, nNegImgs, numTrainStages, mem,
                               nsplits,
                               minhitrate, maxfalsealarm, weightfraction,
                               mode, isSymmetric,  //mode: (2 == "ALL"), isSymmetric: (non-symmetric == 0)
                               equalweights, winwidth, winheight,
                               boosttype, stumperror,
                               maxtreesplits, minpos, bg_vecfile );
}

bool static getImageWidthHeight(std::string filename, int &width, int &height)
{
   IplImage* img = cvLoadImage(filename.c_str());
   bool res;
   if( !img )
   {
       width = 0;
       height = 0;
       res = false;
   } else
   {
       width = img->width;
       height = img->height;
       res = true;
   }
   cvReleaseImage( &img );
   return res;
}
  /**
   * Writes data file with one line per sample image in the format
   * fullPathFilename 'num_area_count' x1 y1 w1 h1 ...;
   * dat file can't handle spaces in file names, so those are skipped or
   * removed or sym-linked (check the code to see which one)
   * Errors are logged.
   *
   * @param runset
   */
Pos_neg_counts CvPerfTrainer::createOpenCvSamples_datafiles(const ::cvac::RunSet& runset, int winWidth, int winHeight,
                                                                const ::cvac::TrainerCallbackHandlerPrx& callbackHandler,
                                                                std::string infoFileName, std::string vecFileName, std::string negFileName) 
{
  
  Pos_neg_counts resultCounts = {};
  std::vector<RectangleLabels> posRectlabels;
  std::vector<RectangleLabels> negRectlabels;
  for (size_t i = 0; i < runset.purposedLists.size(); i++)
  {
    // Attempt to warn on Unexpected-Purpose
    if((POSITIVE != runset.purposedLists[i]->pur.ptype) && (NEGATIVE != runset.purposedLists[i]->pur.ptype)) {
      std::cerr << "Unexpected Purpose-Type found in RunSet.  Expecting 'POS' or 'NEG', found: " << runset.purposedLists[i]->pur.ptype << std::endl;
      callbackHandler->message(0, "Unexpected Purpose-Type found in RunSet.  Expecting 'POS' or 'NEG'. \n");
      return resultCounts;
    }
    
    // Store training-input data to vectors
    cvac::PurposedLabelableSeq* lab = static_cast<cvac::PurposedLabelableSeq*>(runset.purposedLists[i].get());
    if (POSITIVE == runset.purposedLists[i]->pur.ptype)
        resultCounts.numPosImgs = cvac::processLabelArtifactsToRects(&lab->labeledArtifacts, getImageWidthHeight, &posRectlabels);
    else
        resultCounts.numNegImgs = cvac::processLabelArtifactsToRects(&lab->labeledArtifacts, NULL, &negRectlabels);
  }

  ofstream infoFile;
 
  infoFile.open(infoFileName.c_str());

  // Save stored data from RunSet to OpenCv positive samples .dat file
  int cnt = 0;
  std::vector<cvac::RectangleLabels>::iterator it;
  for (it = posRectlabels.begin(); it < posRectlabels.end(); it++)
  {
    cvac::RectangleLabels recLabel = *it;
    if (recLabel.rects.size() <= 0)
    { // No rectangle so use the whole image
        int w, h;
        getImageWidthHeight(recLabel.filename, w, h);
        infoFile << recLabel.filename << " 1 0 0 " << w << " " << h;
    }else 
    {
       int rectCnt = 0;  // Only add labels that are as large as the window size we are using!
       std::vector<cvac::BBoxPtr>::iterator rit;
       // Get count of valid size rectangles.
       for (rit = recLabel.rects.begin(); rit < recLabel.rects.end(); rit++)
       {
           cvac::BBoxPtr rect = *rit;
           if (rect->width < winWidth || rect->height < winHeight)
               continue;  // dont' count this rect.
           rectCnt++;
       }
       if (rectCnt == 0)
       {
           int w, h;
           getImageWidthHeight(recLabel.filename, w, h);
           infoFile << recLabel.filename << " 1 0 0 " << w << " " << h;
       } else
       {
          // fileName, # of objects, x, y, width, height
          infoFile << recLabel.filename << " " <<
                      rectCnt << " ";

          for (rit = recLabel.rects.begin(); rit < recLabel.rects.end(); rit++)
          {
              cvac::BBoxPtr rect = *rit;
              if (rect->width < winWidth || rect->height < winHeight)
                   continue;  // dont' count this rect.
              infoFile << rect->x << " " << rect->y << " " << rect->width <<
                          " " << rect->height  << " ";
          }
       }
    }
    cnt++;
    // NO EXTRA BLANK LINE after the last sample, or cvhaartraining.cpp can fail on: "CV_Assert(elements_read == 1);"
    if ((cnt) < resultCounts.numPosImgs)
        infoFile << endl;
  }
  infoFile.flush();
  infoFile.close();
  // Clean up any memory 
  cvac::cleanupRectangleLabels(&posRectlabels);
  // Save stored data from RunSet to OpenCv negative samples file

  ofstream backgroundFile;
  backgroundFile.open(negFileName.c_str());
  cnt = 0;
  for (it = negRectlabels.begin(); it < negRectlabels.end(); it++)
  {
    // fileName, # of objects, x, y, width, height
    cvac::RectangleLabels recLabel = *it;
    backgroundFile << recLabel.filename;
    cnt++;
    // NO EXTRA BLANK LINE after the last sample, or cvhaartraining.cpp can fail on: "CV_Assert(elements_read == 1);"
    if ((cnt) < resultCounts.numNegImgs)
        backgroundFile << endl;
  }
  backgroundFile.flush();
  backgroundFile.close();
  // Clean up any memory 
  cvac::cleanupRectangleLabels(&negRectlabels);
  return(resultCounts);
}

