#ifndef PIC_H
#define PIC_H

#include "../../../STD/ATOSMINDEF.h"
void PIC_Unmask(int irq);
void PIC_Mask(int irq);
#define PIC_REMAP_OFFSET 0x20
void pic_send_eoi(U8 irq);
void pic_remap(U8 offset1, U8 offset2);


#endif // PIC_H