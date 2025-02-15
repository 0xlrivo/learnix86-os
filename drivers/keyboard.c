#include <stdint.h>
#include <stdio.h>
#include <learnix/x86.h>
#include <learnix/drivers/keyboard.h>

char scancode_to_char[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // 0x00-0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  // 0x0F-0x1C
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',         // 0x1D-0x29
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,           // 0x2A-0x36
    '*', 0, ' ', 0                                                          // 0x37-0x39
};

void keyboard_main() {
    uint8_t scancode = inb(KEYBOARD_PORT);
    if(scancode < sizeof(scancode_to_char)) {
        putchar(scancode_to_char[scancode]);
    }
}