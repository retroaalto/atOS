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
 - DOSFTOOLS

install these by running:
```bash
sudo apt-get install nasm genisoimage cmake ninja-build gcc dosfstools 
```

### Linux


```bash
# Create build files
mkdir build && cd build && cmake ..

# Create .img and .iso images
make

# Run with qemu
make run_cdrom
# or
qemu-system-x86_64 -cdrom ./atos_rt.iso -boot d
```


## Debugging

### Windows

Install [Bochs binaries](https://github.com/bochs-emu/Bochs/releases/tag/REL_2_8_FINAL)

Load bochs_win.bxrc config file. Make sure that the paths are correct inside the file

Make sure to run ```bochsdbg.exe``` and not ```bochs.exe```
### Linux

Install Bochs by running:

```bash
sudo apt install bochs bochs-sdl bochsbios vgabios
```

Start debugging with:
```bash
debug.sh
# or
bochs -f bochs_config.bxrc
```