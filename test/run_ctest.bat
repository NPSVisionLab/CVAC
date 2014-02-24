@echo off
@setlocal 
REM command arguments must be:
REM %1: super-dir to bin/startIcebox.sh
REM %2: dir where tests are located
REM %3: ctest arguments, such as "-R File --verbose"
set ROOT_DIR=%1
set BUILD_DIR=%2
set CTEST_ARGS=%3
set CVAC_REMOTE_TEST_SERVER=172.20.80.78
set RES=0
REM start up services
cd "%ROOT_DIR%"
call bin\startIcebox.bat
timeout /T 30
REM run the tests, capture exit status
cd "%BUILD_DIR%"
if "%CTEST_ARGS%" equ "" (
    ctest 
)else (
    ctest %CTEST_ARGS%
)
if errorlevel  1 ( 
    set RES=1
)
REM irrespective of exit status: shut down services
REM then return exit status
cd "%ROOT_DIR%"
timeout /T 300
call bin\stopIcebox.bat
exit /B %RES%
