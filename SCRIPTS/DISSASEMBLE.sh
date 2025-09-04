#!/bin/bash

file_path="$1"

if [ ! -f "$file_path" ]; then
    echo "File not found!"
    exit 1
fi

objdump -d "$file_path" -M intel,i386 --architecture=i386