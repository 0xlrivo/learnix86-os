#include "../inc/stdio.h"
#include "../../kernel/inc/vga.h"

#include <stdarg.h>

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
            terminal_putchar(*c);
        } 
        // otherwise read the next character to determine the format of the next argument
        else {
            c++;
            switch (*c) {
                // characther
                case 'c': {     
                    terminal_putchar(va_arg(args, int));
                    break;
                }
                // string
                case 's': {
                    terminal_writestring(va_arg(args, char*));
                    break;
                }
                // int
                case 'd': {
                    itoa(va_arg(args, int), buffer, 10);
                    terminal_writestring(buffer);
                    break;
                }
                // hex
                case 'x': {
                    itoa(va_arg(args, int), buffer, 16);
                    terminal_writestring(buffer);
                    break;
                }
                default: {
                    terminal_putchar('%');
                    terminal_putchar(*c);
                    break;
                }
            }
        }
    }
}