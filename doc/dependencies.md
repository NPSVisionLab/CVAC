---
layout: default
title: Dependencies
---
# Manually Installing Dependencies

Please note that for many platforms you can automatically download and use a 3rd-party dependency package, simply by checking the `DOWNLOAD_3RDPARTY_PACKAGE` option in the CMake build process.
* [Python 2.7](http://www.python.org)
* [ZeroC ICE](http://www.zeroc.com/overview.html)
* [OpenCV](http://opencv.org/)
* [libarchive](https://code.google.com/p/libarchive/) (CVAC has been tested with libarchive version 2.8.5. ([source](https://github.com/libarchive/libarchive))
* [Java SE](http://www.oracle.com/technetwork/java/javase/downloads/index.html) (Java 6 or Java 7 required for JavaFileServer and JavaCorpusServer only)
* [UnitTest++](http://unittest-cpp.sourceforge.net/) (required for testing only)
* [Matlab Compiler Runtime](http://www.mathworks.com/products/compiler/mcr/) (required for LabelMe Corpus only)
Required only if you would like to create a Corpus from a LabelMe server.  Download the appropriate installer for your platform and follow the instructions.  The MCR is not required for building but only during runtime.

## Platform-specific instructions (partially complete only)

### Ubuntu
`sudo apt-get update`
`sudo apt-get install make git cmake-curses-gui g++ openjdk-6-jdk libarchive-dev libjavatar-java libcommons-io-java zeroc-ice34 libunittest++`
`sudo add-apt-repository ppa:fkrull/dependencies`
`sudo apt-get update`
`sudo apt-get install python2.7`

### RedHat
`sudo yum install ice-c++-devel.x86_64 ice-java-devel.x86_64 ice-python-devel.x86_64 libarchive-dev libunittest++`

### Mac OSX
It is easiest to install most software packages via [MacPorts](http://www.macports.org).  Once you have MacPorts, do this:
`sudo port install zeroc-ice34`  (or zeroc-ice35 etc)
`sudo port install libarchive` ([libarchive at MacPorts](http://www.macports.org/ports.php?by=library&substr=libarchive)
`sudo port install unittest-cpp` ([UnitTest++ at MacPorts](http://www.macports.org/ports.php?by=library&substr=unittest-cpp)) 

### Windows
#### [ICE](http://www.zeroc.com):
[Download](http://www.zeroc.com/download.html) 
and install the most recent MSI installer for 32-bit Ice.  The installer will suggest a default location, but any location should be possible as CMake finds the path to the Ice Installation directory (from the Windows registry or manually).

#### [libarchive](https://code.google.com/p/libarchive/):
Either install it, or get the latest source from [github](https://github.com/libarchive/libarchive) and build it.  
http://www.libarchive.org and download 'libarchive-3.1.2' under the first link: 'Stable release'. Extract the top-level folder contents: 'libarchive-3.1.2' to /3rdparty subfolder of your CVAC source folder:  ...'CVAC_source_dir'/3rdparty
for example: /user/gitProjects/CVAC_localClone/CVAC/3rdparty
Opening CMake GUI (ccmake util) and running 'Configure' should fill in paths to: LIBARCHIVE_INCLUDE, and LIBARCHIVE_LIBRARY.

#### [UnitTest++](http://unittest-cpp.sourceforge.net/):
Goto the sourceforge page at: http://sourceforge.net/projects/unittest-cpp/
Click the green link button download the latest Zip archive of UnitTest++.
Unzip the top-level folder contents (UnitTest++) to  /3rdparty subfolder of your CVAC source folder:  ...'CVAC_source_dir'/3rdparty

for example: /user/gitProjects/CVAC_localClone/CVAC/3rdparty
Open the latest included version of the Visual Studio project, and allow it to be converted to your working VisualStudio version.  Build both Debug and Release versions of the UnitTest++ library and run the INSTALL target, so that both Debug and Release libraries are installed in 3rdparty/UnitTest++/lib. 
Opening CMake GUI (or ccmake util) and running 'Configure' should fill in paths to: UNITTEST++_INNCLUDE_DIR, UNITTEST++_LIBRARY, and UNITTEST++_LIBRARY_DEBUG.
