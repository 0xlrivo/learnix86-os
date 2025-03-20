#ifndef LEARNIX_LIBC_STDLIB_H
#define LEARNIX_LIBC_STDLIB_H

#include <stdint.h>

/// prints the reason and then halts execution
void panic(const char* reason);

/// converts an integer value to a C string using the specified base
/// @param value number to convert (signed int)
/// @param str buffer to store the conversion's result
/// @param base if base = 10 and value is negative the string will start with '-'
char* itoa(int value, char* str, int base);

/// converts an unsiged integer value to a C string using the specified base
/// @param value number to convert (signed int)
/// @param str buffer to store the conversion's result
/// @param base if base = 10 and value is negative the string will start with '-'
/// @note useful to print addresses with 8 hex digits, which are truncated by a normal int
char* itoal(uint32_t value, char* str, int base);

#endif