#include <DRIVERS/PS2/KEYBOARD.h>
#include <CPU/PIC.h>
#include <DRIVERS/VIDEO/VBE.h>
#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <CPU/ISR/ISR.h>

static CMD_QUEUE cmd_queue;
static PS2_INFO ps2_info = {0};

PS2_INFO *GET_PS2_INFO(VOID) {
    return &ps2_info;
}

BOOLEAN IS_CMD_QUEUE_EMPTY(VOID) { 
    return cmd_queue.head == cmd_queue.tail; 
}
void PUSH_TO_CMD_QUEUE(U8 byte) {
    U8 next = (cmd_queue.head + 1) % CMD_QUEUE_SIZE;
    if(next == cmd_queue.tail) {
        return;
    }
    cmd_queue.buffer[cmd_queue.head] = byte;
    cmd_queue.head = next;
}

CMD_QUEUE *GET_CMD_QUEUE(VOID) { 
    return &cmd_queue; 
}
U8 POP_FROM_CMD_QUEUE(VOID) {
    if (IS_CMD_QUEUE_EMPTY()) {
        return 0; // Queue is empty
    }
    U8 byte = cmd_queue.buffer[cmd_queue.tail];
    cmd_queue.tail = (cmd_queue.tail + 1) % CMD_QUEUE_SIZE;
    return byte;
}
BOOLEAN IS_CMD_QUEUE_FULL(VOID) {
    return (cmd_queue.head + 1) % (U8)CMD_QUEUE_SIZE == cmd_queue.tail;
}
U0 CLEAR_CMD_QUEUE(VOID) {
    cmd_queue.head = 0;
    cmd_queue.tail = 0;
}

U8 PS2_INB(U8 port) {
    U8 status = _inb(port);
    return status;
}




void PS2_KEYBOARD_HANDLER(I32 num, U32 errcode) {
    (void)num; (void)errcode; // not used
    U8 scancode = PS2_INB(PS2_DATAPORT);
    PUSH_TO_CMD_QUEUE(scancode);

    VBE_DRAW_ELLIPSE(300, 300, 50, 25, VBE_AQUA);
    VBE_UPDATE_VRAM();
}




BOOLEAN PS2_WAIT_FOR_INPUT_CLEAR(VOID) {
    int timeout = 100000;
    while(PS2_INB(PS2_CMDPORT) & PS2_INPUT_BUFFER_FULL && --timeout);
    if(timeout == 0) return FALSE;
    return TRUE;
}

BOOLEAN PS2_ACKNOWLEDGED() {
    if(!PS2_WAIT_FOR_INPUT_CLEAR()) return FALSE;
    U8 status = PS2_INB(PS2_DATAPORT);
    if(status == PS2_SPECIAL_RESEND) return FALSE;
    if(status != PS2_SPECIAL_ACK) return FALSE;
    return TRUE;
}

BOOLEAN PS2_OUTB(U8 port, U8 data) {
    if(!PS2_WAIT_FOR_INPUT_CLEAR()) return FALSE;
    _outb(port, data);
    U8 status = PS2_INB(PS2_DATAPORT);
    if(status == PS2_SPECIAL_RESEND) {
        for(U8 i = 0; i < 3; i++) {
            if(!PS2_WAIT_FOR_INPUT_CLEAR()) return FALSE;
            _outb(port, data);
            status = PS2_INB(PS2_DATAPORT);
            if(status == PS2_SPECIAL_ACK) break;
            if(status != PS2_SPECIAL_RESEND) return FALSE;
        }
    } else {
        if(status != PS2_SPECIAL_ACK) return FALSE;
    }
    if(!PS2_WAIT_FOR_INPUT_CLEAR()) return FALSE;
    return TRUE;
}

BOOLEAN PS2_SET_SCAN_CODE_SET(U8 scan_code_set) {
    if(!PS2_OUTB(PS2_WRITEPORT, PS2_SET_SCANCODE_SET)) return FALSE;
    if(!PS2_OUTB(PS2_DATAPORT, scan_code_set)) return FALSE;
    return TRUE;
}

BOOLEAN PS2_KEYBOARD_RETRY_TRY(VOID) {
    if(!PS2_OUTB(PS2_DATAPORT, PS2_RESET_AND_SELF_TEST)) return FALSE;
    if(PS2_INB(PS2_DATAPORT) != PS2_SPECIAL_SELF_TEST_PASSED) return FALSE;
    return TRUE;
}

BOOLEAN PS2_KEYBOARD_RESET(VOID) {
    for(U8 i = 0; i < 3; i++) {
        PS2_WAIT_FOR_INPUT_CLEAR();
        if(PS2_KEYBOARD_RETRY_TRY()) {
            return TRUE; // Success
        }
    }
    return FALSE; // All attempts failed
}

U0 PS2_EMPTY_KEYBOARD_BUFFER(VOID) {
    while((PS2_INB(PS2_DATAPORT) & PS2_OUTPUT_BUFFER_FULL));
}

BOOLEAN PS2_EnableScanning(VOID) {
    if(!PS2_OUTB(PS2_DATAPORT, PS2_ENABLE_SCANNING)) {
        return FALSE;
    }
    if(!PS2_ACKNOWLEDGED()) {
        return FALSE;
    }
    return TRUE;
}

BOOLEAN PS2_DisableScanning(VOID) {
    if(!PS2_OUTB(PS2_DATAPORT, PS2_DISABLE_SCANNING)) {
        return FALSE;
    }
    if(!PS2_ACKNOWLEDGED()) {
        return FALSE;
    }
    return TRUE;
}

U16 PS2_Identify(VOID) {
    if(!PS2_OUTB(PS2_DATAPORT, PS2_IDENTIFY_KEYBOARD)) {
        return 0; // Failed to send identify command
    }
    U8 first_byte = PS2_INB(PS2_DATAPORT);
    if(first_byte == 0xAB) {
        U8 second_byte = PS2_INB(PS2_DATAPORT);
        return first_byte << 8 | second_byte;
    } else {
        return first_byte; // Unknown device
    }
}

// #define PS2_TEST
BOOLEAN PS2_KEYBOARD_INIT(VOID) {
    #ifndef PS2_TEST
    PIC_Mask(1);
    CLI;
    // Initialize USB controllers

    if(!PS2_DisableScanning()) return FALSE;
    
    U16 PS2_type = PS2_Identify();
    ps2_info.type = PS2_type;
    ps2_info.dual_channel = FALSE;
    ps2_info.exists = TRUE;

    // Disable devices
    _outb(PS2_CMDPORT, DISABLE_FIRST_PS2_PORT);
    _outb(PS2_CMDPORT, DISABLE_SECOND_PS2_PORT);

    // Set up controller configuration byte
    _outb(PS2_CMDPORT, GET_CONTROLLER_CONFIGURATION_BYTE);
    U8 config1 = _inb(PS2_DATAPORT);
    config1 &= ~0b01010010; // Disable interrupts, translation and clock signal
    _outb(PS2_CMDPORT, SET_CONTROLLER_CONFIGURATION_BYTE);  // send “write config byte” command
    _outb(PS2_WRITEPORT, config1);  // write value

    // Test whether there are 2 or 1 channels
    _outb(PS2_CMDPORT, ENABLE_SECOND_PS2_PORT);
    _outb(PS2_CMDPORT, GET_CONTROLLER_CONFIGURATION_BYTE);
    U8 config2 = _inb(PS2_DATAPORT);
    // If bit 5 is clear, it exists
    if((config2 & 0b00100000) == 0) {
        ps2_info.dual_channel = TRUE;
        config2 &= ~0b00100010; // Clear bit 5 and 1 to disable
        _outb(PS2_CMDPORT, SET_CONTROLLER_CONFIGURATION_BYTE);
        _outb(PS2_WRITEPORT, config2);
    }

    // Perform inteface test
    _outb(PS2_CMDPORT, TEST_FIRST_PS2_PORT);
    ps2_info.port1_check = PS2_INB(PS2_DATAPORT);
    if(ps2_info.dual_channel) {
        _outb(PS2_CMDPORT, TEST_SECOND_PS2_PORT);
        ps2_info.port2_check = PS2_INB(PS2_DATAPORT);
    } else {
        ps2_info.port2_check = 0;
    }

    // Enable first PS/2 port
    _outb(PS2_CMDPORT, ENABLE_FIRST_PS2_PORT);
    config1 |= 0b00000001; // Enable interrupt for first PS/2 port
    _outb(PS2_CMDPORT, SET_CONTROLLER_CONFIGURATION_BYTE);
    _outb(PS2_WRITEPORT, config1);

    // Enable second PS/2 port if exists
    if(ps2_info.dual_channel) {
        _outb(PS2_CMDPORT, ENABLE_SECOND_PS2_PORT);
        config2 |= 0b00000010; // Enable interrupt for second PS/2 port
        _outb(PS2_CMDPORT, SET_CONTROLLER_CONFIGURATION_BYTE);
        _outb(PS2_WRITEPORT, config2);
    }
    
    if(!PS2_KEYBOARD_RESET()) return FALSE;
    
    if(!PS2_WAIT_FOR_INPUT_CLEAR()) return FALSE;
    
    
    if(!PS2_SET_SCAN_CODE_SET(DEFAULT_SCANCODESET)) {
        if(!PS2_SET_SCAN_CODE_SET(SECONDARY_SCANCODESET)) {
            return FALSE; // Set scan code set failed
        }
    }

    PS2_EMPTY_KEYBOARD_BUFFER(); // Clear keyboard buffer

    CLEAR_CMD_QUEUE();
    #endif

    ISR_REGISTER_HANDLER(PIC_REMAP_OFFSET + 1, PS2_KEYBOARD_HANDLER);
    PIC_Unmask(1);
    #ifndef PS2_TEST
    // enable scanning
    if(!PS2_EnableScanning()) return FALSE;
    STI;
    #endif
    return TRUE;
}




Keys1 ParsePS2_CS1(U8 scancode) {
    switch (scancode) {
        case 0x01: return SC1_ESC;
        default: return 0; // Unknown key
    }
}

Keys2 ParsePS2_CS2(U8 scancode) {
    switch (scancode) {
        case 0x1C: return SC2_A;
        case 0x32: return SC2_B;
        case 0x21: return SC2_C;
        default: return 0; // Unknown key
    }
}