#include <stdio.h>
#include <stdint.h>

void test(void);

void test(void) {
    return;
}

void test2(uint32_t *handler) {
    printf("%p\n", handler);
}

int main() {
    // uint32_t *val = (uint32_t*)&test;
    // printf("%p\n", (uint32_t)&test);
    test2(&test);
    return 0;
}