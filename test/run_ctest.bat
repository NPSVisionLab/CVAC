@echo off
@setlocal 
REM command arguments must be:
REM %1: super-dir to bin/startIcebox.sh
REM %2: dir where tests are located
REM %3: ctest arguments, such as "-R File --verbose"
REM %4: remote file service test machine to connect to
set ROOT_DIR=%1
set BUILD_DIR=%2
set CTEST_ARGS=%3
REM Tell the Tests what machine to use for remote fileserver tests
set CVAC_REMOTE_TEST_SERVER=%4
set RES=0
REM start up services
cd "%BUILD_DIR%"
REM in case it crashed and left the services running stop them
call bin\stopIcebox.bat
waitfor DUMMY /T 2 2> NUL
call bin\startIcebox.bat
waitfor DUMMY /T 5 2> NUL
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
cd "%BUILD_DIR%"
call bin\stopIcebox.bat
exit /B %RES%
