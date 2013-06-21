#
# startServices.sh  Script to start CVAC services for binary distribution
#
# Please set INSTALLDIR to install directory
export INSTALLDIR=
if [ "${INSTALLDIR}" == "" ]
then
    echo "INSTALLDIR needs to be defined! Please set INSTALLDIR to binary distribution install directory"
    exit
fi
# Please set JAVAEXE to java executable
export JAVAEXE=/usr/bin/java
# Pleas set PYTHONEXE to python 2.6 executable
export PYTHONEXE=/usr/bin/python2.6

export ICEDIR=${INSTALLDIR}/3rdparty/ICE
export OPENCVDIR=${INSTALLDIR}/3rdparty/opencv
export PATH=$PATH:${INSTALLDIR}/bin
export MATLAB_MCR_DYLD_PATH="/Applications/MATLAB/MATLAB_Compiler_Runtime/v80/runtime/maci64:/Applications/MATLAB/MATLAB_Compiler_Runtime/v80/sys/os/maci64:/Applications/MATLAB/MATLAB_Compiler_Runtime/v80/bin/maci64:/System/Library/Frameworks/JavaVM.framework/JavaVM:/System/Library/Frameworks/JavaVM.framework/Libraries"
export XAPPLRESDIR=/Applications/MATLAB/MATLAB_Compiler_Runtime/v80/X11/app-defaults
export DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:${INSTALLDIR}/lib:${OPENCVDIR}/lib:${ICEDIR}/lib:$MATLAB_MCR_DYLD_PATH"

# C/C++ services, via IceBox
cd ${INSTALLDIR}
${ICEDIR}/bin/icebox --Ice.Config=config.icebox &

# Java services, via Java IceBox
if [ "${JAVAEXE}" != "" ]
then
    ${JAVAEXE} -cp "${ICEDIR}/lib/Ice.jar:${INSTALLDIR}/bin/FileServer.jar:${INSTALLDIR}/bin/Corpus.jar:${INSTALLDIR}/3rdparty/lib/labelme.jar:${INSTALLDIR}/3rdparty/lib/javabuilder.jar:${INSTALLDIR}/3rdparty/lib/commons-io-2.4.jar:${INSTALLDIR}/3rdparty/lib/javatar-2.5.jar" \
        IceBox.Server --Ice.Config=config.java_icebox &
fi

# Python services that are listed in python.config
if [ "${PYTHONEXE}" != "" ] && [ -f "${INSTALLLDIR}/python.config" ]
then
    export PYTHONPATH="${ICEDIR}/python:${INSTALLDIR}/lib/python:${OPENCVDIR}/lib/python2.6/site-packages"
    grep -v -e ^# ${INSTALLDIR}/python.config | while read LINE
    do
        ${PYTHONEXE} $LINE &
    done
fi

echo CVAC services launched
exit
