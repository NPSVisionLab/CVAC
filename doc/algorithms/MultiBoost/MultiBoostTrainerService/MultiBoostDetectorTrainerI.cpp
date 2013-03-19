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
#include <MultiBoostDetectorTrainerI.h>
#include <stdexcept>
#include <MultiBoostTrainerApp.h>

///////////////////////////////////////////////////////////////////////////////
MultiBoostDetectorTrainerI::MultiBoostDetectorTrainerI()
: mTrainerApp(NULL)
{

}

///////////////////////////////////////////////////////////////////////////////
MultiBoostDetectorTrainerI::~MultiBoostDetectorTrainerI()
{
   delete mTrainerApp; mTrainerApp = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostDetectorTrainerI::initialize(::Ice::Int, const ::Ice::Current&)
{
   mTrainerApp = new MultiBoostTrainerApp();
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostDetectorTrainerI::process(const ::cvac::TrainerCallbackHandlerPrx&, const ::cvac::RunSet&, const ::Ice::Current&)
{
   mTrainerApp->Run();
}

///////////////////////////////////////////////////////////////////////////////
bool MultiBoostDetectorTrainerI::isInitialized(const ::Ice::Current&)
{
   return (mTrainerApp != NULL);
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostDetectorTrainerI::destroy(const ::Ice::Current& )
{
}

///////////////////////////////////////////////////////////////////////////////
::std::string MultiBoostDetectorTrainerI::getName(const ::Ice::Current&)
{
   throw std::runtime_error("The method or operation is not implemented.");
}

///////////////////////////////////////////////////////////////////////////////
::std::string MultiBoostDetectorTrainerI::getDescription(const ::Ice::Current&)
{
   throw std::runtime_error("The method or operation is not implemented.");
}

///////////////////////////////////////////////////////////////////////////////
void MultiBoostDetectorTrainerI::setVerbosity(::Ice::Int, const ::Ice::Current&)
{
   throw std::runtime_error("The method or operation is not implemented.");
}


///////////////////////////////////////////////////////////////////////////////
namespace nps {
   int g_verbose           = 4;
   FILE* g_ostream         = stderr;
   const char* leveltxt[]  = {"silent", "error", "warn ",
      "info ", "info1", "info2",
      "info3", "info4", "info5"};
   void printv(int level, const char* fmt, ...)
   {
      if (nps::g_verbose >= level)
      {
         va_list args;
         va_start(args, fmt);
         fprintf(nps::g_ostream, "%s: ", leveltxt[level]);
         vfprintf(nps::g_ostream, fmt, args);
         va_end(args);
         fflush(nps::g_ostream);
      }
   }
}; // namespace
