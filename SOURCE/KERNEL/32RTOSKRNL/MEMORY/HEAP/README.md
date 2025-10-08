# HEAP Memory Management

HEAP in atOS-RT is a dynamic memory allocation system used by the kernel and processes to allocate and free memory at runtime. It is built on top of the page frame allocator and provides a more flexible way to manage memory compared to static allocation.

## Overview

Every process (including the kernel) allocates its own memory from KHEAP (Kernel HEAP). The kernel heap is initialized during the kernel startup and is used for dynamic memory allocations within the kernel space.

## Components

- **KHEAP.h**: Kernel heap management
- **KHEAP.c**: Implementation of kernel heap functions
