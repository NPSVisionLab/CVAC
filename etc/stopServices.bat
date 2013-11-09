@echo off
@setlocal
set INSTALLDIR=__INSTALL_PATH__
if "%INSTALLDIR" neq "" goto continue
echo "Install Directory must be set for startServices script to work!"
exit /b 1
:continue
REM If running any Python services set in python.config set the path to
REM the Python 2.6 executable below
set PYTHONEXE=__PYTHON_PATH__
set ICEDIR=%INSTALLDIR%/3rdparty/ICE
set PATH="%PATH%;%ICEDIR%/bin"
chdir "%INSTALLDIR%"

set LOCKFILE=.services_started.lock
if not exist "%LOCKFILE%" (
    echo "CVAC services supposedly have not been started (there is no file '%LOCKFILE%')."
    echo "Trying to stop them anyway..."
)
"%ICEDIR%/bin/iceboxadmin.exe" --Ice.Config=config.admin shutdown
"%ICEDIR%/bin/iceboxadmin.exe" --Ice.Config=config.java_admin shutdown
c:\windows\system32\taskkill.exe /FI "WINDOWTITLE eq CVAC Service*"
echo CVAC services stopped
if exist "%LOCKFILE%" (
    del /q /f %LOCKFILE%
)
exit /b 0
