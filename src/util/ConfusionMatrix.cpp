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
#include <util/ConfusionMatrix.h>

///////////////////////////////////////////////////////////////////////////////
/**
 * Default constructor for a ConfusionMatrix
 */
ConfusionMatrix::ConfusionMatrix() {}
ConfusionMatrix::ConfusionMatrix(RunSet runSet) {
  initialize(runSet);
}
ConfusionMatrix::ConfusionMatrix(RunSet runSet, int expectedPurposeCnt) {
  
  // Reserve data structures to requested capacity
  allPurposes.reserve(expectedPurposeCnt);
  
  // Reserve for the main vector, and for all column vectors
  matrix.reserve(expectedPurposeCnt);
  for (int col = 0; col < (int)matrix.size(); col++) {
    matrix[col].reserve(expectedPurposeCnt);
  }
  
  initialize(runSet);
}

void ConfusionMatrix::initialize(RunSet runSet) {

  // Loop through and insert all purposes in RunSet
  for(vector<PurposedListPtr>::iterator it = runSet.purposedLists.begin(); it != runSet.purposedLists.end(); ++it) {

    Purpose itemPurpose = (*it)._ptr->pur;
    
    // itemPurpose not found, insert
    if(NOT_FOUND == purposeIdx(itemPurpose)) {
      localAndClientMsg(VLogger::DEBUG_1, NULL, "ConfusionMatrix added Purpose with matrix-index: %d\n", (itemPurpose.ptype - 1));
      allPurposes.push_back(itemPurpose);
    }
  }

  localAndClientMsg(VLogger::DEBUG_1, NULL, "Initializing: (%dx%d) confusion matrix with Purposes of RunSet's labelables.\n", allPurposes.size(), allPurposes.size());
  int actualDim    = allPurposes.size(),
      detectedDim = allPurposes.size();

  // Rows for all actual
  for (int i = 0; i < actualDim; i++) {
    vector<int> row; // Create an empty row

    // Zero for all detected
    for (int j = 0; j < detectedDim; j++) {
        row.push_back(0);
    }
    matrix.push_back(row); // Add row
  }
}

void ConfusionMatrix::resizeMatrix(int deltaSize) {

  // Vector allPurposes already includes the new element
  int matrixCurSize = ((int)allPurposes.size() - 1);

  // Insert a zero to each matrix column vector
  for (int i = 0; i < matrixCurSize; i++) {
    matrix[i].push_back(0);
  }

  //Create new colomn vector and fill it with full number of 0s
  vector<int> newCol;
  for(int pushCount = 0; pushCount < (int)allPurposes.size(); pushCount++) {
    newCol.push_back(0);
  }

  // Add column vector to matrix
  matrix.push_back(newCol);
}

// Increment confusionTable at the proper indeces based on actual and detected
void ConfusionMatrix::addResult(Purpose actual, Purpose detectionResult) {

  IntPair indeces = lookupPurposes(actual, detectionResult);
  matrix[indeces.actual][indeces.detected]++;

  int matrixEntry = matrix[indeces.actual][indeces.detected];
  localAndClientMsg(VLogger::DEBUG_1, NULL, "Matrix incremented at [%d][%d], now = %d.\n", 
                          indeces.actual, indeces.detected, matrixEntry);
}

ConfusionMatrix::IntPair ConfusionMatrix::lookupPurposes(Purpose actual, Purpose detectionResult) {

  int newPurposeCount = 0;
  int actualIdx = purposeIdx(actual);
  int detectedIdx = purposeIdx(detectionResult);

  if(NOT_FOUND == actualIdx) {
    newPurposeCount++;
    allPurposes.push_back(actual);
    actualIdx = (allPurposes.size() - 1);  // last element
  }
  if(NOT_FOUND == detectedIdx) {
    newPurposeCount++;
    allPurposes.push_back(detectionResult);
    detectedIdx = (allPurposes.size() - 1);  // last element
  }

  if(newPurposeCount > 0) {  // Match matrix to new size

    localAndClientMsg(VLogger::DEBUG_1, NULL, "Purpose type(s) added, resizing (%dx%d) matrix to: (%dx%d)\n", 
                                                    (allPurposes.size() - newPurposeCount), (allPurposes.size() - newPurposeCount), 
                                                    allPurposes.size(), allPurposes.size());
    resizeMatrix(newPurposeCount);
  }

  IntPair pair;
  pair.actual = actualIdx;
  pair.detected = detectedIdx;

  return(pair);
}


int ConfusionMatrix::purposeIdx(Purpose key) {

  for(vector<Purpose>::iterator recordIt = allPurposes.begin(); recordIt != allPurposes.end(); recordIt++) {

    // Multi-class: must check Id
    if(key.ptype == MULTICLASS) {

      if(key.classID == recordIt->classID) {
        return(std::distance(allPurposes.begin(), recordIt));
      }
    }
    
    // Check enum type
    else if(key.ptype == recordIt->ptype) {
      return(std::distance(allPurposes.begin(), recordIt));
    }
  }

  return(NOT_FOUND);
}

// lookup of matrix value
int ConfusionMatrix::get(Purpose actualKey, Purpose detectedKey) {

  int actualIdx = purposeIdx(actualKey);
  int detectedIdx = purposeIdx(detectedKey);

  // ActualKey not found
  if(NOT_FOUND == actualIdx) {

    if(MULTICLASS == actualKey.ptype) {

      int classId = actualKey.classID;
      localAndClientMsg(VLogger::DEBUG, NULL, "Unknown class in 'actualKey'.  Adding class: %d.\n", classId);
    }
    else { // PurposeType
      localAndClientMsg(VLogger::DEBUG, NULL, "Unknown PurposeType in 'actualKey'.  Adding Purpose of type: %d.\n", (int)actualKey.ptype);
    }
  }

  // DetectedKey not found
  if(NOT_FOUND == detectedIdx) {
    
    if(MULTICLASS == detectedKey.ptype) {

      int classId = detectedKey.classID;
      localAndClientMsg(VLogger::WARN, NULL, "Unknown class in 'detectedKey'.  Adding class: %d.\n", classId);
    }
    else {  // PurposeType
      localAndClientMsg(VLogger::WARN, NULL, "Unknown PurposeType in 'detectedKey'.  Adding Purpose of type: %d.\n", (int)detectedKey.ptype);
    }
  }

  if((NOT_FOUND == actualIdx) ||
     (NOT_FOUND == detectedIdx)) {

      addResult(actualKey, detectedKey);  // Add missing Purpose(s)
  }

  return matrix[actualIdx][detectedIdx];
}
