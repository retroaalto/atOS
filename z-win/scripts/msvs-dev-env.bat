@ECHO OFF
cd /d %~dp0
call globals.bat

ECHO Setting up Microsoft Visual Studio development environment...

if not defined DevEnvDir ( 
    call "%TOOLSPATH%\VsDevCmd.bat"
)

ECHO Done.