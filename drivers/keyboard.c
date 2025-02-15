#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <learnix/x86.h>
#include <learnix/drivers/keyboard.h>

// bit 0: shift pressed
// bit 1: caps lock pressed
static uint8_t status = 0;

char scancode_to_char[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // 0x00-0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  // 0x0F-0x1C
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',         // 0x1D-0x29
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,           // 0x2A-0x36
    '*', 0, ' ', 0                                                          // 0x37-0x39
};

void keyboard_main() {
    // read scancode from keyboard data port
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // if the top bit is set means that a key was released
    if(scancode & KEY_RELEASED_MASK) {
        switch(scancode) {
            case LEFT_SHIFT_RELEASED: {
                status |= (0 << 0);
                break;
            }
            case CAPS_LOCK_RELEASED: {
                status |= (0 << 1);
                break;
            }
            default: 
                break;
        }
    } 
    // otherwise a key was pressed
    else {
        switch(scancode) {
            case LEFT_SHIFT_PRESSED: {
                status |= (1 << 0);
                break;
            }
            case CAPS_LOCK_PRESSED: {
                status |= (1 << 1);
                break;
            }
            default: {
                if(status & 0x03) // shift or capslock
                    putchar(toupper(scancode_to_char[scancode]));
                else
                    putchar(scancode_to_char[scancode]);
                break;
            }
        }
    }
}