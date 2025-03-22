#ifndef LEARNIX_LIBC_STDIO_H
#define LEARNIX_LIBC_STDIO_H

#include <stdint.h>
#include <stddef.h>

#define EOF (-1)

/// writes a characther on the standard output
int putchar(int c);

/// writes a C-string on the standard output
int puts(const char*);

/// writes formatted data on the standard output
void printf(const char *format, ...);

#endif