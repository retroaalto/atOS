// When compiling, include VIDEODRIVER.c, VBE.c, and VESA.c
#ifndef VOUTPUT_H
#define VOUTPUT_H
#include <DRIVERS/VIDEO/VBE.h>
#include <STD/ATOSMINDEF.h>

typedef struct {
    U32 x;
    U32 y;
} PixelPos;

typedef struct {

} DrawInfo;

BOOLEAN BEGIN_DRAWING(DrawInfo );

#endif // VOUTPUT_H