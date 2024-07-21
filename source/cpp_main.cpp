#include <iostream>
#include "main.h"

extern "C" void cpp_call(int argc, char const *argv[]){
    std::cout << "cpp_call, args from main.c:" << std::endl; 
    
    for(int i = 0; i < argc; i++){
        std::cout << argv[i] << std::endl;
    }
}