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
 - CMake with Ninja
 - GENISOIMAGE

install these by running:
```bash
sudo apt-get install nasm genisoimage cmake ninja-build gcc
```

### Linux


```bash
# Create build files
mkdir build && cd build && cmake ..

# Create .img and .iso images
make

# Run with qemu
make run
# or
qemu-system-x86_64 -cdrom ./atos_rt.iso -boot d
```
