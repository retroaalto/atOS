#ifndef RTOSKRNL_H
#define RTOSKRNL_H
#ifndef RTOS_KERNEL
#define RTOS_KERNEL
#endif // RTOS_KERNEL
#include "../../STD/ASM.h"
#include "../../STD/ATOSMINDEF.h"
#include "./DRIVERS/VIDEO/VBE.h"
#include "./DRIVERS/PS2/PS2.h"
#include "./DRIVERS/PS2/KEYBOARD/PS2_KEYBOARD.h"
#include "./RTOSKRNL/RTOSKRNL_INTERNAL.h"
#include "./KERNEL.h"

__attribute__((noreturn))
U0 rtos_kernel(PTR_LIST *ptr_list);

__attribute__((noreturn, section(".text")))
void _start(PTR_LIST *ptr_list);

#endif // RTOSKRNL_H