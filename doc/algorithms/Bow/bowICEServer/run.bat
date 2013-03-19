@echo off
set SAVEPATH=%PATH%
set PATH=%PATH%:../../../bin
rem  Bat script to run icebox with the OpenCVPerformance service.
@echo on
%ICE_ROOT%\bin\iceboxd --Ice.Config=config.icebox
@echo off
REM set PATH=%SAVEPATH%
