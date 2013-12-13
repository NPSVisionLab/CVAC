#ifndef __OUTPUTRESULT_H__
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
#define __OUTPUTRESULT_H__

#include <Data.h>
#include <Services.h>
#include <string>
#include <vector>
#include <util/RunSetIterator.h>
#include <cv.h>

/**
 * The OutputResults class sends results to the client based upon how the
 * client has specified it wants to recieve these results.
 */
namespace cvac
{
    class OutputResults
    {
    public:
        OutputResults(const cvac::DetectorCallbackHandlerPrx callback,
                      const std::string callbackFreq);

        /**
         * Add these rectangles and labelable to the results and
         * output to client if its the correct time based on callbackFeq
         */
        void addResult(cvac::Result &curRes,
                  cvac::Labelable &labelable,
                  std::vector<cv::Rect> rects,
                  std::string labName, float confidence);

        /**
         * Tell OutputResults the results are complete.  The client
         * will be called with any final results as per callbackFeq.
         */
        void finishedResults(cvac::RunSetIterator& runSetIter);

    private:
        std::string mLastFile;
        std::string mCallbackFreq;
        cvac::Result *mPrevResult;
        cvac::DetectorCallbackHandlerPrx mCallback;
    };

}

                  
#endif // __OUTPUTRESULT_H__
