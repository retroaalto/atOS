# atOS-RT - atOS Revised Technology

A 32-bit operating system.

---

## Table of Contents

- [Overview](#overview)
- [Running the ISO](#running-the-iso)
- [Development](#development)
  - [Dependencies](#dependencies)
  - [Building](#building)
  - [Debugging](#debugging)
- [Tools](#tools)
- [License](#license)

---

## Overview

**atOS-RT** is a 32-bit operating system project aimed at providing a platform for exploring low-level system programming concepts. It includes a custom bootloader, kernel, and tools for ISO and FAT file system manipulation.

---

## Running the ISO

To run the `atOS-RT.iso`, you can use any virtual machine software. Below is an example using QEMU:

### Install QEMU

```bash
sudo apt install qemu-system-x86
```

### Run the ISO

```bash
qemu-system-i386 -boot d -cdrom atOS-RT.iso -m 512
```

---

## Development

### Dependencies

Install the required dependencies for running and developing atOS-RT:

#### For Running atOS-RT

```bash
sudo apt install qemu-system-x86
```

#### For Developing/Building atOS-RT

```bash
sudo apt install qemu-system-x86 nasm make gcc genisoimage gdb bochs bochs-x
```

---

### Building

Use the `make` command to build and run the project. Below are some common commands:

#### View Available Commands

```bash
make help
```

#### Build and Run the ISO

```bash
make iso run
```

#### Build and Debug the ISO

```bash
make iso debug
```

---

### Debugging

To debug the project using GDB:

1. Build the ISO in debug mode:
   ```bash
   make iso debug
   ```

2. Start GDB with the provided configuration:
   ```bash
   gdb -x ./.gdbinit
   ```

---

## Tools

A set of tools is provided to simplify development and testing. These tools are located in the `TOOLS` directory.

### Usage

Navigate to the `TOOLS` directory:

```bash
cd ./TOOLS
```

#### View Available Commands

```bash
make help
```

#### Run ISO Test File

```bash
make iso run
```

#### Run FAT Test File

```bash
make fat run
```

---

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.