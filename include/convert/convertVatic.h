#ifndef CONVERT_VATIC_H
#define CONVERT_VATIC_H

#include <string>
#include <Data.h>  // Ice classes
#include <Services.h>
#include <exception>
#include <convert/convertUtils.h>
#include <util/VLogger.h>
#include <util/FileUtils.h>
#include <stdarg.h>
#include <fstream>
#include <iostream>

/*
#ifdef WIN32
  #  define USE_UNORDERED_MAP 1
#else
  #  define USE_UNORDERED_MAP 0
#endif
*/

//#if USE_UNORDERED_MAP
  #include <map>
//#endif

//using namespace std::tr1;
//typedef unordered_map<int, bool> ObjIdMap; 

namespace cvac
{
	LabelableList* convert(std::string filePath);
}

#endif