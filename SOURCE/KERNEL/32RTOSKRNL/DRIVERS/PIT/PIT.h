#ifndef PIT_H
#define PIT_H
#include "../../../STD/ATOSMINDEF.h"
U0 PIT_INIT(U0);
U32 *PIT_GET_TICKS_PTR();
U32 *PIT_GET_HZ_PTR();
void pit_set_frequency(U32 freq);

#endif // PIT_H