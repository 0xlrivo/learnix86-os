#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CONTROL_PORT 0x64

#define KEY_RELEASED_MASK 0x80

#define LEFT_SHIFT_PRESSED 0x2a
#define LEFT_SHIFT_RELEASED 0xAA

#define CAPS_LOCK_PRESSED 0x3A
#define CAPS_LOCK_RELEASED 0xBA // useless

void keyboard_main();

#endif