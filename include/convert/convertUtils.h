#ifndef CONVERT_UTILS_H
#define CONVERT_UTILS_H

#include <string>
#include <Data.h>  // Ice classes
#include <Services.h>
#include <exception>
#include <util/VLogger.h>
#include <stdarg.h>

namespace cvac
{
	void addTrack(LabelableList* curList, FrameLocation& frloc);
}

#endif