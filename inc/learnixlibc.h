/* Learnix86 OS implementation of libc */
#pragma once

#include <stddef.h>

// returns the lenght of the provided string
size_t strlen(const char* str);

// copies one block of memory into another
void *memcopy(void *to, void *from, size_t n);