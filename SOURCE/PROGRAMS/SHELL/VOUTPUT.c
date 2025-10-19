#include <PROGRAMS/SHELL/VOUTPUT.h>
#include <PROGRAMS/SHELL/FONT8x16.h>
#include <PROGRAMS/SHELL/SHELL.h>
#include <PROGRAMS/SHELL/COMMANDS.h>
#include <STD/DEBUG.h>
#include <STD/STRING.h>
#include <STD/PROC_COM.h>
#include <CPU/PIT/PIT.h>
#include <STD/GRAPHICS.h>
#include <STD/MEM.h>
#define RESTORE_CURSOR_BEFORE_MOVE() \
    RESTORE_CURSOR_UNDERNEATH(cursor.Column, cursor.Row)

static OutputInfo cursor __attribute__((section(".data"))) = { 0 };

static U8 text_buffer[AMOUNT_OF_ROWS][AMOUNT_OF_COLS] __attribute__((section(".data"))) = {0};
static U8 current_line[CUR_LINE_MAX_LENGTH] ATTRIB_DATA;
static SHELL_INSTANCE *shndl ATTRIB_DATA = NULLPTR;
static U32 prompt_length ATTRIB_DATA = 0;
static U32 prompt_row ATTRIB_DATA = 0;   // row where the current prompt lives
static U32 edit_pos ATTRIB_DATA = 0;     // index into current_line (0..len)
static U32 history_index ATTRIB_DATA = 0; // 0 = current line, 1..history_count = browsing history

// Command history (up to CMD_LINE_HISTORY entries)
// Each entry is a dynamically allocated string
static U8 *line_history[CMD_LINE_HISTORY] ATTRIB_DATA = {0};
static U32 history_count ATTRIB_DATA = 0;

static volatile U32 pending_scrolls ATTRIB_DATA = 0;
static U32 last_scroll_process_tick ATTRIB_DATA = 0;
#define SCROLL_PROCESS_INTERVAL_MS 16

// ---------------------- Local fast-path helpers ----------------------
static inline BOOLEAN is_printable(U8 c) {
    return (c >= 0x20 && c != 0x7F);
}

// Draw a background rectangle for len characters at (col,row)
static inline VOID DRAW_CELL_BG_RUN(U32 col, U32 row, U32 len, VBE_PIXEL_COLOUR bg) {
    if (len == 0) return;
    U32 x = COL_TO_PIX(col);
    U32 y = ROW_TO_PIX(row);
    U32 cell_adv = CHAR_WIDTH + CHAR_SPACING;
    U32 w = len * cell_adv - CHAR_SPACING; // include inner spacings, not trailing gap
    DRAW_FILLED_RECTANGLE(x, y, w, CHAR_HEIGHT, bg);
}

// Efficiently draw a string of length len at cell (col,row)
// Background is cleared with a single filled-rectangle per run, then
// foreground spans are emitted as horizontal lines per bit-run.
static VOID DRAW_STRING_AT(U32 col, U32 row, const U8 *s, U32 len, VBE_PIXEL_COLOUR fg, VBE_PIXEL_COLOUR bg) {
    if (!s || len == 0) return;

    // Clear background for the entire span with a single call (per row strip)
    DRAW_CELL_BG_RUN(col, row, len, bg);

    // For each glyph scanline, draw contiguous foreground runs merged across characters
    U32 base_x = COL_TO_PIX(col);
    U32 base_y = ROW_TO_PIX(row);
    const U32 cell_adv = CHAR_WIDTH + CHAR_SPACING;
    for (U32 i = 0; i < CHAR_HEIGHT; i++) {
        BOOLEAN in_run = FALSE;
        U32 run_start_x = 0;
        // Scan across all bits in the span
        for (U32 ci = 0; ci < len; ci++) {
            U8 ch = s[ci];
            const U8 *glyph = &WIN1KXHR__8x16[ch * CHAR_HEIGHT];
            U8 bits = glyph[i];
            for (U32 bj = 0; bj < CHAR_WIDTH; bj++) {
                U8 bit = (bits >> (CHAR_WIDTH - 1 - bj)) & 1;
                U32 px = base_x + ci * cell_adv + bj;
                if (bit) {
                    if (!in_run) {
                        in_run = TRUE;
                        run_start_x = px;
                    }
                } else if (in_run) {
                    // finish run up to previous pixel
                    DRAW_LINE(run_start_x, base_y + i, px - 1, base_y + i, fg);
                    in_run = FALSE;
                }
            }
        }
        if (in_run) {
            // close last run to the end of the span
            U32 end_x = base_x + len * cell_adv - CHAR_SPACING - 1;
            DRAW_LINE(run_start_x, base_y + i, end_x, base_y + i, fg);
        }
    }
}

// Write a span into the text buffer and draw it in batched mode.
// Handles wrapping across rows.
static U32 WRITE_SPAN_FAST(const U8 *s, U32 len) {
    if (!s || len == 0) return 0;
    U32 written = 0;

    // Hide any cursor glyph overlay before we modify cells
    RESTORE_CURSOR_UNDERNEATH(cursor.Column, cursor.Row);

    while (written < len) {
        if (cursor.Row >= cursor.ROWS) {
            SCROLL_TEXTBUFFER_UP();
            cursor.Row = cursor.ROWS - 1;
            cursor.Column = 0;
        }

        U32 avail = (cursor.COLUMNS > cursor.Column) ? (cursor.COLUMNS - cursor.Column) : 0;
        if (avail == 0) {
            // wrap to next line
            cursor.Column = 0;
            ROW_INC();
            continue;
        }
        U32 chunk = (len - written < avail) ? (len - written) : avail;

        // Copy into text buffer
        MEMCPY(&text_buffer[cursor.Row][cursor.Column], s + written, chunk);

        // Draw at once
        DRAW_STRING_AT(cursor.Column, cursor.Row, s + written, chunk, cursor.fgColor, cursor.bgColor);

        cursor.Column += chunk;
        written += chunk;
    }

    // Redraw cursor at the end position
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
    return written;
}

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
    SET_CURSOR_VISIBLE(TRUE);
    TOGGLE_INSERT_MODE(); // Looks weird but starts NOT in insert mode
    cursor.ROWS = AMOUNT_OF_ROWS;
    cursor.COLUMNS = AMOUNT_OF_COLS;
    cursor.SWIDTH = SCREEN_WIDTH;
    cursor.SHEIGHT = SCREEN_HEIGHT;

    cursor.text_buffer = (U8*)text_buffer; // Cast to U8* for easier access and dynamic allocation
    MEMZERO(current_line, CUR_LINE_MAX_LENGTH);
    shndl = GET_SHNDL();
}
OutputHandle GetOutputHandle(void) {
    return &cursor;
}

U0 SCROLL_TEXTBUFFER_UP(U0) {
    // Shift text buffer rows (fast)
    size_t row_bytes = (size_t)cursor.COLUMNS;
    MEMMOVE(&text_buffer[0][0], &text_buffer[1][0], row_bytes * (cursor.ROWS - 1));
    for (U32 c = 0; c < cursor.COLUMNS; c++) text_buffer[cursor.ROWS - 1][c] = ' ';

    // Mark that a scroll happened and needs a redraw, but don't redraw here
    pending_scrolls++;
}


VOID PUSH_TO_HISTORY(U8 *line) {
    if (history_count < CMD_LINE_HISTORY) {
        line_history[history_count] = (U8 *)MAlloc(STRLEN(line) + 1);
        STRNCPY(line_history[history_count], line, STRLEN(line) + 1);
        history_count++;
    } else {
        // Free the oldest entry
        Free(line_history[0]);
        // Shift all entries up
        for (U32 i = 1; i < CMD_LINE_HISTORY; i++) {
            line_history[i - 1] = line_history[i];
        }
        // Add new entry at the end
        line_history[CMD_LINE_HISTORY - 1] = (U8 *)MAlloc(STRLEN(line) + 1);
        STRNCPY(line_history[CMD_LINE_HISTORY - 1], line, STRLEN(line) + 1);
    }
}

VOID CLEAR_HISTORY() {
    for (U32 i = 0; i < history_count; i++) {
        if (line_history[i]) {
            Free(line_history[i]);
            line_history[i] = NULLPTR;
        }
    }
    history_count = 0;
}

U8 *GET_CURRENT_LINE() {
    return current_line;
}

VOID CLEAR_CURRENT_LINE() {
    MEMZERO(current_line, CUR_LINE_MAX_LENGTH);
    edit_pos = 0;
}
VOID PRINTNEWLINE(VOID) {
    cursor.Column = 0;
    ROW_INC();
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}

U0 CLEAR_TEXTBUFFER(U0) {
    MEMZERO(text_buffer, AMOUNT_OF_ROWS * AMOUNT_OF_COLS);
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

static inline void DRAW_CHAR_AT(U32 col, U32 row, U8 ch, VBE_PIXEL_COLOUR fg, VBE_PIXEL_COLOUR bg) {
    // Fast path: draw a single-character span with provided colours
    DRAW_STRING_AT(col, row, &ch, 1, fg, bg);
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
void RESTORE_CURSOR_UNDERNEATH(U32 col, U32 row) {
    REDRAW_CHAR(col, row);
}

VOID PUT_CURRENT_PATH(VOID) {
    PUTS("/");
    return;
}

U0 SET_CURSOR_VISIBLE(BOOLEAN visible) {
    cursor.CURSOR_VISIBLE = visible;
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
    if(!cursor.CURSOR_VISIBLE) return;
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
    }
    // Redraw affected region in one go
    U32 span_len = cursor.COLUMNS - col;
    DRAW_STRING_AT(col, row, &text_buffer[row][col], span_len, cursor.fgColor, cursor.bgColor);
    // the cell at [row][col] will be overwritten by caller
}

// Shift contents left from col (inclusive) removing the char at col.
// last column becomes space.
VOID MOVE_BUFFER_CONTENTS_LEFT_FROM(U32 col, U32 row) {
    if (col >= cursor.COLUMNS) return;
    for (U32 c = col; c < cursor.COLUMNS - 1; c++) {
        text_buffer[row][c] = text_buffer[row][c + 1];
    }
    text_buffer[row][cursor.COLUMNS - 1] = ' ';
    // Redraw affected region once
    U32 span_len = cursor.COLUMNS - col;
    DRAW_STRING_AT(col, row, &text_buffer[row][col], span_len, cursor.fgColor, cursor.bgColor);
}

VOID REDRAW_CURRENT_LINE() {
    if(shndl->state == STATE_CMD_INTERFACE) {
        // Clear that text row visually
        for (U32 c = 0; c < cursor.COLUMNS; c++) {
            text_buffer[cursor.Row][c] = ' ';
        }
    
        // Copy from current_line into text_buffer for drawing
        U32 len = STRLEN(current_line);
        for (U32 c = 0; c < len && c < cursor.COLUMNS; c++) {
            text_buffer[cursor.Row][c] = current_line[c];
        }
    
        // Redraw the whole current line in one span
        DRAW_STRING_AT(0, cursor.Row, &text_buffer[cursor.Row][0], cursor.COLUMNS, cursor.fgColor, cursor.bgColor);
    } else {
        // Ensure we always redraw on the prompt row (no vertical movement while editing)
        U32 r = prompt_row;

        // Clear the editable region (from prompt_length to end-of-line)
        for (U32 c = prompt_length; c < cursor.COLUMNS; c++) {
            text_buffer[r][c] = ' ';
        }

        // Copy from current_line into text_buffer starting at prompt_length
        U32 len = STRLEN(current_line);
        if (len > (cursor.COLUMNS - prompt_length)) {
            len = cursor.COLUMNS - prompt_length;
        }
        for (U32 i = 0; i < len; i++) {
            text_buffer[r][prompt_length + i] = current_line[i];
        }

        // Redraw only the affected region as one span
        DRAW_STRING_AT(prompt_length, r, &text_buffer[r][prompt_length], cursor.COLUMNS - prompt_length, cursor.fgColor, cursor.bgColor);

        // Ensure screen cursor is synced to edit_pos
        cursor.Row = r;
        cursor.Column = prompt_length + edit_pos;
        DRAW_CURSOR_AT(cursor.Column, cursor.Row);
    }
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

    // Draw cursor at new position immediately
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}



VOID DELETE_CHAR_AT_CURSOR() {
    RESTORE_CURSOR_AT(cursor.Column, cursor.Row);

    // remove char under cursor by shifting left from cursor.Column
    MOVE_BUFFER_CONTENTS_LEFT_FROM(cursor.Column, cursor.Row);

    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}


VOID PROCESS_PENDING_SCROLLS(VOID) {
    U32 now = GET_PIT_TICKS();
    if (pending_scrolls == 0) return;

    // Only redraw at most once per SCROLL_PROCESS_INTERVAL_MS
    if ((now - last_scroll_process_tick) < SCROLL_PROCESS_INTERVAL_MS) {
        return;
    }
    last_scroll_process_tick = now;

    // If many scrolls accumulated, we already advanced text_buffer accordingly,
    // we just need to update the screen once to match the buffer.
    // So perform a single redraw of the whole text buffer OR (better) redraw visible region.
    DRAW_TEXT_BUFFER();   // cheaper frequency => acceptable
    pending_scrolls = 0;
}


VOID BLINK_CURSOR(VOID) {
    static BOOLEAN visible = TRUE;
    static U32 last_toggle_tick = 0;
    U32 current_tick = GET_PIT_TICKS();

    PROCESS_PENDING_SCROLLS();

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
    // Put path and prompt chars (this moves cursor.Column)
    PUT_CURRENT_PATH();
    PUTS(PATH_END_CHAR);

    // Store where editable area starts (and which row it's on)
    prompt_length = cursor.Column;
    prompt_row = cursor.Row;

    // initialize edit buffer position
    edit_pos = 0;
    MEMZERO(current_line, CUR_LINE_MAX_LENGTH);

    // Ensure cursor is at the editable start
    cursor.Column = prompt_length;
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
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

    // Handle tab (4 spaces) - batch insert spaces
    if (c == '\t') {
        U32 spaces = 4 - (cursor.Column % 4);
        if (spaces) {
            if (cursor.INSERT_MODE) {
                for (U32 i = 0; i < spaces; i++) {
                    MOVE_BUFFER_CONTENTS_RIGHT_FROM(cursor.Column, cursor.Row);
                }
            }
            U8 sp[8];
            for (U32 i = 0; i < spaces && i < 8; i++) sp[i] = ' ';
            WRITE_SPAN_FAST(sp, spaces);
        }
        return 1;
    }

    // Normal character
    if (cursor.INSERT_MODE) {
        // Make room at current cursor position
        MOVE_BUFFER_CONTENTS_RIGHT_FROM(cursor.Column, cursor.Row);
    }

    // Fast path draw for a single character
    text_buffer[cursor.Row][cursor.Column] = c;
    DRAW_STRING_AT(cursor.Column, cursor.Row, &text_buffer[cursor.Row][cursor.Column], 1, cursor.fgColor, cursor.bgColor);

    // Advance cursor
    COLUMN_INC();

    // Immediately show cursor at new pos
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);

    return 1;
}
U32 PUTS(U8 *str) {
    if (!str) return 0;
    U32 total = 0;

    // Batch printable spans, handle controls individually
    while (*str) {
        // Handle control characters upfront
        if (*str == '\n') { total += PUTC(*str++); continue; }
        if (*str == '\r') { total += PUTC(*str++); continue; }
        if (*str == '\t') { total += PUTC(*str++); continue; }

        // Accumulate printable run
        const U8 *start = str;
        U32 len = 0;
        while (str[len] && is_printable(str[len])) {
            len++;
        }
        if (len) {
            total += WRITE_SPAN_FAST(start, len);
            str += len;
            continue;
        }

        // Fallback for any other byte
        total += PUTC(*str++);
    }
    return total;
}

VOID DRAW_TEXT_BUFFER(VOID) {
    for (U32 r = 0; r < cursor.ROWS; r++) {
        DRAW_STRING_AT(0, r, &text_buffer[r][0], cursor.COLUMNS, cursor.fgColor, cursor.bgColor);
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
    PUT_SHELL_START();
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

VOID NEW_ROW(VOID) {
    // Move down one row
    ROW_INC();

    // Reset horizontal position
    cursor.Column = 0;

    // If we’re beyond the visible area, scroll up
    if (cursor.Row >= cursor.ROWS) {
        SCROLL_TEXTBUFFER_UP();         // <- implement or call your video driver’s scroll
        cursor.Row = cursor.ROWS - 1;
    }

    // Move the hardware/text cursor
    HANDLE_LE_CURSOR();
}



VOID HANDLE_LE_ENTER() {
    // Terminate buffer at edit_pos and execute
    if (edit_pos >= CUR_LINE_MAX_LENGTH) edit_pos = CUR_LINE_MAX_LENGTH - 1;
    current_line[edit_pos] = '\0';
    RESTORE_CURSOR_BEFORE_MOVE();
    HANDLE_COMMAND(current_line);   // parse/execute
    PUSH_TO_HISTORY(current_line);   // save to history
    
    // Clear current_line and move to next row; reinitialize prompt
    MEMZERO(current_line, CUR_LINE_MAX_LENGTH);
    edit_pos = 0;

    // Move to new line and place prompt
    RESTORE_CURSOR_BEFORE_MOVE();
    cursor.Column = 0;
    PUT_SHELL_START();

    history_index = 0; // reset browsing state
}

VOID HANDLE_LE_BACKSPACE() {
    if (edit_pos == 0) return; // don't delete before prompt
    edit_pos--;

    // shift left from edit_pos
    U32 len = STRLEN(current_line);
    for (U32 i = edit_pos; i < len; i++) {
        current_line[i] = current_line[i + 1];
    }
    RESTORE_CURSOR_BEFORE_MOVE();
    REDRAW_CURRENT_LINE();
}

VOID HANDLE_LE_DELETE() {
    U32 len = STRLEN(current_line);
    if (edit_pos >= len) return; // nothing to delete

    for (U32 i = edit_pos; i < len; i++) {
        current_line[i] = current_line[i + 1];
    }
    RESTORE_CURSOR_BEFORE_MOVE();

    REDRAW_CURRENT_LINE();
}

VOID HANDLE_LE_ARROW_LEFT() {
    if (edit_pos == 0) return; // prevent moving into prompt
    edit_pos--;
    cursor.Column = prompt_length + edit_pos;
    RESTORE_CURSOR_BEFORE_MOVE();
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}

VOID HANDLE_LE_ARROW_RIGHT() {
    U32 len = STRLEN(current_line);
    if (edit_pos < len) {
        edit_pos++;
        cursor.Column = prompt_length + edit_pos;
        RESTORE_CURSOR_BEFORE_MOVE();
        DRAW_CURSOR_AT(cursor.Column, cursor.Row);
    }
}

VOID HANDLE_LE_CTRL_C() {
    // Draw "^C" to visually show the interrupt
    PUTS("^C\n");

    // Reset the current input line
    MEMZERO(current_line, CUR_LINE_MAX_LENGTH);
    edit_pos = 0;

    // If a process was running, mark it as interrupted
    if (shndl == STATE_CMD_INTERFACE) {
        // TERMINATE_FOCUSED_PROCESS(); // <-- implement this if your PROC system supports it
    }

    // Return shell to line editing mode
    shndl = STATE_EDIT_LINE;
    history_index = 0;

    // Display prompt again
    PUT_SHELL_START();
}

VOID HANDLE_LE_DEFAULT(KEYPRESS *kp, MODIFIERS *mod) {
    if(kp->keycode == KEY_C && mod->ctrl) {
        HANDLE_LE_CTRL_C();
        return;
    }
    U8 c = keypress_to_char(kp->keycode);
    if (!c) return;

    U32 len = STRLEN(current_line);
    if (len >= CUR_LINE_MAX_LENGTH - 1) return;

    // Make room at edit_pos
    for (U32 i = len + 1; i > edit_pos; i--) {
        current_line[i] = current_line[i - 1];
    }

    current_line[edit_pos] = c;
    edit_pos++;
    RESTORE_CURSOR_BEFORE_MOVE();

    REDRAW_CURRENT_LINE();
}

VOID HANDLE_LE_CURSOR() {
    // Sync screen cursor with edit_pos
    cursor.Row = prompt_row;
    cursor.Column = prompt_length + edit_pos;
    RESTORE_CURSOR_BEFORE_MOVE();
    DRAW_CURSOR_AT(cursor.Column, cursor.Row);
}

VOID HANDLE_LE_ARROW_UP() {
    if (history_count == 0) return;

    // If we're at newest line, start browsing from most recent entry
    if (history_index < history_count)
        history_index++;
    else
        history_index = history_count; // clamp

    // Copy the corresponding line from history
    const U8 *line = line_history[history_count - history_index];
    STRNCPY(current_line, line, CUR_LINE_MAX_LENGTH);
    edit_pos = STRLEN(current_line);

    REDRAW_CURRENT_LINE();
    HANDLE_LE_CURSOR();
}

VOID HANDLE_LE_ARROW_DOWN() {
    if (history_count == 0) return;
    if (history_index == 0) return; // already at current line

    history_index--;

    if (history_index == 0) {
        // Return to blank current line
        MEMZERO(current_line, CUR_LINE_MAX_LENGTH);
        edit_pos = 0;
    } else {
        const U8 *line = line_history[history_count - history_index];
        STRNCPY(current_line, line, CUR_LINE_MAX_LENGTH);
        edit_pos = STRLEN(current_line);
    }

    REDRAW_CURRENT_LINE();
    HANDLE_LE_CURSOR();
}

U0 LE_CLS(U0) {
    // Clear physical screen
    CLEAR_SCREEN_COLOUR(cursor.bgColor);

    // Clear text buffer and redraw (optional; could skip redraw until prompt)
    CLEAR_TEXTBUFFER();
    // DRAW_TEXT_BUFFER();

    // Reset editing state
    MEMZERO(current_line, CUR_LINE_MAX_LENGTH);
    edit_pos = 0;
    history_index = 0;

    // Reset cursor position
    cursor.Row = 0;
    cursor.Column = 0;
    // Print prompt at top
    PUT_SHELL_START();
}
