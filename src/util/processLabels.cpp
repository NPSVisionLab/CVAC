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
#include <Data.h>
#include <Services.h>
#include <util/processLabels.h>

using namespace Ice;
using namespace cvac;

//===========================================================================
/**
 * Return the width and height in pixels of the image or zero if not found.
 */

//===========================================================================
/**
 * Return the filenames and rectangles listed in artifacts.  If no artifacts
 * listed then have a single rectangle of the size of the image file.
 */
int cvac::processLabelArtifactsToRects(LabelableList *artifacts, GetImageSizeFunction sfunc, std::vector<RectangleLabels> *result)
{
    int count = 0;
    if (NULL == artifacts)
        return count;
    std::vector<LabelablePtr>::iterator it;
    RectangleLabels rlabels;
    std::string lastFile;
    bool newData = false;
    for (it = artifacts->begin(); it < artifacts->end(); it++)
    {
        LabelablePtr lptr = (*it);
        
        
        Substrate sub = lptr->sub;
        FilePath  filePath = sub.path;
        std::string fname = filePath.directory.relativePath;
        fname += std::string("/");
        fname += filePath.filename;
       
        if (lastFile.size() == 0 || lastFile.compare(fname) != 0)
        { // new file
            if (newData)
                result->push_back(rlabels); // Save the old rlabels
            rlabels.rects = std::vector<BBoxPtr>();
            rlabels.filename = std::string(fname);
            lastFile = fname;
            count++;
            newData = true;
        }
        LabeledLocationPtr locptr =
                 LabeledLocationPtr::dynamicCast(lptr);
        if (locptr.get() != NULL)
        {
            SilhouettePtr sptr = SilhouettePtr::dynamicCast(locptr->loc);
            if (sptr.get() != NULL)
            {
                BBoxPtr lrect = new BBox();
                int minx = -1;
                int miny = -1;
                int maxx = -1;
                int maxy = -1;
                std::vector<Point2DPtr>::iterator itpt;
                for (itpt = sptr->points.begin(); itpt < sptr->points.end(); itpt++)
                {
                    Point2DPtr pptr = (*itpt);
                    if (minx == -1)
                        minx = pptr->x;
                    else if (pptr->x < minx)
                        minx = pptr->x;
                    if (maxx == -1)
                        maxx = pptr->x;
                    else if (pptr->x > maxx)
                        maxx = pptr->x;
                    if (miny == -1)
                        miny = pptr->y;
                    else if (pptr->y < miny)
                        miny = pptr->y;
                    if (maxy == -1)
                        maxy = pptr->y;
                    else if (pptr->y > maxy)
                        maxy = pptr->y;
                }
                lrect->x = minx;
                lrect->y = miny;
                lrect->width = maxx - minx + 1;
                lrect->height = maxy - miny + 1; 
                rlabels.rects.push_back(lrect);  
                continue;
            } 
            
        }
        if (sfunc != NULL) {
            // Assume no location so return with and height of the image
            int w;
            int h;
            // We need to keep the rectangle in sync with the files so if not found return 0 width/height
            BBoxPtr lrect = new BBox();
            lrect->x = 0;
            lrect->y = 0;
            if ((*sfunc)(fname, w, h))
            {
                lrect->width = w;
                lrect->height = h;
   
            } else {
                lrect->width = 0;
                lrect->height = 0;
            }
            rlabels.rects.push_back(lrect);
        }
   
    }
    if (newData)
    { // write out the last record
        result->push_back(rlabels);
    }
    return count;
}

/**
 * Delete all the BBoxPtrs
 */
void cvac::cleanupRectangleLabels(std::vector<RectangleLabels> *rects)
{
    std::vector<RectangleLabels>::iterator it;
    for (it = rects->begin(); it < rects->end(); it++)
    {
       RectangleLabels rlabels = *it;
       std::vector<BBoxPtr>::iterator itpt;
       for (itpt = rlabels.rects.begin(); itpt < rlabels.rects.end(); itpt++)
       {
           BBoxPtr boxptr = *itpt;
           boxptr = 0;  // The smart pointer will delete the object on last reference!
       }
    }
    rects->clear();
}
