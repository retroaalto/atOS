#include <ERROR/ERROR.h>
static U32 error_code __attribute__((section(".data"))) = ERROR_NONE;

U32 GET_LAST_ERROR() {
    return error_code;
}

void SET_ERROR_CODE(U32 code) {
    error_code = code;
}

void CLEAR_ERROR_CODE() {
    error_code = ERROR_NONE;
}