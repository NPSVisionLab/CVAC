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
#include <UnitTest++.h>
#include <util/FileUtils.h>
#include <util/processRunSet.h>
#include <util/ConfusionMatrix.h>

using namespace Ice;
using namespace cvac;
using namespace UnitTest;
using namespace std;

  vector<Purpose> runSetGroundTruth_p(RunSet &runSet) {

      Purpose positive(POSITIVE, 0);
      vector<Purpose> groundTruth;

      groundTruth.push_back(positive);
      for(unsigned int i = 0; i < groundTruth.size(); i++) {
        addToRunSet(runSet, "","", groundTruth[i]);
      }
      return(groundTruth);
  }
  vector<Purpose> runSetGroundTruth_pp_nn(RunSet &runSet) {

      Purpose positive(POSITIVE, 0), negative(NEGATIVE, 0);
      vector<Purpose> groundTruth;

      groundTruth.push_back(positive);
      groundTruth.push_back(positive);
      groundTruth.push_back(negative);
      groundTruth.push_back(negative);
      for(unsigned int i = 0; i < groundTruth.size(); i++) {
        addToRunSet(runSet, "","", groundTruth[i]);
      }
      return(groundTruth);
  }

  vector<Purpose> runSetGroundTruth_ppp_n(RunSet &runSet) {

      Purpose positive(POSITIVE, 0), negative(NEGATIVE, 0);
      vector<Purpose> groundTruth;

      groundTruth.push_back(positive);
      groundTruth.push_back(positive);
      groundTruth.push_back(positive);
      groundTruth.push_back(negative);
      for(unsigned int i = 0; i < groundTruth.size(); i++) {
        addToRunSet(runSet, "","", groundTruth[i]);
      }
      return(groundTruth);
  }
  vector<Purpose> runSetGroundTruth_multiclass_1_2_3(RunSet &runSet) {

    Purpose pOne(MULTICLASS, 1), pTwo(MULTICLASS, 2), pThree(MULTICLASS, 3);

    vector<Purpose> groundTruth;
    groundTruth.push_back(pOne);
    groundTruth.push_back(pOne);
    groundTruth.push_back(pTwo);
    groundTruth.push_back(pTwo);
    groundTruth.push_back(pThree);
    groundTruth.push_back(pThree);

    // Add multiclass labelables to RunSet
    addToRunSet(runSet, "","", pOne);
    addToRunSet(runSet, "","", pOne);
    addToRunSet(runSet, "","", pTwo);
    addToRunSet(runSet, "","", pTwo);
    addToRunSet(runSet, "","", pThree);
    addToRunSet(runSet, "","", pThree);

    return(groundTruth);
  }

  void addResultsToMatrix(vector<Purpose> runsetPurposes, vector<Purpose> &testResults, ConfusionMatrix &cMatrix) {

    // For all RunSet purposes
    vector<Purpose>::iterator runsetIt = runsetPurposes.begin();
    vector<Purpose>::iterator testResultsIt = testResults.begin();

    for (runsetIt = runsetPurposes.begin(); runsetIt < runsetPurposes.end(); runsetIt++)  // Match result-Purpose with runset-image Purpose
    {
      int runsetIdx = ((int)(*runsetIt).ptype - 1);
      int resultIdx = (((int)(*testResultsIt).ptype) - 1);

      printf("\n");
      printf("Test| adding Result at: [%d][%d]\n", runsetIdx, resultIdx);

      cMatrix.addResult((*runsetIt), (*testResultsIt));
      testResultsIt++;
    }
  }

  TEST(ConfusionMatrix_1by1_onlyP_check_N)
  { printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("Test| ConfusionMatrix_1by1_onlyP_check_N\n");
    RunSet runSet;
    Purpose positive(POSITIVE, 0), negative(NEGATIVE, 0);

    vector<Purpose> groundTruth = runSetGroundTruth_p(runSet);
    ConfusionMatrix cMatrix(runSet);

    vector<Purpose> testResults;
    testResults.push_back(negative);  // Single negative detection against single positive ground truth

    addResultsToMatrix(groundTruth, testResults, cMatrix);
    assert(0 == cMatrix.get(positive, positive));
    assert(1 == cMatrix.get(positive, negative));
    assert(0 == cMatrix.get(negative, positive));
    assert(0 == cMatrix.get(negative, negative));
  }

  TEST(ConfusionMatrix_2by2_noCorrectDetections)
  { printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("Test| ConfusionMatrix_2by2_noCorrectDetections\n");
    RunSet runSet;
    Purpose positive(POSITIVE, 0), negative(NEGATIVE, 0);

    vector<Purpose> groundTruth = runSetGroundTruth_pp_nn(runSet);
    ConfusionMatrix cMatrix(runSet);

    vector<Purpose> testResults;
    testResults.push_back(negative);  // Detections opposite ground truth
    testResults.push_back(negative);
    testResults.push_back(positive);
    testResults.push_back(positive);

    addResultsToMatrix(groundTruth, testResults, cMatrix);
    assert(0 == cMatrix.get(positive, positive));
    assert(2 == cMatrix.get(positive, negative));
    assert(2 == cMatrix.get(negative, positive));
    assert(0 == cMatrix.get(negative, negative));
  }

  TEST(ConfusionMatrix_2by2_oneHitPerMatrixElement)
  { printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("Test| ConfusionMatrix_2by2_oneHitPerMatrixElement\n");
    RunSet runSet;
    Purpose positive(POSITIVE, 0), negative(NEGATIVE, 0);

    vector<Purpose> groundTruth = runSetGroundTruth_pp_nn(runSet);
    ConfusionMatrix cMatrix(runSet);

    vector<Purpose> testResults;
    testResults.push_back(positive);
    testResults.push_back(negative);
    testResults.push_back(positive);
    testResults.push_back(negative);

    addResultsToMatrix(groundTruth, testResults, cMatrix);

    assert(1 == cMatrix.get(positive, positive)); 
    assert(1 == cMatrix.get(positive, negative));
    assert(1 == cMatrix.get(negative, positive));
    assert(1 == cMatrix.get(negative, negative));
  }

  TEST(ConfusionMatrix_2by2_3p_1n)
  { printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("Test| ConfusionMatrix_2by2_3p_1n\n");
    RunSet runSet;
    Purpose positive(POSITIVE, 0), negative(NEGATIVE, 0);

    vector<Purpose> groundTruth = runSetGroundTruth_ppp_n(runSet);
    ConfusionMatrix cMatrix(runSet);

    // 'Detections' for each RunSet labelable
    vector<Purpose> testResults;
    testResults.push_back(positive);  // Test: all detections as expected: (True positive or True negative)
    testResults.push_back(positive);
    testResults.push_back(positive);
    testResults.push_back(positive);

    addResultsToMatrix(groundTruth, testResults, cMatrix);
    assert(3 == cMatrix.get(positive, positive));  // Detected three positive labelables
    assert(0 == cMatrix.get(positive, negative));
    assert(1 == cMatrix.get(negative, positive));  // Detected on the single negative item
    assert(0 == cMatrix.get(negative, negative));
  }

  TEST(ConfusionMatrix_MultiClass_Test)
  { printf("Test| MultiClass_Test_1_2_3\n");
    RunSet runSet;
    Purpose pOne(MULTICLASS, 1), pTwo(MULTICLASS, 2), pThree(MULTICLASS, 3);

    // Get runset ground truth for each multiclass-labelable
    vector<Purpose> groundTruth = runSetGroundTruth_multiclass_1_2_3(runSet);

    // Is the ConfusionMatrix class constructed properly
    ConfusionMatrix cMatrix(runSet);

    // 'Detections' for each multiclass-labelable
    vector<Purpose> testResults;
    testResults.push_back(pOne);
    testResults.push_back(pOne);
    testResults.push_back(pTwo);
    testResults.push_back(pTwo);
    testResults.push_back(pThree);
    testResults.push_back(pThree);

    addResultsToMatrix(groundTruth, testResults, cMatrix);
    assert(2 == cMatrix.get(pOne,   pOne));
    assert(2 == cMatrix.get(pTwo,   pTwo));
    assert(2 == cMatrix.get(pThree, pThree));

    assert(0 == cMatrix.get(pOne,   pTwo));
    assert(0 == cMatrix.get(pOne,   pThree));
    assert(0 == cMatrix.get(pTwo,   pOne));
    assert(0 == cMatrix.get(pTwo,   pThree));
    assert(0 == cMatrix.get(pThree, pOne));
    assert(0 == cMatrix.get(pThree, pTwo));
  }
