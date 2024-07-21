#!/bin/sh
echo "Building project..."

cd "$(dirname "$0")"

. ./scripts/globals.sh

cd ..

mkdir -p build
cd build

# For release:
if [ "$1" = "Release" ]; then
    cmake -G "Ninja" -DCMAKE_C_COMPILER="$COMPILER_C_PATH" -DCMAKE_CXX_COMPILER="$COMPIlER_CXX_PATH" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_INSTALL_PREFIX="./install/x64-release"  -DUSEDEBUG:STRING="0" -DCMAKE_MAKE_PROGRAM="$NINJAPATH" ..
    cmake --build . --config Release
    if [ $? -ne 0 ]; then
        echo "Build failed, exiting..."
        exit 1
    fi
    cd ..
    exit 0
fi

# For debug:
cmake -G "Ninja" -DCMAKE_C_COMPILER="$COMPILER_C_PATH" -DCMAKE_CXX_COMPILER="$COMPIlER_CXX_PATH" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_INSTALL_PREFIX="./install/x64-debug"  -DUSEDEBUG:STRING="1" -DCMAKE_MAKE_PROGRAM="$NINJAPATH" ..
cmake --build . --config Debug

if [ $? -ne 0 ]; then
    echo "Build failed, exiting..."
    exit 1
fi

cd ..