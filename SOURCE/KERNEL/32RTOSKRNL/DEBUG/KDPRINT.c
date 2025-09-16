/*+++
    SOURCE/KERNEL/32RTOSKRNL/DEBUG/KDPRINT.c - Kernel Debug Print Implementation

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    Minimal serial driver used to dump kernel debug strings to the
    QEMU console (via COM1 / -serial stdio).

AUTHORS
    ChatGPT Codex Agent

REVISION HISTORY
    2025/09/07 - ChatGPT Codex Agent
        Initial version.
REMARKS
    Configure QEMU with `-serial stdio` to view kdprint output.
---*/
#include "./KDPRINT.h"
#include "../../../../STD/ASM.h"

#define SERIAL_COM1_PORT 0x3F8
#define SERIAL_DATA      (SERIAL_COM1_PORT)
#define SERIAL_INT_ENABLE (SERIAL_COM1_PORT + 1)
#define SERIAL_FIFO_CTRL  (SERIAL_COM1_PORT + 2)
#define SERIAL_LINE_CTRL  (SERIAL_COM1_PORT + 3)
#define SERIAL_MODEM_CTRL (SERIAL_COM1_PORT + 4)
#define SERIAL_LINE_STATUS (SERIAL_COM1_PORT + 5)

#define SERIAL_LINE_STATUS_THRE 0x20

static BOOL g_SerialReady = FALSE;

static inline U0 serial_outb(U16 port, U8 value) {
    outb(port, value);
}

static inline U8 serial_inb(U16 port) {
    U8 value = 0;
    inb(port, value);
    return value;
}

static inline BOOL serial_transmit_empty(U0) {
    return (serial_inb(SERIAL_LINE_STATUS) & SERIAL_LINE_STATUS_THRE) != 0;
}

U0 kdprint_init(U0) {
    if(g_SerialReady) {
        return;
    }
    serial_outb(SERIAL_INT_ENABLE, 0x00);   // Disable interrupts
    serial_outb(SERIAL_LINE_CTRL, 0x80);    // Enable DLAB
    serial_outb(SERIAL_DATA, 0x03);         // Divisor low byte (38400 baud)
    serial_outb(SERIAL_INT_ENABLE, 0x00);   // Divisor high byte
    serial_outb(SERIAL_LINE_CTRL, 0x03);    // 8 bits, no parity, one stop
    serial_outb(SERIAL_FIFO_CTRL, 0xC7);    // Enable FIFO, clear, 14-byte threshold
    serial_outb(SERIAL_MODEM_CTRL, 0x0B);   // IRQs enabled, RTS/DSR set
    g_SerialReady = TRUE;
}

static inline U0 serial_write_char(CHAR c) {
    if(!g_SerialReady) {
        return;
    }
    while(!serial_transmit_empty()) {
        // spin until transmitter ready
    }
    serial_outb(SERIAL_DATA, (U8)c);
}

U0 kdprint_char(CHAR ch) {
    if(!g_SerialReady) {
        kdprint_init();
    }
    if(ch == '\n') {
        serial_write_char('\r');
    }
    serial_write_char(ch);
}

U0 kdprint(const char* message) {
    if(message == NULLPTR) {
        return;
    }

    if(!g_SerialReady) {
        kdprint_init();
    }

    for(const char* ptr = message; *ptr != '\0'; ++ptr) {
        CHAR ch = (CHAR)(*ptr);
        if(ch == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(ch);
    }
}

U0 kdprint_hex(U8 value) {
    static const char* hex = "0123456789ABCDEF";
    char buffer[3];
    buffer[0] = hex[(value >> 4) & 0x0F];
    buffer[1] = hex[value & 0x0F];
    buffer[2] = '\0';
    kdprint(buffer);
}
