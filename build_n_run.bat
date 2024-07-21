@echo off 
cd /d %~dp0
cd z-win
call build.bat
call run.bat %*