#pragma once
/****
 *CVAC Software Disclaimer
 *
 *This software was developed at the Naval Postgraduate School, Monterey, CA,
 *by employees of the Federal Government in the course of their official duties.
 *Pursuant to title 17 Section 105 of the United States Code this software
 *is not subject to copyright protection and is in the public domain. It is 
 *an experimental system.  The Naval Postgraduate School assumes no
 *responsibility whatsoever for its use by other parties, and makes
 *no guarantees, expressed or implied, about its quality, reliability, 
 *or any other characteristic.
 *We would appreciate acknowledgement and a brief notification if the software
 *is used.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above notice,
 *      this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the Naval Postgraduate School, nor the name of
 *      the U.S. Government, nor the names of its contributors may be used
 *      to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****/
/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>
using namespace std;

// Copy of original cvPerformance trainer's header, drawn from OpenCv cvhaartraining
#include "cvhaartrainingCVAC.h"

// File Utils from CVAC to allow Training directory and file-name manipulation
#ifdef WIN32
  #include <util/wdirent.h>
#else
  #include <dirent.h>
#endif

class CvPerfICETrainI;  // Forward definition

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>

typedef struct pos_neg_sampleNum_struct
{
  int numPosImgs, 
      numNegImgs;
} Pos_neg_counts;

#include "CvPerfICETrainI.h"
#include "util/FileUtils.h"
#include "util/processRunSet.h"
using namespace cvac;

class CvPerfTrainer {
public:
	CvPerfTrainer();
	~CvPerfTrainer();

  // Wrapper function CV Library Call
  int prepareTrainingSamples(const char* infoname, const char* vecname, int numSamples, int width, int height);
  Pos_neg_counts createOpenCvSamples_datafiles( const ::cvac::RunSet& runset, int winWidth, int winHeight,
                                                const ::cvac::TrainerCallbackHandlerPrx& callbackHandler,
                                                std::string infoFileName, std::string vecFileName, std::string negFileName);
  
  void trainDataset(const ::cvac::TrainerCallbackHandlerPrx& callback,
                    const char* dirname, const char* vecname, const char* bgfilename,
                    int winwidth, int winheight,
                    int nPosImgs, int nNegImgs,
                    int numTrainStages);

private:

};
