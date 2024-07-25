#!/bin/bash

# Ensure we are in the correct directory
cd "$(dirname "$0")"

# Create build directory if it doesn't exist
rm -rf ./build
mkdir -p build
cd build

# Run CMake to configure the project
cmake .. -DUSEDEBUG=1

# Build the project
# make VERBOSE=1
make

# Create the ISO image
make create_iso

# Check if ISO creation was successful
if [ -f ATOS.ISO ]; then
    echo "ISO file created successfully."
else
    echo "ISO creation failed."
    exit 1
fi
cd ..
cp ./build/ATOS.ISO .
echo ISO FILE COPIED TO PROJECT ROOT

