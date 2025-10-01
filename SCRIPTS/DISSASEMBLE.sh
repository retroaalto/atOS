#!/bin/bash

file_path="$1"

if [ ! -f "$file_path" ]; then
    echo "File not found!"
    exit 1
fi

objdump -D -b binary -m i386 "$file_path" > dissasembled_output.txt
