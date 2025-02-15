#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

int memcmp(const void*, const void*, size_t);

void* memcpy(void* restrict dst, const void* restrict src, size_t size);

void* memmove(void*, const void*, size_t);

void *memset(void *ptr, int value, size_t num);

size_t strlen(const char*);

#endif