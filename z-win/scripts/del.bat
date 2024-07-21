@ECHO OFF
ECHO Cleaning up build directory...

cd /d %~dp0
cd ..\..
if exist build RMDIR /S /Q build

ECHO Done.