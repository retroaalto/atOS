#include <PROGRAMS/SHELL/VOUTPUT.h>
#include <PROGRAMS/SHELL/FONT8x16.h>
#include <STD/DEBUG.h>
#include <STD/STRING.h>

static OutputInfo cursor __attribute__((section(".data"))) = { 0 };

static U8 text_buffer[AMOUNT_OF_ROWS][AMOUNT_OF_COLS] __attribute__((section(".data"))) = {{0}};

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
    cursor.FLUSH_ON_CHANGE = TRUE;
    // Clear screen
    VBE_CLEAR_SCREEN(cursor.bgColor);
}
OutputHandle *GetOutputHandle(void) {
    return &cursor;
}
U0 SCROLL_TEXTBUFFER_UP(U0) {
    for(U32 r = 1; r < AMOUNT_OF_ROWS; r++) {
        for(U32 c = 0; c < AMOUNT_OF_COLS; c++) {
            text_buffer[r - 1][c] = text_buffer[r][c];
        }
    }
    // Clear the last row
    for(U32 c = 0; c < AMOUNT_OF_COLS; c++) {
        text_buffer[AMOUNT_OF_ROWS - 1][c] = ' ';
    }
}
U0 CLEAR_TEXTBUFFER(U0) {
    for(U32 r = 0; r < AMOUNT_OF_ROWS; r++) {
        for(U32 c = 0; c < AMOUNT_OF_COLS; c++) {
            text_buffer[r][c] = ' ';
        }
    }
}
U0 COLUMN_INC(U0) {
    cursor.Column++;
    if(cursor.Column >= AMOUNT_OF_COLS) {
        cursor.Column = 0;
        cursor.Row++;
        if(cursor.Row >= AMOUNT_OF_ROWS) {
            if(cursor.AUTO_SCROLL) {
                cursor.Row = AMOUNT_OF_ROWS - 1;
                SCROLL_TEXTBUFFER_UP();
            }

            if(cursor.AUTO_WRAP) {
                cursor.Row = 0;
            } else {
                cursor.Row = AMOUNT_OF_ROWS - 1;
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
    if(cursor.Row >= AMOUNT_OF_ROWS) {
        if(cursor.AUTO_SCROLL) {
            cursor.Row = AMOUNT_OF_ROWS - 1;
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
        cursor.Column = 0;
        ROW_INC();
        return 1;
    }

    // Handle tab (4 spaces)
    if (c == '\t') {
        U32 spaces = 4 - (cursor.Column % 4);
        for (U32 i = 0; i < spaces; i++) COLUMN_INC();
        return 1;
    }

    // Ignore control characters
    if (c > 255) return 0;

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
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x + j, y + i, colour));
        }
    }

    // Move cursor
    COLUMN_INC();

    // Flush if needed
    if (cursor.FLUSH_ON_CHANGE) {
        FLUSH_SCREEN();
    }

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
    VBE_CLEAR_SCREEN(cursor.bgColor);
    VBE_UPDATE_VRAM();
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
    VBE_UPDATE_VRAM();
}