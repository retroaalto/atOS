#ifndef RTOSKRNL_H
#define RTOSKRNL_H
#include "../../STD/ASM.h"
#include "../../STD/ATOSMINDEF.h"
#include "./DRIVERS/VIDEO/VBE.h"

__attribute__((noreturn))
U0 rtos_kernel(U0);

__attribute__((noreturn, section(".text")))
void _start(void);

#endif // RTOSKRNL_H