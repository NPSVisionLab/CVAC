@echo off
REM set SAVEPATH=%PATH%
REM set PATH=%PATH%:../../bin
rem  Bat script to run icebox with the WireDiagram service
@echo on
java -cp %ICE_ROOT%\lib\Ice.jar:WireDiagram.jar:javacv.jar IceBox.Server --Ice.Config=config.icebox
@echo off
REM set PATH=%SAVEPATH%
