@echo off
REM Script to get around problem with Windows loading the tcl library.
REM This will be fixed in another virtualenv release but for now do this.
call virt\Scripts\activate.bat
set TCL_LIBRARY=c:\python27\tcl\tcl8.5
python %1
