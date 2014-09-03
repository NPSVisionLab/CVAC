---
layout: default
title: Building from Source
---
# Building EasyCV from Source

EasyCV has a few dependencies, some of which are optional.  If available, you can download a "3rd Party" package for your specific platform or install each dependency manually.  Once done, build project files for your favorite tool chain with CMake, open the project, and build it.  Details are in the following sections.

## Building and Installation

Generate project files for your favorite IDE with [CMake](http://www.cmake.org): makefiles with ccmake or the [CMake GUI](http://www.cmake.org/cmake/help/runningcmake.html).  If you have all dependencies installed:

_that's it!_  Once installed, read the [User Documentation](https://github.com/NPSVisionLab/CVAC/wiki/User-Documentation).

### Dependencies

For the most popular platforms, you can simply check the following CMake option and a package with all 3rd-party dependencies will automatically be downloaded and extracted into a "3rdparty" subdirectory.

`DOWNLOAD_3RDPARTY_PACKAGE = ON`

You need to "configure" CMake once so you can see this option.  Ignore the errors that you might be getting.  Then check this DOWNLOAD_3RDPARTY_PACKAGE option (not: BUILD_3RDPARTY_PACKAGE) and "configure" again.  Check that it is using the libraries in the 3rdparty folder now.  If not, you have to "delete" those variables and "configure" again.  Note that some variables are "advanced" and only visible when you toggled into that mode.  Mixing packages from your local installation and 3rdparty is possible, as long as you don't mix the source of include files and libraries _within_ a package.

If you prefer to use existing local installations for the dependencies, or install the manually, you can follow these steps for [manually installing 3rd-party dependencies](dependencies.html).

### Issues
On some Mac platforms the default C/C++ compilers do not accept the Ice definitions and complain about "upCast" issues.  If you have non-LLVM versions of gcc and g++, build with those instead of the default clang compiler:

`CC=/usr/bin/gcc CXX=/usr/bin/g++ cmake ..`

Note that setting -D arguments when invoking cmake is probably insufficient, it will get ignored.  Likewise, setting the CMAKE_CXX_COMPILER variable in a CMake GUI is not going to change the compiler for good.  Also, make sure that gcc is not the LLVM version (gcc --version should not mention LLVM).

If you encounter this error: " 'ptrdiff_t' does not name a type", you need to patch Ice as described here: [http://www.zeroc.com/forums/bug-reports/5697-include-ice-buffer-h-41-17-error-ptrdiff_t-does-not-name-type.html](http://www.zeroc.com/forums/bug-reports/5697-include-ice-buffer-h-41-17-error-ptrdiff_t-does-not-name-type.html)

If you have additional algorithms, add CMake code for them in UserCMakeLists.txt.  For example, this file could look like this:

    OPTION(BUILD_WITH_MYALGORITHM "Enables the awesome MyAlgorithm" ON)
    IF(BUILD_WITH_MYALGORITHM)
       FIND_PACKAGE( OpenCV REQUIRED core imgproc highgui PATHS ../3rdparty/OpenCV-2.4.2)
    ENDIF (BUILD_WITH_MYALGORITHM)
  