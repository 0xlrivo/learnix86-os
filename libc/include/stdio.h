#ifndef STDIO_H
#define STDIO_H

#define EOF (-1)

/* converts an integer number into a string and stores the result in str */
char* itoa(int value, char *str, int base);

/* prints a formatted string */
void printf(const char *format, ...);

int putchar(int);

int puts(const char*);

#endif