---
layout: default
title: Building from Source
---
# Building EasyCV from Source

EasyCV has a few dependencies, some of which are optional.  If available, you can download a "3rd Party" package for your specific platform or install each dependency manually.  Once done, build project files for your favorite tool chain with CMake, open the project, and build it.  Details are in the following sections.

## Building and Installation

Generate makefiles or project files for your favorite IDE with [CMake](http://www.cmake.org) by running ccmake or the [CMake GUI](http://www.cmake.org/cmake/help/runningcmake.html).  If you have all dependencies installed (see below) and the CMake generator succeeded building makefiles or project files, just build within your favorite IDE.

After building your project code, build the "INSTALL" project or call `make install` to put scripts and libraries into the right places.

## Running from a Source Build

The location of files differs from where the installer puts the components as described here.
You run EasyCV from the build directory instead of from the root directory.  So if your root directory is /myname/CVAC, your build directory is /myname/CVAC/build, and the CMAKE\_INSTALL\_PREFIX is
"/myname/CVAC/build/installed," then you have to cd to /myname/CVAC/build to run installed/EasyCV/bin/startServices.sh

Config files are generated into the build directory.  Binaries and
libraries are in build/bin and build/lib, respectively.  Client code
will have to set env variables as specified at the end of the make install to run.
Note that the 3rdparty dependencies do not get installed but remain in the root directory.
For example, the PIL (Pillow) library needs to be unzipped and the PYTHONPATH has to
point to it:

```sh
cd /myname/CVAC/3rdparty
unzip Pillow-2.6.0-cp27-none-macosx.zip
export PYTHONPATH=$PYTHONPATH:/myname/CVAC/3rdparty/PIL
```

To bring up the EasyCV Control Center, call

`python /myname/CVAC/build/installed/EasyCV/src/easy/gui.py`.

Now read the [User Documentation](user-documentation.html).

## Dependencies

For the most popular platforms, you can simply check the following CMake option and a package with all 3rd-party dependencies will automatically be downloaded and extracted into a "3rdparty" subdirectory.

`DOWNLOAD_3RDPARTY_PACKAGE = ON`

You need to "configure" CMake once so you can see this option.  Ignore the errors that you might be getting.  Then check this DOWNLOAD\_3RDPARTY\_PACKAGE option (not: BUILD\_3RDPARTY\_PACKAGE) and "configure" again.  Check that it is using the libraries in the 3rdparty folder now.  If not, you have to "delete" those variables and "configure" again.  Note that some variables are "advanced" and only visible when you toggled into that mode.  Mixing packages from your local installation and 3rdparty is possible, as long as you don't mix the source of include files and libraries _within_ a package.

If you prefer to use existing local installations for the dependencies, or install the manually, you can follow these steps for [manually installing 3rd-party dependencies](dependencies.html).

## Integrating Your Algorithms

If you have additional algorithms, add CMake code for them in UserCMakeLists.txt.  For example, this file could look like this:

    OPTION(BUILD_WITH_MYALGORITHM "Enables the awesome MyAlgorithm" ON)
    IF(BUILD_WITH_MYALGORITHM)
       FIND_PACKAGE( OpenCV REQUIRED core imgproc highgui PATHS ../3rdparty/OpenCV-2.4.2)
    ENDIF (BUILD_WITH_MYALGORITHM)

## Problems?  Issues?

See the [troubleshooting](troubleshooting.html) page.

