#include "../inc/learnixlibc.h"

size_t strlen(const char* str) {
    size_t i;
    for(i = 0; str[i] != '\0'; i++);
    return i;
}

void *memcopy(void *to, void *from, size_t n) {
    char *to_ = (char*)to;
    char *from_ = (char*)from;
    for(size_t i = 0; i < n; i++) {
        to_[i] = from_[i];
    }
}