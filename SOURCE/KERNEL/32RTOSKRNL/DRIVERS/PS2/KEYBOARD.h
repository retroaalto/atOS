/*+++
    SOURCE/KERNEL/32RTOSKRNL/DRIVERS/PS2/KEYBOARD.h - PS/2 Keyboard Driver

    Part of atOS

    Licensed under the MIT License. See LICENSE file in the project root for full license information.

DESCRIPTION
    PS/2 Keyboard Driver

AUTHORS
    Antonako1

REVISION HISTORY
    yyyy/mm/dd - Name(s)
        Description

REMARKS
    When compiling, include KEYBOARD.c
---*/
#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H
#include <DRIVERS/PS2/KEYBOARD.h>
#include <STD/ASM.h>
#include <STD/ATOSMINDEF.h>

/*+++
PS/2 Ports
---*/
#define PS2_DATAPORT 0x60
#define PS2_WRITEPORT 0x60
#define PS2_CMDPORT  0x64

/*+++
PS/2 Status bits
---*/
#define PS2_OUTPUT_BUFFER_FULL  0b00000001 // Data ready
#define PS2_INPUT_BUFFER_FULL   0b00000010 // Controller busy
#define PS2_SYSTEM_FLAG         0b00000100 // System flag
#define PS2_COMMAND_BIT         0b00010000 // Command bit
#define PS2_DATA_BIT            0b00000000 // Data bit

/*+++
PS/2 Keyboard Commands
---*/
#define PS2_SET_LEDS    0xED
#define PS2_ECHO        0xEE
#define PS2_GET_SCANCODE_SET 0xF0
#define PS2_SET_SCANCODE_SET 0xF0
#define PS2_IDENTIFY_KEYBOARD 0xF2
#define PS2_SET_TYPEMATIC_RATE_AND_DELAY 0xF3
#define PS2_ENABLE_SCANNING 0xF4
#define PS2_DISABLE_SCANNING 0xF5
#define PS2_SET_DEF_PARAMETERS 0xF6
#define PS2_SET_ALL_KEYS_TYPEMATIC_AUTOREPEAT 0xF7
#define PS2_SET_ALL_KEYS_MAKE_RELEASE 0xF8
#define PS2_SET_ALL_KEYS_MAKE_ONLY 0xF9
#define PS2_SET_ALL_KEYS_TYPEMATIC_AUTOREPEAT_MAKE_RELEASE 0xFA
#define PS2_SET_SPECIFIC_KEY_TYPEMATIC_AUTORELEASE 0xFB
#define PS2_SET_SPECIFIC_KEY_MAKE_RELEASE_ONLY 0xFC
#define PS2_SET_SPECIFIC_KEY_MAKE_ONLY 0xFD
#define PS2_RESEND_LAST_BYTE 0xFE
#define PS2_RESET_AND_SELF_TEST 0xFF

/*+++
PS/2 Keyboard Special Bytes
---*/
#define PS2_SPECIAL_KEYDETECT_BUFFER_ERROR1 0x00
#define PS2_SPECIAL_SELF_TEST_PASSED 0xAA
#define PS2_SPECIAL_ECHO_RESPONSE 0xEE
#define PS2_SPECIAL_ACK 0xFA // Command acknowledgment
#define PS2_SPECIAL_SELF_TEST_FAILED1 0xFC // After reset or powerup
#define PS2_SPECIAL_SELF_TEST_FAILED2 0xFD // After reset or powerup
#define PS2_SPECIAL_RESEND 0xFE // Repeat last command
#define PS2_SPECIAL_KEYDETECT_BUFFER_ERROR2 0xFF


#define GET_CONTROLLER_CONFIGURATION_BYTE 0x20
#define SET_CONTROLLER_CONFIGURATION_BYTE 0x60
#define ENABLE_SECOND_PS2_PORT 0xA8
#define TEST_FIRST_PS2_PORT 0xAB
#define TEST_SECOND_PS2_PORT 0xA9
#define ENABLE_FIRST_PS2_PORT 0xAE
#define DISABLE_FIRST_PS2_PORT 0xAD
#define DISABLE_SECOND_PS2_PORT 0xA7

typedef struct {
    U16 type;
    BOOLEAN dual_channel;
    BOOLEAN exists;
    U8 port1_check;
    U8 port2_check;
} PS2_INFO;

extern PS2_INFO *PS2_info;

#define CMD_QUEUE_SIZE 8
typedef struct {
    U8 head;
    U8 tail;
    U8 buffer[CMD_QUEUE_SIZE];
} CMD_QUEUE;

PS2_INFO *GET_PS2_INFO(VOID);

CMD_QUEUE *GET_CMD_QUEUE(VOID);
U0 PUSH_TO_CMD_QUEUE(U8 byte);
U8 POP_FROM_CMD_QUEUE(VOID);
BOOLEAN IS_CMD_QUEUE_EMPTY(VOID);
BOOLEAN IS_CMD_QUEUE_FULL(VOID);
U0 CLEAR_CMD_QUEUE(VOID);


BOOLEAN PS2_KEYBOARD_INIT(VOID);
BOOLEAN PS2_KEYBOARD_RESET(VOID);
BOOLEAN PS2_SET_SCAN_CODE_SET(U8 set);
void PS2_KEYBOARD_HANDLER(I32 num, U32 errcode);

/*+++
Scan code set 1
---*/
#define SCANCODESET1 0x01
typedef enum {
    SC1_ESC = 0x01
} Keys1;
/*+++
Scan code set 2
---*/
#define SCANCODESET2 0x02
typedef enum {
    SC2_A = 0x1C,
    SC2_B = 0x32,
    SC2_C = 0x21
} Keys2;

#define DEFAULT_SCANCODESET SCANCODESET2
#define SECONDARY_SCANCODESET SCANCODESET1


#endif // PS2_KEYBOARD_H