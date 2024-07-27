# atOS Renewed technology

## Running

Running with Qemu:
```
qemu-system-x86_64 -cdrom ./atos_rt.iso -boot d
```

## Building

### Requirements
 - NASM
 - C compiler
 - CMake and Ninja
 - GENISOIMAGE
 - DD

install these by running:
```bash
sudo apt-get install nasm genisoimage cmake ninja-build gcc
```

#### Linux

.ISO file can be created on linux by running:

```bash
build_iso.sh
```
