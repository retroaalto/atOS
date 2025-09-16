/*+++
    SOURCE/KERNEL/32RTOSKRNL/DRIVERS/INPUT/KEYBOARD.c - PS/2 Keyboard Driver

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Interrupt-driven PS/2 keyboard handler that translates scancodes
    to ASCII values and buffers them for consumers.

AUTHORS
    ChatGPT Codex Agent

REVISION HISTORY
    2025/09/07 - ChatGPT Codex Agent
        Initial version.
REMARKS
    Currently supports set-1 scancode translation with Shift/Caps Lock.
---*/
#include "./KEYBOARD.h"
#include "../../CPU/IRQ/IRQ.h"
#include "../../DEBUG/KDPRINT.h"
#include "../../../../STD/ASM.h"

#define KBD_DATA_PORT      0x60
#define KBD_CMD_PORT       0x64
#define KBD_STATUS_PORT    0x64
#define KBD_STATUS_OBF     0x01
#define KBD_STATUS_IBF     0x02

#define KBD_CMD_ENABLE_FIRST_PORT 0xAE
#define KBD_CMD_DISABLE_FIRST_PORT 0xAD
#define KBD_CMD_READ_CFG   0x20
#define KBD_CMD_WRITE_CFG  0x60
#define KBD_CMD_ENABLE_SCAN 0xF4
#define KBD_CFG_IRQ1       0x01

#define ASCII_CASE_DIFF    ('a' - 'A')

#if (KBD_BUFFER_SIZE & (KBD_BUFFER_SIZE - 1u)) != 0
#error "KBD_BUFFER_SIZE must be a power of two"
#endif

#define KBD_BUFFER_MASK    (KBD_BUFFER_SIZE - 1u)

static CHAR g_KeyBuffer[KBD_BUFFER_SIZE];
static VOLATILE U8 g_BufferHead = 0;
static VOLATILE U8 g_BufferTail = 0;
static VOLATILE BOOL g_ShiftActive = FALSE;
static VOLATILE BOOL g_CapsLock = FALSE;
static VOLATILE BOOL g_ExpectExtended = FALSE;

static const CHAR g_ScancodeMap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, '7',
    '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const CHAR g_ScancodeMapShift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, '7',
    '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static inline U8 kbd_next_index(U8 index) {
    return (U8)((index + 1u) & KBD_BUFFER_MASK);
}

static inline BOOL kbd_buffer_full(U0) {
    return kbd_next_index(g_BufferHead) == g_BufferTail;
}

static U0 kbd_handle_scancode(U8 scancode);
static U0 kbd_process_pending(BOOL log_event);

static inline U0 kbd_wait_input_clear(U0) {
    U8 status = 0;
    do {
        inb(KBD_STATUS_PORT, status);
    } while(status & KBD_STATUS_IBF);
}

static inline U0 kbd_wait_output_full(U0) {
    U8 status = 0;
    do {
        inb(KBD_STATUS_PORT, status);
    } while(!(status & KBD_STATUS_OBF));
}

static inline U8 kbd_controller_read_cfg(U0) {
    U8 cfg = 0;
    kbd_wait_input_clear();
    outb(KBD_CMD_PORT, KBD_CMD_READ_CFG);
    kbd_wait_output_full();
    inb(KBD_DATA_PORT, cfg);
    return cfg;
}

static inline U0 kbd_controller_write_cfg(U8 cfg) {
    kbd_wait_input_clear();
    outb(KBD_CMD_PORT, KBD_CMD_WRITE_CFG);
    kbd_wait_input_clear();
    outb(KBD_DATA_PORT, cfg);
}

static inline U0 kbd_controller_send(U8 cmd) {
    kbd_wait_input_clear();
    outb(KBD_DATA_PORT, cmd);
}

static U0 kbd_buffer_push(CHAR key) {
    if(key == 0) {
        return;
    }

    U8 next = kbd_next_index(g_BufferHead);
    if(next == g_BufferTail) {
        return; // drop when buffer is full
    }

    g_KeyBuffer[g_BufferHead] = key;
    g_BufferHead = next;
}

static BOOL kbd_try_read_scancode(U8 *code) {
    U8 status = 0;
    inb(KBD_STATUS_PORT, status);
    if(!(status & KBD_STATUS_OBF)) {
        return FALSE;
    }

    U8 value = 0;
    inb(KBD_DATA_PORT, value);
    *code = value;
    return TRUE;
}

static U0 kbd_drain_output(U0) {
    U8 status = 0;
    U8 discard = 0;

    for(;;) {
        inb(KBD_STATUS_PORT, status);
        if(!(status & KBD_STATUS_OBF)) {
            break;
        }
        inb(KBD_DATA_PORT, discard);
    }
}

static U0 keyboard_isr(I32 vector, U32 errcode) {
    (void)vector;
    (void)errcode;
    kbd_process_pending(TRUE);
}

U0 KBD_INIT(U0) {
    g_BufferHead = 0;
    g_BufferTail = 0;
    g_ShiftActive = FALSE;
    g_CapsLock = FALSE;
    g_ExpectExtended = FALSE;

    kbd_wait_input_clear();
    outb(KBD_CMD_PORT, KBD_CMD_DISABLE_FIRST_PORT);

    kbd_drain_output();

    U8 cfg = kbd_controller_read_cfg();
    cfg |= KBD_CFG_IRQ1;
    kbd_controller_write_cfg(cfg);

    kbd_wait_input_clear();
    outb(KBD_CMD_PORT, KBD_CMD_ENABLE_FIRST_PORT);

    kbd_controller_send(KBD_CMD_ENABLE_SCAN);
    kbd_wait_output_full(); // consume ACK from device (0xFA)
    U8 ack = 0;
    inb(KBD_DATA_PORT, ack);

    IRQ_REGISTER_HANDLER(IRQ_KEYBOARD, keyboard_isr);
    IRQ_CLEAR_MASK(IRQ_KEYBOARD);
}

BOOL KBD_HAS_KEY(U0) {
    return g_BufferHead != g_BufferTail;
}

CHAR KBD_PEEK_KEY(U0) {
    if(!KBD_HAS_KEY()) {
        return 0;
    }

    return g_KeyBuffer[g_BufferTail];
}

CHAR KBD_READ_KEY(U0) {
    if(!KBD_HAS_KEY()) {
        return 0;
    }

    CHAR value = g_KeyBuffer[g_BufferTail];
    g_BufferTail = kbd_next_index(g_BufferTail);
    return value;
}

U0 KBD_POLL(U0) {
    kbd_process_pending(FALSE);
}

static U0 kbd_process_pending(BOOL log_event) {
    U8 scancode = 0;
    while(kbd_try_read_scancode(&scancode)) {
        if(log_event) {
            kdprint("IRQ1 scancode: ");
            kdprint_hex(scancode);
            kdprint("\n");
        }
        kbd_handle_scancode(scancode);
    }
}

static U0 kbd_handle_scancode(U8 scancode) {
    if(scancode == 0xE0) {
        g_ExpectExtended = TRUE;
        return;
    }

    if(g_ExpectExtended) {
        g_ExpectExtended = FALSE;
        return; // ignore extended scancodes for now
    }

    if(scancode & 0x80u) {
        U8 make = (U8)(scancode & 0x7Fu);
        if(make == 0x2A || make == 0x36) {
            g_ShiftActive = FALSE;
        }
        return;
    }

    if(scancode == 0x2A || scancode == 0x36) {
        g_ShiftActive = TRUE;
        return;
    }

    if(scancode == 0x3A) {
        g_CapsLock = !g_CapsLock;
        return;
    }

    CHAR translated = 0;
    if(scancode < 128) {
        CHAR base = g_ScancodeMap[scancode];
        CHAR shifted = g_ScancodeMapShift[scancode];
        BOOL shift = g_ShiftActive;

        translated = shift ? shifted : base;

        if(base >= 'a' && base <= 'z') {
            BOOL uppercase = (shift ^ g_CapsLock) ? TRUE : FALSE;
            translated = uppercase ? shifted : base;
        }
    }

    kbd_buffer_push(translated);
}
