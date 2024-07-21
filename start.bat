@ECHO off
cd /d %~dp0 
RMDIR /Q /S .\.git
git init -b main
git add .
git commit -m "Initial commit"
git status
DEL start.sh
DEL start.bat