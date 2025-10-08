// When compiling, include VIDEODRIVER.c, VBE.c, and VESA.c
// This is a shell-level text output driver using VBE graphics mode
// integrated part of atOShell, not a separate library! Do not use
// in other programs.
#ifndef VOUTPUT_H
#define VOUTPUT_H
#include <DRIVERS/VIDEO/VBE.h>
#include <STD/TYPEDEF.h>

typedef struct {
    U32 Column;
    U32 Row;
    VBE_PIXEL_COLOUR bgColor;
    VBE_PIXEL_COLOUR fgColor;
    BOOLEAN AUTO_SCROLL;
    BOOLEAN AUTO_WRAP;
    BOOLEAN CURSOR_VISIBLE;
    BOOLEAN CURSOR_BLINK; 

    // Screen size in characters
    U32 ROWS;
    U32 COLUMNS;
    
    // Screen width and height in pixels
    U32 SWIDTH;
    U32 SHEIGHT; 

    U32 X_POS; // Left-most pixel of the shell area
    U32 Y_POS; // Top-most pixel of the shell area

    // Text buffer for the screen
    // Each character is represented by a single byte
    U8 *text_buffer;
} OutputInfo;

typedef OutputInfo* OutputHandle;

OutputHandle GetOutputHandle(void);

#undef  SCREEN_WIDTH
#undef  SCREEN_HEIGHT
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

#define CHAR_WIDTH    8
#define CHAR_HEIGHT   16

#define CHAR_SPACING  2
#define AMOUNT_OF_COLS (SCREEN_WIDTH / (CHAR_WIDTH + CHAR_SPACING))
#define AMOUNT_OF_ROWS (SCREEN_HEIGHT / (CHAR_HEIGHT + CHAR_SPACING))
#define CHARACTERS 256 // Extended ASCII characters

U0 INIT_SHELL_VOUTPUT(VOID);
// Increases the cursor column by one, moving to next row if at end
U0 COLUMN_INC(U0);
// Decreases the cursor column by one, if not at the start
U0 COLUMN_DEC(U0);
// Increases the cursor row by one, moving to next column if at end
U0 ROW_INC(U0);
// Decreases the cursor row by one, if not at the top
U0 ROW_DEC(U0);
// Sets the background color
U0 SET_BG_COLOR(VBE_PIXEL_COLOUR color);
// Sets the foreground (text) color
U0 SET_FG_COLOR(VBE_PIXEL_COLOUR color);
// Sets the cursor position to (col, row)
U0 SET_CURSOR_POS(U32 col, U32 row);
// Gets the current cursor position as a single U32, with high 16 bits as row and low 16 bits as column
U32 GET_CURSOR_POS(U0);
// Sets the visibility of the cursor
U0 SET_CURSOR_VISIBLE(BOOLEAN visible);
// Enables or disables cursor blinking
U0 SET_CURSOR_BLINK(BOOLEAN blink);
// Enables or disables automatic scrolling when the cursor moves beyond the last row
U0 SET_AUTO_SCROLL(BOOLEAN enable);
// Enables or disables automatic line wrapping when the end of a line is reached
U0 SET_AUTO_WRAP(BOOLEAN enable);
// Writes a null-terminated string to the screen
U32 PUTS(U8 *str);
// Draws a single character at (x, y) in pixels
U32 PUTC(U8 c);

// Formatted print to the screen, similar to printf in C standard library
// Supports %c, %s, %d, %u, %x, and %%
U32 PRINTF(U8 *format, ...);

// Clears the screen and resets cursor position
VOID CLS(U0);
// Immediately updates the screen with the current text buffer
VOID FLUSH_SCREEN(U0);
// Converts a text column number to pixel X coordinate
U32 COL_TO_PIX(U32 col);
// Converts a text row number to pixel Y coordinate
U32 ROW_TO_PIX(U32 row);

#endif // VOUTPUT_H