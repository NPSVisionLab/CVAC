@echo off
set SAVEPATH=%PATH%
set PATH=../bin;%PATH%
rem  Bat script to run icebox with the all services
@echo on
%ICE_ROOT%\bin\iceboxd --Ice.Config=config.icebox
@echo off
set PATH=%SAVEPATH%
