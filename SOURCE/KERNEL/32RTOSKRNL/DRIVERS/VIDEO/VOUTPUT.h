// When compiling, include VIDEODRIVER.c, VBE.c, and VESA.c
#ifndef VOUTPUT_H
#define VOUTPUT_H
#include <DRIVERS/VIDEO/VBE.h>
#include <STD/TYPEDEF.h>

typedef struct {
    U32 Column;
    U32 Row;
    U32 bgColor;
    U32 fgColor;
    BOOLEAN AUTO_SCROLL;
    BOOLEAN AUTO_WRAP;
    BOOLEAN CURSOR_VISIBLE;
    BOOLEAN CURSOR_BLINK; 
    BOOLEAN FLUSH_ON_CHANGE; // If TRUE, the screen is updated immediately after drawing
} OutputInfo;

typedef OutputInfo* OutputHandle;

#undef SCREEN_WIDTH
#undef SCREEN_HEIGHT
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define CHAR_WIDTH    12
#define CHAR_HEIGHT   24
#define CHAR_SPACING  2
#define AMOUNT_OF_COLS (SCREEN_WIDTH / (CHAR_WIDTH + CHAR_SPACING))
#define AMOUNT_OF_ROWS (SCREEN_HEIGHT / (CHAR_HEIGHT + CHAR_SPACING))
#define CHARACTERS 256 // Extended ASCII characters

#define FILLSHAPE 0x0000001
typedef U32 DrawInfo;

U0 COLUMN_INC(U0);
U0 COLUMN_DEC(U0);
U0 ROW_INC(U0);
U0 ROW_DEC(U0);
U0 SET_BG_COLOR(U32 color);
U0 SET_FG_COLOR(U32 color);
U0 SET_CURSOR_POS(U32 col, U32 row);
U32 GET_CURSOR_POS(U0);
U0 SET_CURSOR_VISIBLE(BOOLEAN visible);
U0 SET_CURSOR_BLINK(BOOLEAN blink);
U0 SET_AUTO_SCROLL(BOOLEAN enable);
U0 SET_AUTO_WRAP(BOOLEAN enable);
U32 PUTS(U8 *str);
U32 PUTC(U8 c);
VOID CLS(U0);
VOID FLUSH_SCREEN(U0);

BOOLEAN BEGIN_DRAWING(DrawInfo drawInfo);

#endif // VOUTPUT_H