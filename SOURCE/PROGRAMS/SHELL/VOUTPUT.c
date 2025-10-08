#include <PROGRAMS/SHELL/VOUTPUT.h>
#include <PROGRAMS/SHELL/FONT8x16.h>
#include <STD/DEBUG.h>
#include <STD/STRING.h>
#include <STD/GRAPHICS.h>

/**
 * If automatic flush is enabled, updated only the area around the drawn character.
 * This is an optimization to avoid redrawing the entire screen for each character.
 */

static OutputInfo cursor __attribute__((section(".data"))) = { 0 };

static U8 text_buffer[AMOUNT_OF_ROWS][AMOUNT_OF_COLS] __attribute__((section(".data"))) = {0};

U0 INIT_SHELL_VOUTPUT(VOID) {
    // Clear text buffer
    for(U32 r = 0; r < AMOUNT_OF_ROWS; r++) {
        for(U32 c = 0; c < AMOUNT_OF_COLS; c++) {
            text_buffer[r][c] = ' ';
        }
    }
    cursor.Column = 0;
    cursor.Row = 0;
    cursor.fgColor = VBE_GREEN;
    cursor.bgColor = VBE_BLACK;
    cursor.AUTO_SCROLL = TRUE;
    cursor.AUTO_WRAP = TRUE;
    cursor.CURSOR_VISIBLE = TRUE;
    cursor.CURSOR_BLINK = TRUE;

    cursor.ROWS = AMOUNT_OF_ROWS;
    cursor.COLUMNS = AMOUNT_OF_COLS;
    cursor.SWIDTH = SCREEN_WIDTH;
    cursor.SHEIGHT = SCREEN_HEIGHT;

    cursor.text_buffer = (U8*)text_buffer; // Cast to U8* for easier access and dynamic allocation
}
OutputHandle GetOutputHandle(void) {
    return &cursor;
}
U0 SCROLL_TEXTBUFFER_UP(U0) {
    for(U32 r = 1; r < cursor.ROWS; r++) {
        for(U32 c = 0; c < cursor.COLUMNS; c++) {
            text_buffer[r - 1][c] = text_buffer[r][c];
        }
    }
    // Clear the last row
    for(U32 c = 0; c < cursor.COLUMNS; c++) {
        text_buffer[cursor.ROWS - 1][c] = ' ';
    }
}
U0 CLEAR_TEXTBUFFER(U0) {
    for(U32 r = 0; r < cursor.ROWS; r++) {
        for(U32 c = 0; c < cursor.COLUMNS; c++) {
            text_buffer[r][c] = ' ';
        }
    }
}
U0 COLUMN_INC(U0) {
    cursor.Column++;
    if(cursor.Column >= cursor.COLUMNS) {
        cursor.Column = 0;
        cursor.Row++;
        if(cursor.Row >= cursor.ROWS) {
            if(cursor.AUTO_SCROLL) {
                cursor.Row = cursor.ROWS - 1;
                SCROLL_TEXTBUFFER_UP();
            }

            if(cursor.AUTO_WRAP) {
                cursor.Row = 0;
            } else {
                cursor.Row = cursor.ROWS - 1;
            }
        }
    }
}
U0 COLUMN_DEC(U0) {
    if(cursor.Column > 0) {
        cursor.Column--;
    }
}
U0 ROW_INC(U0) {
    cursor.Row++;
    if(cursor.Row >= cursor.ROWS) {
        if(cursor.AUTO_SCROLL) {
            cursor.Row = cursor.ROWS - 1;
            SCROLL_TEXTBUFFER_UP();
        } else {
            cursor.Row = 0;
        }
    }
}
U0 ROW_DEC(U0) {
    if(cursor.Row > 0) {
        cursor.Row--;
    }
}

U32 COL_TO_PIX(U32 col) {
    return col * (CHAR_WIDTH + CHAR_SPACING);
}
U32 ROW_TO_PIX(U32 row) {
    return row * (CHAR_HEIGHT + CHAR_SPACING);
}

VOID DRAW_LINEINFO();
U32 PRINTF(U8 *format, ...) {}

U32 PUTC(U8 c) {
    if (AMOUNT_OF_COLS == 0 || AMOUNT_OF_ROWS == 0) return 0;

    // Handle newline
    if (c == '\n') {
        ROW_INC();
        return 1;
    }
    
    // Handle carriage return
    if (c == '\r') {
        cursor.Column = 0;
        return 1;
    }

    // Handle tab (4 spaces)
    if (c == '\t') {
        U32 spaces = 4 - (cursor.Column % 4);
        for (U32 i = 0; i < spaces; i++) COLUMN_INC();
        return 1;
    }

    // Update text buffer
    text_buffer[cursor.Row][cursor.Column] = c;

    U32 x = COL_TO_PIX(cursor.Column);
    U32 y = ROW_TO_PIX(cursor.Row);

    const U8 *glyph = &WIN1KXHR__8x16[c * CHAR_HEIGHT];

    // Draw the character
    for (U32 i = 0; i < CHAR_HEIGHT; i++) {
        U8 row = glyph[i];
        for (U32 j = 0; j < CHAR_WIDTH; j++) {
            U8 bit = (row >> (CHAR_WIDTH - 1 - j)) & 1;
            VBE_PIXEL_COLOUR colour = bit == 1 ? cursor.fgColor : cursor.bgColor;
            // MSB is leftmost pixel
            DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x + j, y + i, colour));
        }
    }

    // Move cursor
    COLUMN_INC();

    return 1;
}

U32 PUTS(U8 *str) {
    U32 count = 0;
    while (*str) {
        count += PUTC(*str++);
    }
    return count;
}
VOID CLS(U0) {
    CLEAR_SCREEN_COLOUR(cursor.bgColor);
    CLEAR_TEXTBUFFER();
}
U0 SET_BG_COLOR(VBE_PIXEL_COLOUR color) {}
U0 SET_FG_COLOR(VBE_PIXEL_COLOUR color) {}
U0 SET_CURSOR_POS(U32 col, U32 row) {}
U32 GET_CURSOR_POS(U0) {}
U0 SET_CURSOR_VISIBLE(BOOLEAN visible) {}
U0 SET_CURSOR_BLINK(BOOLEAN blink) {}
U0 SET_AUTO_SCROLL(BOOLEAN enable) {}
U0 SET_AUTO_WRAP(BOOLEAN enable) {}
VOID FLUSH_SCREEN(U0) {
    FLUSH_VRAM();
}