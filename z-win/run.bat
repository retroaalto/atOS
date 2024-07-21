@echo off
cd /d %~dp0
call .\scripts\globals.bat
cd ..
cd ..\build
.\win\%PROJECTNAME%.exe %*
echo %ERRORLEVEL%
cd ..