#ifndef LEARNIXLIBC_STDIO_H
#define LEARNIXLIBC_STDIO_H

/* converts an integer number into a string and stores the result in str */
char* itoa(int value, char *str, int base);

/* prints a formatted string */
void printf(const char *format, ...);

#endif