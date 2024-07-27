#!/bin/bash
cd "$(dirname "$0")"
set +e
rm -f *.img
rm -f *.iso
rm -f *.bin
rm -rf build
rm -rf iso
set -e
mkdir -p ./build

echo Assembling bootloader_1
nasm -f bin -o ./build/bootloader_1.bin ./source/bootloader_1.asm
nasm -f bin -o ./build/bootloader_2.bin ./source/bootloader_2.asm


echo Assembling kernel
nasm -fbin -o ./build/kernel.bin ./source/kernel.asm
if [ $? -ne 0 ]; then
    echo "NASM kernel.asm assembly failed"
    exit 1
fi

echo Creating 1.44mb floppy
dd if=/dev/zero of=floppy.img bs=1024 count=1440
if [ $? -ne 0 ]; then
    echo "Floppy creation failed_1"
    exit 1
fi

echo Adding bootloader_1 to floppy
dd if=./build/bootloader_1.bin of=floppy.img seek=0 count=1 conv=notrunc
if [ $? -ne 0 ]; then
    echo "Floppy creation failed_2"
    exit 1
fi

# echo "Adding kernel to floppy"
# dd if=./build/kernel.bin of=floppy.img bs=512 seek=1 conv=notrunc
# if [ $? -ne 0 ]; then
#     echo "Failed to write kernel to floppy"
#     exit 1
# fi

mkdir -p iso
cp floppy.img ./iso/

echo Creating ISO image
genisoimage -quiet -V 'ATOS' -input-charset iso8859-1 -o atos_rt.iso -b floppy.img \
    -hide floppy.img ./iso/

if [ "$1" = "r" ]; then
    qemu-system-x86_64 -cdrom ./atos_rt.iso -boot d
fi