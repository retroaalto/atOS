#include "stdio.h"
#include "stdlib.h"
#include "main.h"

int main(int argc, char const *argv[]){
    printf("running functions!\n");

    const char *str = "teststring";
    print_message(str);

    cpp_call(argc, argv);
    return 0;
}
