@echo off
@setlocal
set INSTALLDIR=""
if "%INSTALLDIR" neq "" goto goto continue
echo "Install Directory must be set for startServices script to work!"
exit /b 1
:continue
REM If running any Python services set in python.config set the path to
REM the Python 2.6 executable below
set PYTHONEXE=""
set ICEDIR=%INSTALLDIR%/3rdparty/ICE
set PATH=%PATH%;%ICEDIR%/bin
chdir "%INSTALLDIR%"
"%ICEDIR%/bin/iceadmin.exe" --Ice.Config=config.admin shutdown
"%ICEDIR%/bin/iceadmin.exe" --Ice.Config=config.java_admin shutdown
if "%PYTHONEXE%" neq "" taskkill /FI "WINDOWTITLE eq CVAC Service (Python)" /IM python.exe
echo CVAC services stopped
exit /b 0
