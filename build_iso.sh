#!/bin/bash
cd "$(dirname "$0")"
mkdir build
cd build
cmake ..
cd ..
cd build
if [ "$1" = "fat_tool" ]; then
    make build_fat_tool
    echo TOOL FOUND IN ./build/tools/fat
    # ./tools/fat
    if [ "$2" = "make" ]; then
        make
    fi
    exit
fi
make
if [ "$1" = "rc" ]; then
    echo no can do
    exit 1
    make run_cdrom
fi
if [ "$1" = "rf" ]; then
    make run_floppy
fi