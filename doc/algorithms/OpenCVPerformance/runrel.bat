@echo off
REM set SAVEPATH=%PATH%
REM set PATH=%PATH%:../../bin
rem  Bat script to run icebox with the OpenCVPerformance service.
@echo on
%ICE_ROOT%\bin\icebox --Ice.Config=config.icebox
@echo off
REM set PATH=%SAVEPATH%
