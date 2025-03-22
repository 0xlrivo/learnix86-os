#ifndef LEARNIX_LIBC_STRING_H
#define LEARNIX_LIBC_STRING_H

#include <stdint.h>

int memcmp(const void*, const void*, uint32_t);

void* memcpy(void* restrict dst, const void* restrict src, uint32_t size);

void* memmove(void*, const void*, uint32_t);

void *memset(void *ptr, int value, uint32_t num);

uint32_t strlen(const char*);

#endif