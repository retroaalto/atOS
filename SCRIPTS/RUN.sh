#!/bin/sh

qemu-img create -f raw hdd.img 256M
qemu-system-i386 \
  -boot d \
  -cdrom atOS-RT.iso \
  -drive file=hdd.img,format=raw,if=ide,index=0,media=disk \
  -m 512