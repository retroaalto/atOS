#!/bin/bash
echo "Setting global variables..."

ROOTPATH="$(cd "$(dirname "$0")"; pwd)"
CMAKEPATH="cmake"
NINJAPATH="ninja"
# Place here the same name you have in CMakelists.txt (project_name)
PROJECTNAME="C_CPP_BASE" 
COMPIlER_CXX_PATH="g++"
COMPILER_C_PATH="gcc"

echo "Done."

export ROOTPATH TOOLSPATH CMAKEPATH NINJAPATH PROJECTNAME COMPILER_C_PATH COMPIlER_CXX_PATH

