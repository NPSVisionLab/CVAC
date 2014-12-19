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

#include <util/OutputResults.h>
#include <util/RunSetWrapper.h>
using namespace cvac;
using namespace Ice;
OutputResults::OutputResults(
                        const cvac::DetectorCallbackHandlerPrx callback,
                        const std::string callbackFreq)
{
    if (callbackFreq.empty())
       mCallbackFreq = "labelable";
    else
        mCallbackFreq = callbackFreq;
    mCallback = callback;
}


void OutputResults::addResult(cvac::Result& _res,
                               cvac::Labelable& _converted,
                               std::vector<cv::Rect> _rects,
                               std::string labName,
                               float confidence)
{   
    int detcount = _rects.size();
    localAndClientMsg(VLogger::DEBUG, NULL, "detections: %d\n", detcount); 

    if(_rects.size()>0)
    {
      if(RunSetWrapper::isVideo(_res.original))
      {
        LabeledTrackPtr newFound = new LabeledTrack();
        newFound->lab.hasLabel = true;
        newFound->confidence = 1.0f;

        for(std::vector<cv::Rect>::iterator it = _rects.begin(); it != _rects.end(); ++it)
        { 
          newFound->lab.hasLabel = true;
          newFound->lab.name = labName;
          newFound->confidence = confidence;

          BBox* box = new BBox();
          box->x = (*it).x;
          box->y = (*it).y;
          box->width = (*it).width;
          box->height = (*it).height;

          FrameLocation floc;
          floc.loc = box;        

          if(!_converted.lab.hasLabel && !_converted.lab.name.empty())  //the sign for frame information
            floc.frame.framecnt = atoi(_converted.lab.name.c_str());  //This info. is frameNumber
          else
          {
            floc.frame.framecnt = -1; //there is no frame info. even though it comes from a video 
            localAndClientMsg(VLogger::WARN, NULL,
              "There is no frame info. even though it comes from a video (%s).\n",
              (RunSetWrapper::getFilename(_res.original)).c_str()); 
          }
          newFound->keyframesLocations.push_back(floc);
        }
        _res.foundLabels.push_back( newFound );
      }
      else
      {
        if (RunSetWrapper::isVideo(_res.original)) //Is this case possible?
        {
          localAndClientMsg(VLogger::WARN, NULL,
            "Though this file (%s) is not an image or a video, somethings are found.\n",
            RunSetWrapper::getFilename(_res.original).c_str());  
        }

        for(std::vector<cv::Rect>::iterator it = _rects.begin(); it != _rects.end(); ++it)
        {
          LabeledLocationPtr newFound = new LabeledLocation();    

          newFound->lab.hasLabel = true;
          newFound->lab.name = labName;
          newFound->confidence = confidence;

          BBox* box = new BBox();
          box->x = (*it).x;
          box->y = (*it).y;
          box->width = (*it).width;
          box->height = (*it).height;
          newFound->loc = box; 

          _res.foundLabels.push_back( newFound );
        }
      }
    }else
    { // we got not hits but still return the label
        LabelablePtr newFound = new Labelable();
        newFound->lab.hasLabel = true;
        newFound->lab.name = labName;
        newFound->confidence = confidence;
        _res.foundLabels.push_back( newFound );
    }
    if (mCallbackFreq.compare("immediate") == 0)
    {
        // localAndClientMsg(VLogger::WARN, mCallback, "callbackFrequency mode 'immediate' not supported!\n"); 
        ResultSet resSet;
        resSet.results.push_back(_res);
        mCallback->foundNewResults(resSet);
        // We don't want to send the results multiple times so get rid
        // of results we have aready sent.  We don't have to do this in the
        // labelable case since its a different result but in the case of video it 
        // can be the same result getting added to.
        // TODO have a result sent flag so we can keep the result but know not to send it.
        _res.foundLabels.pop_back();
    }else if (mCallbackFreq.compare("labelable") == 0)
    {
        string orig = RunSetWrapper::getFilename(_res.original);
        if (mLastFile.empty())
        {
            mLastFile = orig;
        }else
        {
            if (mLastFile.compare(orig) != 0)
            { // Filename has changed so output previous results.
                ResultSet resSet;
                resSet.results.push_back( *mPrevResult );
                mCallback->foundNewResults(resSet);
                mLastFile = orig;
            }
        }
        mPrevResult = &_res;
    }
  
}

void OutputResults::finishedResults(cvac::RunSetIterator &runSetIter)
{
    if (mCallbackFreq.compare("labelable") == 0)
    { // Send out the final results from the current result set only
        ResultSet resSet;
        resSet.results.push_back(runSetIter.getCurrentResult());
        mCallback->foundNewResults(resSet); 
    }else if (mCallbackFreq.compare("final") == 0)
    { // We are finally done so sent out all results at once.
        mCallback->foundNewResults(runSetIter.getResultSet()); 
    }
    mLastFile = ""; // Clear for next time
    mPrevResult = NULL;
}
