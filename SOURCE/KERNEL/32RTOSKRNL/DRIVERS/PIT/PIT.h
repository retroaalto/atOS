#ifndef PIT_H
#define PIT_H
#include "../../../STD/ATOSMINDEF.h"
#ifndef RTOS_KERNEL
U0 PIT_INIT(U0);
void pit_handler(I32 num, U32 errcode);
U32 *PIT_GET_TICKS_PTR();
U32 *PIT_GET_HZ_PTR();
#endif
#endif // PIT_H