#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

void printf(const char *format, ...) {
    // starg.h defined list of arguments
    va_list args;
    // initialize the list of arguments with the latest known argument (format at the beginning)
    va_start(args, format);

    // used to store itoa translations
    char buffer[32];

    // loop through the format string
    for (const char *c = format; *c != '\0'; c++) {
        // if c isn't '%' simply print it as it is
        if (*c != '%') {
            putchar(*c);
        } 
        // otherwise read the next character to determine the format of the next argument
        else {
            c++;
            switch (*c) {
                // characther
                case 'c': {     
                    putchar(va_arg(args, int));
                    break;
                }
                // string
                case 's': {
                    puts(va_arg(args, char*));
                    break;
                }
                // signed int
                case 'd': {
                    itoa(va_arg(args, int), buffer, 10);
                    puts(buffer);
                    break;
                }
                // hex
                case 'x': {
                    itoal(va_arg(args, uint32_t), buffer, 16);
                    puts(buffer);
                    break;
                }
                default: {
                    putchar('%');
                    putchar(*c);
                    break;
                }
            }
        }
    }
}