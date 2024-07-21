@ECHO OFF
setlocal

cd /d %~dp0
call .\scripts\msvs-dev-env.bat
cd ..\..
ECHO Building project...

if not exist build mkdir build
cd build

@REM Release version
if /I "%1" == ""Release"" (
    "C:\Windows\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "%CMAKEPATH%\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_C_COMPILER:STRING="%COMPILERPATH%" -DCMAKE_CXX_COMPILER:STRING="%COMPILERPATH%" -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_INSTALL_PREFIX:PATH="./install/x64-release"   -DCMAKE_MAKE_PROGRAM="%NINJAPATH%\ninja.exe" -DATRCBUILDCVPATH:PATH="%ATRCBUILDPATH%" -DSFMLBUILDCVPATH:PATH="%SFMLBUILDPATH%" -DUSEDEBUG:STRING="0" ".."
    if %ERRORLEVEL% NEQ 0 (
        ECHO Build failed, exiting...
        exit /B 1
    )
    cmake --build . --config Release
    if %ERRORLEVEL% NEQ 0 (
        ECHO Build failed, exiting...
        exit /B 1
    )
    cd..
    exit /B 0
)
@REM Debug:
"C:\Windows\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "%CMAKEPATH%\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_C_COMPILER:STRING="%COMPILERPATH%" -DCMAKE_CXX_COMPILER:STRING="%COMPILERPATH%" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="./install/x64-debug"   -DCMAKE_MAKE_PROGRAM="%NINJAPATH%\ninja.exe" -DATRCBUILDCVPATH:PATH="%ATRCBUILDPATH%" -DSFMLBUILDCVPATH:PATH="%SFMLBUILDPATH%" -DUSEDEBUG:STRING="1" ".."
if %ERRORLEVEL% NEQ 0 (
    ECHO Build failed, exiting...
    exit /B 1
)
cmake --build . --config Debug

if %ERRORLEVEL% NEQ 0 (
    ECHO Build failed, exiting...
    exit /B 1
)
CD ..
