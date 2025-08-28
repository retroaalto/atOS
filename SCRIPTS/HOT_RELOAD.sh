#!/bin/bash

# Paths
ISO_IMAGE="./OUTPUT/ISO/atOS.iso"

# Step 1: Build kernel and iso
echo "Building kernel..."
make iso

# Step 2: Start or reload QEMU
if [ -z "$1" ]; then
    make runlh
else
    # Hot reload: change CDROM in running QEMU via monitor
nc localhost 4444 <<EOF
change ide1-cd0 ./OUTPUT/ISO/atOS.iso
system_reset
EOF
fi
