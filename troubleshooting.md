---
layout: default
title: EasyCV Troubleshooting
---

### IceBox.jar not found
IceBox.jar only exists in ICE 3.5, not in ICE 3.4.2 (currently the default for CVAC).  This should not cause any errors during build or run time.

### 'module' object has no attribute 'getDetector'
If running your Python code produces an error similar to this:
detector = easy.getDetector( "bowTest:default -p 10104" )
AttributeError: 'module' object has no attribute 'getDetector'

Then Python is loading the wrong "easy" module.  Make sure you either change the PYTHONPATH environment variable to point to easy.py or you call "python setup.py install" with the Easy! setup script.
export PYTHONPATH=src/easy:lib/python

### Missing bzip2.dll entry points
Windows: Missing entry points in bzip2.dll: make sure you use an up-to-date version. If you installed the 3rd party package, your PATH environment variable should contain c:\path\to\3rdparty\ICE\bin as one of the first entries.

### Fatal Python error: Interpreter not initialized (version mismatch?)
OpenCV is built with Python2.6 and so is ICE.  Make sure all your Python libraries are built for 2.6, too.

### Ice::upCast undefined
Undefined symbols:
  "Ice::upCast(Ice::ObjectFactory*)", referenced from...

Possible reason 1: You are using two different Ice versions.  Make sure you have only one installed - best is the one that comes in the 3rdparty package.  On OSX, check if you have local versions installed via macports (they would be in /opt/local).  You can pre-compile the sliced files (Data.cpp, for example) with gcc -E and make sure you get the right include files.

Possible reason 2: Compiler issues between GNU and Apple, such that the Ice library is compiled with one and you're compiling with the other.  Try switching your compiler: llvm-g++ and llvm-gcc, or i686-apple-darwin10-gcc etc.  But then again, compiler issues might be due to this reason:

Possible reason 3: Apple's compiler is not fully supported by ICE's "slice2cpp" compiler.  See [here](http://www.zeroc.com/forums/bug-reports/4965-slice2cpp-output-does-not-compile-standards-conformant-compiler.html) for more.  Solution: define CC=/usr/bin/gcc and CXX=/usr/bin/g++ **before** running CMake for the first time.  For OSX Mavericks define CC=/usr/bin/cc and CXX=/usr/bin/g++-4.2 also set the cmake build variable CMAKE_CXX_COMPILER=/usr/bin/g++-4.2. You can also set the cmake build variable CMAKE_C_COMPILER=/usr/bin/cc instead of defining CC.


### Ice::UnexpectedObjectException
When invoking an ICE RPC, the seemingly correct argument does not get recognized:

unknown = Exception.cpp:30: Ice::UnexpectedObjectException:
unexpected class instance of type '::cvac::PurposedList'; expected instance of type '::cvac::PurposedList':
expected element of type '::cvac::PurposedList' but received '::cvac::PurposedList'

That's likely due to the client and server being built against a different Data.ice or other *.ice file versions.  Rebuild everything, server and client, and make sure they slice from the same Data.ice file.  If you are sure that it's not a different ice file, it might be the optimization flags for your compiler.  OSX with heavy optimizations seems to produce slightly different slice types - maybe one gets optimized away.  If you get this error running OSX Mavericks version see "Possible reason 3:" above for a possible solution.

### Connection refused exception
Even though you have a service running locally, and you can connect via "nc" or "telnet", the ICE client won't let you connect.

This might be due to "-h localhost" being specified in the config.service file, but not in the client.  Or the other way around.  Particularly Win8 and some Linux systems want the explicit specification of localhost, or none.