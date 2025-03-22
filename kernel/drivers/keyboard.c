#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <learnix/x86/x86.h>
#include <learnix/drivers/keyboard.h>

// bit 0: shift pressed
// bit 1: caps lock pressed
static uint8_t status = 0;

char scancode_lowercase[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // 0x00-0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  // 0x0F-0x1C
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',         // 0x1D-0x29
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,           // 0x2A-0x36
    '*', 0, ' ', 0                                                          // 0x37-0x39
};

char scancode_uppercase[] = {
    0, 27, '!', '"', '£', '$', '%', '&', '/', '(', ')', '=', '-', '=', '\b', // 0x00-0x0E
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 'è', '*', '\n',  // 0x0F-0x1C
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'ç', '\'', '`',         // 0x1D-0x29
    0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 0,           // 0x2A-0x36
    '*', 0, ' ', 0                                                          // 0x37-0x39
};


void keyboard_main() {
    // read scancode from keyboard data port
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // if the top bit is set means that a key was released
    if(scancode & KEY_RELEASED_MASK) {
        switch(scancode) {
            case LEFT_SHIFT_RELEASED: {
                status &= ~1;           // set status 0th bit to 0
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
                status |= (1 << 0);     // set status 0th bit to 1
                break;
            }
            case CAPS_LOCK_PRESSED: {
                status ^= (1 << 1);     // flip the caps lock status bit when pressed
                break;
            }
            default: {
                // @todo caps lock only works on letters
                if(status & 0x03)   // shift or capslock pressed 
                    putchar(scancode_uppercase[scancode]);
                else
                    putchar(scancode_lowercase[scancode]);
                break;
            }
        }
    }
}