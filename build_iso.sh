#!/bin/bash
cd "$(dirname "$0")"
rm -f *.img
rm -f *.iso
rm -f *.bin
rm -rf build
rm -rf iso

mkdir -p ./build

echo Assembling bootloader
nasm -f bin -o ./build/bootloader.bin ./source/bootloader.asm
if [ $? -ne 0 ]; then
    echo "NASM assembly failed"
    exit 1
fi

echo Creating 1.44mb floppy
dd if=/dev/zero of=floppy.img bs=1024 count=1440
if [ $? -ne 0 ]; then
    echo "Floppy creation failed_1"
    exit 1
fi

echo Adding bootloader to floppy
dd if=./build/bootloader.bin of=floppy.img seek=0 count=1 conv=notrunc
if [ $? -ne 0 ]; then
    echo "Floppy creation failed_2"
    exit 1
fi

mkdir -p iso
cp floppy.img ./iso/

echo Creating ISO image
genisoimage -quiet -V 'ATOS' -input-charset iso8859-1 -o atos_rt.iso -b floppy.img \
    -hide floppy.img ./iso/

if [ "$1" = "r" ]; then
    qemu-system-x86_64 -cdrom ./atos_rt.iso -boot d
fi