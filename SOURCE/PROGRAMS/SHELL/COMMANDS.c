#include "COMMANDS.h"
#include <PROGRAMS/SHELL/SHELL.h>
#include <STD/STRING.h>

#define CMP_CMD(cmd, str) (STRNCMP((U8*)cmd, (U8*)str, STRLEN(str)) == TRUE)

U0 HANDLE_COMMAND(U8 *line) {
    OutputHandle cursor = GetOutputHandle();
    cursor->CURSOR_VISIBLE = FALSE;
    // For now, just echo the command back
    if(CMP_CMD(line, "help")) {
        PRINTNEWLINE();
        PUTS((U8*)"Available commands:"LEND);
        PUTS((U8*)"help - Show this help message" LEND);
        PUTS((U8*)"clear - Clear the screen" LEND);
        PUTS((U8*)"cls - Clear the screen" LEND);
        PUTS((U8*)"echo [text] - Echo the provided text" LEND);
        PUTS((U8*)"exit - Exit the shell" LEND);
        PUTS((U8*)"version - Show shell version" LEND);
    } else if(CMP_CMD(line, "clear") || CMP_CMD(line, "cls")) {
        LE_CLS();
    } else if(CMP_CMD(line, "version")) {
        PRINTNEWLINE();
        PUTS((U8*)"atOS Shell version 1.0.0" LEND);
    } else if(CMP_CMD(line, "exit")) {
        PRINTNEWLINE();
        PUTS((U8*)"Exiting shell..." LEND);
        // In a real shell, you would terminate the shell process here
        // For now, just print a message
    } else if(CMP_CMD(line, "echo ")) {
        // Echo the rest of the line after "echo "
        PRINTNEWLINE();
        U32 echo_start = STRLEN("echo ");
        if(STRLEN(line) > echo_start) {
            PUTS(&line[echo_start]);
            PRINTNEWLINE();
        } else {
            PRINTNEWLINE();
        }
    } else if(STRLEN(line) == 0) {
        //
        PRINTNEWLINE();
    } else {
        PRINTNEWLINE();
        PUTS((U8*)"Unknown command: ");
        PUTS(line);
        PRINTNEWLINE();
    }
    // Move to new line and place prompt
    cursor->CURSOR_VISIBLE = TRUE;
}