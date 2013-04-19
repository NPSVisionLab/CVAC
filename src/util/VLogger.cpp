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
#include <util/FileUtils.h>
#include <util/VLogger.h>
using namespace cvac;

VLogger::VLogger(Levels baseLevel) {

  this->baseLevel = baseLevel;
  setUpMsgStrings();
}

VLogger::VLogger() {

  this->baseLevel = DEBUG_1;  // Give a chance for local msgs before sets
  setUpMsgStrings();
}

// All services read from config.client function and call here to ensure
// that at least one of them sets the verbosity level
void VLogger::setLocalVerbosityLevel(Levels localLevel) {

  baseLevel = localLevel;
  localAndClientMsg(VLogger::DEBUG_1, NULL, "Set local verbosity to: %d\n", (int)baseLevel);
}

void VLogger::setLocalVerbosityLevel(std::string verbosityStr) {

  if(verbosityStr.empty()) {
    localAndClientMsg(VLogger::WARN, NULL, "No verbosity level specified\n");
  } else {
    for (unsigned int lv=0; lv<levelText->size(); lv++)
      {
	if (strcasecmp(levelText->at(lv).c_str(), verbosityStr.c_str())==0) {
	  setLocalVerbosityLevel((VLogger::Levels)lv);
	  return;
	}
      }
    int servicesVerbosity = atoi(verbosityStr.c_str());
    setLocalVerbosityLevel((VLogger::Levels)servicesVerbosity);
  }
}

VLogger::Levels VLogger::getBaseLevel() {
  return(baseLevel);
}

VLogger::Levels VLogger::getIntLevel(int intLevel) {

  switch(intLevel) {
    case 0: return SILENT;
      break;
    case 1: return ERROR_V;
      break;
    case 2: return WARN;
      break;
    case 3: return INFO;
      break;
    case 4: return DEBUG;
      break;
    case 5: return DEBUG_1;
      break;
    case 6: return DEBUG_2;
      break;
    case 7: return DEBUG_3;

    default:
      localAndClientMsg(VLogger::ERROR_V, NULL, "Unable to map integer level: %d to VLogger::Levels.\n", intLevel);
      return(DEBUG_3);
  }
}

// Print-prefixes for message levels
void VLogger::setUpMsgStrings() {

  levelText = new vector<string>();
  levelText->push_back("silent");
  levelText->push_back("error");
  levelText->push_back("warn");
  levelText->push_back("info");
  levelText->push_back("debug");
  levelText->push_back("debug-1");
  levelText->push_back("debug-2");
  levelText->push_back("debug-3");

  g_ostream = stderr;
}

void VLogger::printv(Levels msgLevel, const char* fmt, ...)
{
  if ((baseLevel != SILENT) && (baseLevel >= msgLevel)) {

      va_list args;
      va_start(args, fmt);
      
      fprintf(g_ostream, "%s: ", (levelText->at(msgLevel)).c_str());
      
      va_end(args);
      fflush(g_ostream);
    }
}

void VLogger::printv(Levels msgLevel, const char *fmt, va_list args)
{
  if ((baseLevel != SILENT) && (baseLevel >= msgLevel)) {
    
      fprintf(g_ostream, "%s: ", (levelText->at(msgLevel)).c_str());
      vfprintf(g_ostream, fmt, args);
      
      fflush(g_ostream);
    }
}
