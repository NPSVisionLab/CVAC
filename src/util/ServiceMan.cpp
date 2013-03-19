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
#include <util/ServiceMan.h>
#include <util/Timing.h>

#include <map>

typedef std::map<std::string, int> ServiceMapType;
/* int values 1 = requested stop, 2 = stop complete  */
static ServiceMapType *stopServices = NULL;

void cvac::stopService(std::string service)
{
    if (stopServices == NULL)
    {
        stopServices = new ServiceMapType();
    }
    ServiceMapType::iterator it = stopServices->find(service);
    if (it != stopServices->end())
    {
        it->second = 1;
    }
}

void cvac::runningStoppableService(std::string service)
{
    if (stopServices == NULL)
    {
        stopServices = new ServiceMapType();
    }
    ServiceMapType::iterator it = stopServices->find(service);
    if (it == stopServices->end())
    {
        stopServices->insert(make_pair(service, 0));
    }
}

void cvac::waitForStopService(std::string service)
{
    cvac::stopService(service);
    while (cvac::isStopCompleted(service) == false)
    {
        cvac::sleep(100);
    }
}

bool cvac::stopRequested(std::string service)
{
    if (stopServices == NULL)
    {
        return false;
    }
    ServiceMapType::iterator it = stopServices->find(service);
    if (it != stopServices->end())
    {
        if (it->second == 1 || it->second == 2)
            return true;
    }
    return false;
}

void cvac::clearStop(std::string service)
{
    if (stopServices == NULL)
    {
        return;
    }
    ServiceMapType::iterator it = stopServices->find(service);
    if (it != stopServices->end())
    {
        stopServices->erase(it);
    }
}

void cvac::stopCompleted(std::string service)
{
    if (stopServices == NULL)
    {
        return;
    }
    ServiceMapType::iterator it = stopServices->find(service);
    if (it != stopServices->end())
    {
        it->second = 2;
    }
}

bool cvac::isStopCompleted(std::string service)
{
    if (stopServices == NULL)
    {
        return true;
    }
    ServiceMapType::iterator it = stopServices->find(service);
    if (it != stopServices->end())
    {
        if (it->second == 2)
            return true;
        else
            return false;
    }
    return true; // service not running
}

static std::string curServiceName;

void cvac::setServiceName(std::string name)
{
    curServiceName = name;
}

std::string cvac::getServiceName()
{
    return curServiceName;
}

