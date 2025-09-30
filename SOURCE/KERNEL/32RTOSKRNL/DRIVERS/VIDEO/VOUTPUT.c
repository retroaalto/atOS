#include <DRIVERS/VIDEO/VOUTPUT.h>
#include <DRIVERS/VIDEO/FONT12x24.h>

static OutputInfo cursor __attribute__((section(".data"))) = OUTPUT_INFO_INIT;

static U8 text_buffer[AMOUNT_OF_ROWS][AMOUNT_OF_COLS] __attribute__((section(".data"))) = {{0}};

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
U32 PUTC(U8 c) {
    if(AMOUNT_OF_COLS == 0 || AMOUNT_OF_ROWS == 0) return 0;

    if(c == '\n') {
        cursor.Column = 0;
        ROW_INC();
        return 1;
    }

    if(c == '\t') {
        U32 spaces = 4 - (cursor.Column % 4);
        for(U32 i = 0; i < spaces; i++) COLUMN_INC();
        return 1;
    }

    if(c < 32 || c > 255) return 0;

    text_buffer[cursor.Row][cursor.Column] = c; // update text buffer

    U32 x = COL_TO_PIX(cursor.Column);
    U32 y = ROW_TO_PIX(cursor.Row);
    for(U32 i = 0; i < CHAR_HEIGHT; i++) {
        VBE_LETTERS_TYPE row = FONT12x24[c][i];
        for(U32 j = 0; j < CHAR_WIDTH; j++) {
            VBE_PIXEL_COLOUR colour = (row & (1 << ((CHAR_WIDTH - 1) - j))) ? cursor.fgColor : cursor.bgColor;
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x + j, y + i, colour));
        }
    }

    COLUMN_INC();

    if(cursor.FLUSH_ON_CHANGE) {
        FLUSH_SCREEN();
    }

    return 1;
}


VOID CLS(U0) {}
U0 SET_BG_COLOR(VBE_PIXEL_COLOUR color) {}
U0 SET_FG_COLOR(VBE_PIXEL_COLOUR color) {}
U0 SET_CURSOR_POS(U32 col, U32 row) {}
U32 GET_CURSOR_POS(U0) {}
U0 SET_CURSOR_VISIBLE(BOOLEAN visible) {}
U0 SET_CURSOR_BLINK(BOOLEAN blink) {}
U0 SET_AUTO_SCROLL(BOOLEAN enable) {}
U0 SET_AUTO_WRAP(BOOLEAN enable) {}
U32 PUTS(U8 *str) {}
U0 FLUSH_SCREEN(U0) {
    VBE_UPDATE_VRAM();
}