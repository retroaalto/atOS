#ifndef MAIN_H
#define MAIN_H

#ifdef DEBUG
    #include <stdio.h>
    const bool not___defined___ = bool( printf("Debug mode\n") );
#endif

#define VERSION         "0.0.1"

#ifdef __cplusplus
extern "C" {
#endif

void print_message(const char *str);
void cpp_call(int argc, char const *argv[]);

#ifdef __cplusplus
}
#endif


#endif // MAIN_H