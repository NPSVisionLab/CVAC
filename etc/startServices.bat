@echo off
@setlocal
REM Edit the the INSTALLDIR line below with where the distribution was installed
set INSTALLDIR=__INSTALL_PATH__
if "%INSTALLDIR%" neq "" goto continue
echo "Install Directory must be set for startServices script to work!"
exit /b 1
:continue
REM If running the Java services (Corpus, FileServer, etc) edit JAVAEXE
REM and set to the path of the java executable
set JAVAEXE=__JAVA_PATH__
REM If running any Python services set in python.config set the path to
REM the Python 2.6 executable below
set PYTHONEXE=__PYTHON_PATH__
set THRDPARTYDIR=%INSTALLDIR%/3rdparty
set THRDPARTYLIBDIR=%INSTALLDIR%/3rdparty/lib
set JARDIR=%INSTALLDIR%/bin
set ICEDIR=%THRDPARTYDIR%/ICE
set ICEBOXJAR=%ICEDIR%/lib/IceBox.jar
set OPENCVDIR=%THRDPARTYDIR%/opencv
set PATH=bin;%ICEDIR%/bin;%OPENCVDIR%/bin;%THRDPARTYDIR%/libarchive\bin;%PATH%
chdir "%INSTALLDIR%"
start "CVAC Services (C++)" "cmd /K ""%ICEDIR%/bin/icebox.exe" --Ice.Config=config.icebox"

if "%JAVAEXE%" neq "" goto startjava
goto next
:startjava
    start "CVAC Services (Java)" "cmd /K ""%JAVAEXE%" -cp "%ICEDIR%/lib/Ice.jar;%ICEBOXJAR%;%JARDIR%/FileServer.jar;%JARDIR%/Corpus.jar;%THRDPARTYLIBDIR%/labelme.jar;%THRDPARTYLIBDIR%/javabuilder.jar;%THRDPARTYLIBDIR%/commons-io-2.4.jar;%THRDPARTYLIBDIR%/javatar-2.5.jar" IceBox.Server --Ice.Config=config.java_icebox"
:next
REM Python services that are listed in python.config
if "%PYTHONEXE%" neq "" (if exist "%INSTALLDIR%/python.config" goto startpython)
goto next2
:startpython
    for /F "eol=# tokens=*" %%A in (%INSTALLDIR%/python.config) do start "CVAC Service (Python)" "cmd /K ""%PYTHONEXE%" %%A"
:next2
echo CVAC services launched
exit /b 0
