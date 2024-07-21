#!/bin/bash
echo "Cleaning up build directory..."

cd "$(dirname "$0")"
cd ../..
if [ -d "build" ]; then
    rm -rf build
fi

echo "Done."