/*
    * implements functions to manage the IDT (Interrupt Descriptor Table)
*/

#include "../learnixlibc/inc/stdio.h"
#include "../learnixlibc/inc/string.h"
#include "inc/interrupts.h"
#include "inc/pic.h"
#include "inc/vga.h"
#include "inc/x86.h"
#include <stdint.h>

idtr_t idtr;
idte_t idt[IDT_ENTRIES]; // IDT

// called by kernel_main once during OS setup
void idt_init() {
    // zero-out the IDT entries
    memset(&idt, 0, sizeof(idte_t) * IDT_ENTRIES);

    // setup interrupt service routines
    idt_set_gate(IRQ1_IDX, (uint32_t)keyboard_isr_wrapper, 0x08, 0x8E); // keyboard handler (IRQ1)

    // and finally load the IDT into the IDTR register and enable interrupts
    idt_load();
}

void idt_load() {
    // setup IDTR register
    idtr.base = (uint32_t)&idt;
    idtr.limit = (sizeof(idte_t) * IDT_ENTRIES) - 1;

    // load the table with the lidt instruction
    asm volatile ("lidt (%0)" : : "r" (&idtr));

    // enables interrupts
    asm volatile ("sti");
}

void idt_set_gate(int n, uint32_t handler, uint16_t selector, uint8_t type_attributes) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = selector;
    idt[n].zero = 0;
    idt[n].type_attributes = type_attributes;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

char scancode_to_char[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // 0x00-0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  // 0x0F-0x1C
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',         // 0x1D-0x29
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,           // 0x2A-0x36
    '*', 0, ' ', 0                                                          // 0x37-0x39
};

// IRQ1
void keyboard_handler() {
    uint8_t scancode = inb(KEYBOARD_PORT);
    if(scancode < sizeof(scancode_to_char)) {
        terminal_putchar(scancode_to_char[scancode]);
    }
    pic_send_eoi(1);
}
