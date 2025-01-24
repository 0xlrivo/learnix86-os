#include "../inc/string.h"

char* strcpy (char* destination, const char* source) {
    char* curr = destination;
    while (*source) {
        *curr++ = *source++; 
    };
    return destination;
}