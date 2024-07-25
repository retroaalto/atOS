# atOS Renewed technology

## Building

### Requirements
 - NASM
 - CMake and Ninja
 - GENISOIMAGE
 - Somekind of c compiler

install these by running:
```bash
sudo apt-get install nasm genisoimage cmake ninja-build gcc
```


#### Linux

.ISO file can only be created on linux. You can create it by running:

```bash
build_iso.sh
```
## Running

### Windows

Download qemu binaries for [Windows](https://qemu.weilnetz.de/w64/)

Run the ISO file:
```
qemu_start.bat
```