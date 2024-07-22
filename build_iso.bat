@echo off
setlocal
cd /D %~dp0
set "ARG1=%1"

:: For components needed for iso
set "OUTPUT=output"
set "WINPE=WinPE"
:: Cmake build directory
set "BUILD=build"
:: For nasm
set "BOOTLOADER_BUILD=NASM_BUILD"
:: Final location for ISO image
set "ISO_OUTPUT=ISO_OUTPUT"
if /I "%ARG1%" == "DEL" (
    RMDIR /S /Q %OUTPUT%
    RMDIR /S /Q %BUILD%
    RMDIR /S /Q %BOOTLOADER_BUILD%
    RMDIR /S /Q %ISO_OUTPUT%
    RMDIR /S /Q %WINPE%
)

set "MSVSTOOLSPATH=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools"
set "CMAKEPATH=C:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\2022\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE"
set "NINJAPATH=C:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\2022\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja"
set "IMGBURNPATH=C:\Program Files (x86)\ImgBurn"
set "COMPILERPATH=cl.exe"
set "NASMLOCATION=%USERPROFILE%\AppData\Local\bin\NASM"
set "ADKPEPATH=C:\Program Files (x86)\Windows Kits\10\Assessment and Deployment Kit\Windows Preinstallation Environment"

set "PROJECT_NAME=atOS-RT"
set "ARCH=amd64"

ECHO Setting up Microsoft Visual studio environment
if not defined DevEnvDir ( 
    call "%MSVSTOOLSPATH%\VsDevCmd.bat"
)

ECHO Adding NASM to local path
set "PATH=%PATH%;%NASMLOCATION%"

nasm --help > nul  || (
                        echo "No nasm found"
                        exit /b 1
                        )

if not exist "%OUTPUT%" mkdir "%OUTPUT%"
if not exist "%BUILD%" mkdir "%BUILD%"
if not exist "%BOOTLOADER_BUILD%" mkdir "%BOOTLOADER_BUILD%"
if not exist "%ISO_OUTPUT%" mkdir "%ISO_OUTPUT%"

cd "%BOOTLOADER_BUILD%"
ECHO Assembling bootloader file
nasm -f bin ..\source\BOOTLOADER.ASM -o .\bootloader.bin

ECHO Compiling kernel

ECHO Creating .ISO file
cd /D %~dp0

setlocal
    @REM Scripts don't do this themselves??
    set "WinPERoot=%ADKPEPATH%"
    set "OSCDImgRoot=C:\Program Files (x86)\Windows Kits\10\Assessment and Deployment Kit\Deployment Tools\%ARCH%\Oscdimg"
    Echo Adding %OSCDImgRoot% to PATH
    set "PATH=%PATH%;%OSCDImgRoot%"
    call "%ADKPEPATH%\copype.cmd" %ARCH% ".\%WINPE%\"

    ECHO Moving files to ISO folder ^(%OUTPUT%^)
    COPY /B/Y ".\%BOOTLOADER_BUILD%\bootloader.bin" ".\%OUTPUT%\"

    call "%ADKPEPATH%\MakeWinPEMedia.cmd" /ISO ".\%WINPE%\" ".\%ISO_OUTPUT%\%PROJECT_NAME%-%ARCH%.ISO"
endlocal 

endlocal