#include <PROGRAMS/SHELL/VOUTPUT.h>
#include <PROGRAMS/SHELL/FONT8x16.h>
#include <STD/DEBUG.h>
#include <STD/STRING.h>
#include <STD/PROC_COM.h>
#include <CPU/PIT/PIT.h>
#include <STD/GRAPHICS.h>

#define RESTORE_CURSOR_BEFORE_MOVE() \
    RESTORE_CURSOR_UNDERNEATH(cursor.Column, cursor.Row)

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
    cursor.CURSOR_STYLE = CURSOR_BLOCK;
    cursor.INSERT_MODE = FALSE;
    // cursor.CURSOR_BLINK = FALSE;
    SET_CURSOR_BLINK(TRUE);
    TOGGLE_INSERT_MODE(); // Looks weird but starts NOT in insert mode
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
    // Move all rows up by one
    for (U32 r = 1; r < cursor.ROWS; r++) {
        for (U32 c = 0; c < cursor.COLUMNS; c++) {
            text_buffer[r - 1][c] = text_buffer[r][c];
        }
    }

    // Clear last row
    for (U32 c = 0; c < cursor.COLUMNS; c++) {
        text_buffer[cursor.ROWS - 1][c] = ' ';
    }

    // Redraw everything after scrolling
    DRAW_TEXT_BUFFER();
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

    if (cursor.Column >= cursor.COLUMNS) {
        cursor.Column = 0;
        cursor.Row++;

        // If we moved beyond the last row, scroll up
        if (cursor.Row >= cursor.ROWS) {
            SCROLL_TEXTBUFFER_UP();
            cursor.Row = cursor.ROWS - 1;
        }
    }
}

U0 COLUMN_DEC(U0) {
    if(cursor.Column > 0) {
        cursor.Column--;
    } else if(cursor.Row > 0) {
        cursor.Row--;
        cursor.Column = cursor.COLUMNS - 1;
    }
}
U0 ROW_INC(U0) {
    cursor.Row++;
    if(cursor.Row >= cursor.ROWS) {
        cursor.Row = cursor.ROWS - 1;
        SCROLL_TEXTBUFFER_UP();
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

VOID TOGGLE_INSERT_MODE() {
    cursor.INSERT_MODE = !cursor.INSERT_MODE;
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}
U32 PRINTF(U8 *format, ...) {}

VOID NEW_ROW(VOID) {
    ROW_INC();
    cursor.Column = 0;
}

static inline void DRAW_CHAR_AT(U32 col, U32 row, U8 ch, VBE_PIXEL_COLOUR fg, VBE_PIXEL_COLOUR bg) {
    U32 x = COL_TO_PIX(col);
    U32 y = ROW_TO_PIX(row);
    const U8 *glyph = &WIN1KXHR__8x16[ch * CHAR_HEIGHT];

    for (U32 i = 0; i < CHAR_HEIGHT; i++) {
        U8 bits = glyph[i];
        for (U32 j = 0; j < CHAR_WIDTH; j++) {
            U8 bit = (bits >> (CHAR_WIDTH - 1 - j)) & 1;
            VBE_PIXEL_COLOUR colour = bit ? fg : bg;
            DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x + j, y + i, colour));
        }
    }
}



void REDRAW_CHAR(U32 col, U32 row) {
    U8 ch = text_buffer[row][col];
    DRAW_CHAR_AT(col, row, ch, cursor.fgColor, cursor.bgColor);
}

// Draw block cursor glyph (ASCII 219) over the current cell
static inline void DRAW_BLOCK_CURSOR(U32 col, U32 row) {
    DRAW_CHAR_AT(col, row, 219, cursor.fgColor, cursor.bgColor);
}

// Restore the cell under cursor (redraw original char)
static inline void RESTORE_CURSOR_UNDERNEATH(U32 col, U32 row) {
    REDRAW_CHAR(col, row);
}

VOID PUT_CURRENT_PATH(VOID) {
    PUTS("/");
    return;
}

U0 SET_CURSOR_VISIBLE(BOOLEAN visible) {
    static BOOLEAN was_visible = TRUE;
    if (visible && !was_visible) {
        DRAW_CURSOR_AT(cursor.Column, cursor.Row);
    } else if (!visible && was_visible) {
        RESTORE_CURSOR_AT(cursor.Column, cursor.Row);
    }
    was_visible = visible;
}

U0 SET_CURSOR_BLINK(BOOLEAN blink) {
    cursor.CURSOR_BLINK = blink;

    // When disabling blink, make sure cursor is drawn and stays visible
    if (!blink) {
        DRAW_CURSOR_AT(cursor.Column, cursor.Row);
    }
}



// Draw the standard block cursor (ASCII 219) at given cell
// Draw block cursor by inverting colors of the character underneath
static inline void DRAW_BLOCK_CURSOR_AT(U32 col, U32 row) {
    U8 ch = text_buffer[row][col];

    // Swap foreground and background colors for inverted look
    VBE_PIXEL_COLOUR fg = cursor.bgColor;
    VBE_PIXEL_COLOUR bg = cursor.fgColor;

    DRAW_CHAR_AT(col, row, ch, fg, bg);
}


// Draw underline cursor: draw a horizontal bar covering bottom two pixel rows
static inline void DRAW_UNDERLINE_CURSOR_AT(U32 col, U32 row) {
    U32 x = COL_TO_PIX(col);
    U32 y = ROW_TO_PIX(row);
    // draw last 2 rows of the character cell
    U32 start = CHAR_HEIGHT - 2;
    for (U32 i = start; i < CHAR_HEIGHT; i++) {
        for (U32 j = 0; j < CHAR_WIDTH; j++) {
            DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x + j, y + i, cursor.fgColor));
        }
    }
}

// Draw bar cursor: vertical line at leftmost column of glyph
static inline void DRAW_BAR_CURSOR_AT(U32 col, U32 row) {
    U32 x = COL_TO_PIX(col);
    U32 y = ROW_TO_PIX(row);
    // draw a 2-pixel wide vertical bar for visibility
    for (U32 i = 0; i < CHAR_HEIGHT; i++) {
        for (U32 j = 0; j < 2; j++) {
            DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x + j, y + i, cursor.fgColor));
        }
    }
}

// Generic draw according to current style (draws on top of existing pixels)
void DRAW_CURSOR_AT(U32 col, U32 row) {
    switch (cursor.CURSOR_STYLE) {
        case CURSOR_BLOCK:
            DRAW_BLOCK_CURSOR_AT(col, row);
            break;
        case CURSOR_UNDERLINE:
            DRAW_UNDERLINE_CURSOR_AT(col, row);
            break;
        case CURSOR_BAR:
            DRAW_BAR_CURSOR_AT(col, row);
            break;
        default:
            DRAW_BLOCK_CURSOR_AT(col, row);
            break;
    }
}


// Restore underlying character (always redraws from text_buffer)
void RESTORE_CURSOR_AT(U32 col, U32 row) {
    RESTORE_CURSOR_UNDERNEATH(col, row);
}

// ---------------------- Insert mode helpers ----------------------
// Shift contents right from col (inclusive) to make room for insertion.
// last column is dropped.
VOID MOVE_BUFFER_CONTENTS_RIGHT_FROM(U32 col, U32 row) {
    if (col >= cursor.COLUMNS) return;
    // Start from end and move towards col+1
    for (int c = (int)cursor.COLUMNS - 1; c > (int)col; c--) {
        text_buffer[row][c] = text_buffer[row][c - 1];
        REDRAW_CHAR((U32)c, row);
    }
    // the cell at [row][col] will be overwritten by caller
}

// Shift contents left from col (inclusive) removing the char at col.
// last column becomes space.
VOID MOVE_BUFFER_CONTENTS_LEFT_FROM(U32 col, U32 row) {
    if (col >= cursor.COLUMNS) return;
    for (U32 c = col; c < cursor.COLUMNS - 1; c++) {
        text_buffer[row][c] = text_buffer[row][c + 1];
        REDRAW_CHAR(c, row);
    }
    text_buffer[row][cursor.COLUMNS - 1] = ' ';
    REDRAW_CHAR(cursor.COLUMNS - 1, row);
}

VOID HANDLE_BACKSPACE() {
    // If at top-left, nothing to delete
    if (cursor.Row == 0 && cursor.Column == 0) return;

    // Before changing, remove visible cursor
    RESTORE_CURSOR_AT(cursor.Column, cursor.Row);

    // If we're at column 0 but not top row, go to previous line's end
    if (cursor.Column == 0) {
        ROW_DEC();
        cursor.Column = cursor.COLUMNS - 1;
    } else {
        cursor.Column--; // move to the char to delete (left of original cursor)
    }

    // Delete that character (shift left)
    MOVE_BUFFER_CONTENTS_LEFT_FROM(cursor.Column, cursor.Row);

    // Ensure the entire tail of row is redrawn
    for (U32 c = cursor.Column; c < cursor.COLUMNS; c++) {
        REDRAW_CHAR(c, cursor.Row);
    }

    // Draw cursor at new position immediately
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}



VOID DELETE_CHAR_AT_CURSOR() {
    RESTORE_CURSOR_AT(cursor.Column, cursor.Row);

    // remove char under cursor by shifting left from cursor.Column
    MOVE_BUFFER_CONTENTS_LEFT_FROM(cursor.Column, cursor.Row);

    // redraw affected region
    for (U32 c = cursor.Column; c < cursor.COLUMNS; c++) {
        REDRAW_CHAR(c, cursor.Row);
    }

    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}


VOID BLINK_CURSOR(VOID) {
    static BOOLEAN visible = TRUE;
    static U32 last_toggle_tick = 0;
    U32 current_tick = GET_PIT_TICKS();

    if (!cursor.CURSOR_BLINK) {
        if (!visible) {
            visible = TRUE;
            DRAW_CURSOR_AT(cursor.Column, cursor.Row);
        }
        return;
    }

    if ((current_tick - last_toggle_tick) >= PIT_TICK_MS * 5) { // Toggle every 500ms
        last_toggle_tick = current_tick;
        visible = !visible;

        if (visible)
            DRAW_CURSOR_AT(cursor.Column, cursor.Row);
        else
            RESTORE_CURSOR_AT(cursor.Column, cursor.Row);
    }
}


VOID PUT_SHELL_START(VOID) {
    PUT_CURRENT_PATH();
    PUTS("> ");
    return;
}

U32 PUTC(U8 c) {
    if (AMOUNT_OF_COLS == 0 || AMOUNT_OF_ROWS == 0) return 0;

    // Hide cursor while updating cell to avoid artifacts
    RESTORE_CURSOR_AT(cursor.Column, cursor.Row);

    // Handle newline
    if (c == '\n') {
        cursor.Column = 0;
        ROW_INC();
        DRAW_CURSOR_AT(cursor.Column, cursor.Row);
        return 1;
    }

    // Handle carriage return
    if (c == '\r') {
        cursor.Column = 0;
        DRAW_CURSOR_AT(cursor.Column, cursor.Row);
        return 1;
    }

    // Handle tab (4 spaces) - insert spaces
    if (c == '\t') {
        U32 spaces = 4 - (cursor.Column % 4);
        for (U32 i = 0; i < spaces; i++) {
            if (cursor.INSERT_MODE) {
                MOVE_BUFFER_CONTENTS_RIGHT_FROM(cursor.Column, cursor.Row);
            }
            text_buffer[cursor.Row][cursor.Column] = ' ';
            REDRAW_CHAR(cursor.Column, cursor.Row);
            COLUMN_INC();
        }
        DRAW_CURSOR_AT(cursor.Column, cursor.Row);
        return 1;
    }

    // Normal character
    if (cursor.INSERT_MODE) {
        // Make room at current cursor position
        MOVE_BUFFER_CONTENTS_RIGHT_FROM(cursor.Column, cursor.Row);
    }

    text_buffer[cursor.Row][cursor.Column] = c;
    REDRAW_CHAR(cursor.Column, cursor.Row);

    // Advance cursor
    COLUMN_INC();

    // Immediately show cursor at new pos
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);

    return 1;
}
U32 PUTS(U8 *str) {
    U32 count = 0;
    while (*str) {
        count += PUTC(*str++);
    }
    return count;
}

VOID DRAW_TEXT_BUFFER(VOID) {
    for (U32 r = 0; r < cursor.ROWS; r++) {
        for (U32 c = 0; c < cursor.COLUMNS; c++) {
            REDRAW_CHAR(c, r);
        }
    }
}

VOID CLS(U0) {
    CLEAR_SCREEN_COLOUR(cursor.bgColor);
    CLEAR_TEXTBUFFER();
    DRAW_TEXT_BUFFER();
}

U0 SET_BG_COLOR(VBE_PIXEL_COLOUR color) {}
U0 SET_FG_COLOR(VBE_PIXEL_COLOUR color) {}
U0 SET_CURSOR_POS(U32 col, U32 row) {}
U32 GET_CURSOR_POS(U0) {}
U0 SET_AUTO_SCROLL(BOOLEAN enable) {}
U0 SET_AUTO_WRAP(BOOLEAN enable) {}

VOID FLUSH_SCREEN(U0) {
    FLUSH_VRAM();
}

#define RESTORE_CURSOR_IF_VISIBLE() \
    do { \
        if (cursor.CURSOR_BLINK || cursor.CURSOR_STYLE == CURSOR_BLOCK) { \
            RESTORE_CURSOR_UNDERNEATH(cursor.Column, cursor.Row); \
        } \
    } while(0)

VOID HANDLE_KEY_ARROW_LEFT() {
    RESTORE_CURSOR_IF_VISIBLE();
    if (cursor.Column > 0) {
        COLUMN_DEC();
    } else if (cursor.Row > 0) {
        cursor.Row--;
        cursor.Column = cursor.COLUMNS - 1;
    }
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}

VOID HANDLE_KEY_ARROW_RIGHT() {
    RESTORE_CURSOR_IF_VISIBLE();
    if (cursor.Column < cursor.COLUMNS - 1) {
        COLUMN_INC();
    } else if (cursor.Row < cursor.ROWS - 1) {
        cursor.Row++;
        cursor.Column = 0;
    }
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}

VOID HANDLE_KEY_ARROW_UP() {
    RESTORE_CURSOR_IF_VISIBLE();
    if (cursor.Row > 0) {
        cursor.Row--;
        if (cursor.Column >= cursor.COLUMNS)
            cursor.Column = cursor.COLUMNS - 1;
    }
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}

VOID HANDLE_KEY_ARROW_DOWN() {
    RESTORE_CURSOR_IF_VISIBLE();
    if (cursor.Row < cursor.ROWS - 1) {
        cursor.Row++;
        if (cursor.Column >= cursor.COLUMNS)
            cursor.Column = cursor.COLUMNS - 1;
    }
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}


VOID HANDLE_KEY_ENTER() {
    RESTORE_CURSOR_BEFORE_MOVE();
    cursor.Column = 0;
    ROW_INC();
}

VOID HANDLE_KEY_DELETE() {
    DELETE_CHAR_AT_CURSOR();
}

VOID HANDLE_CTRL_C() {
    // Draw `^C` at current position
    PUTC('^');
    PUTC('C');
    
    RESTORE_CURSOR_BEFORE_MOVE();
    cursor.Column = 0;
    ROW_INC();

    // Print new prompt
    PUT_SHELL_START();
}