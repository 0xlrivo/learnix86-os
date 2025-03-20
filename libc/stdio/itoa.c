#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

char* itoa(uint32_t value, char *str, int base) {
    char *digits = "0123456789ABCDEF";
    bool is_negative = false;
    int i = 0;

    if (value < 0 && base == 10) {
        is_negative = true;
        value = -value;
    }

    do {
        str[i++] = digits[value % base];
        value /= base;
    } while(value > 0);

    // pad hex numbers to 8 digits (32 bit)
    if (base == 16) {
        while(i < 8) {
            str[i++] = '0';
        }
        str[i++] = 'x';
        str[i++] = '0';
    }

    if (is_negative)
        str[i++] = '-';

    str[i] = '\0';

    // invert the string
    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = tmp;
    }
}