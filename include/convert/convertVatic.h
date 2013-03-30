#ifndef CONVERT_VATIC_H
#define CONVERT_VATIC_H

#include <string>
#include <Data.h>  // Ice classes
#include <Services.h>
#include <exception>
#include <util/VLogger.h>
#include <stdarg.h>

namespace cvac
{
	LabelableList* convert(std::string filePath);
}

#endif