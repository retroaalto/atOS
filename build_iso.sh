#!/bin/bash
cd "$(dirname "$0")"
# rm -rf build
mkdir build
cd build
cmake ..
cd ..
cd build
ls ../build
make
if [ "$1" = "r" ]; then
    make run
fi