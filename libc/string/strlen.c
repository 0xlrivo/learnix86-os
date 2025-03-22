#include <string.h>

uint32_t strlen(const char* str) {
    uint32_t i;
    for(i = 0; str[i] != '\0'; i++);
    return i;
}