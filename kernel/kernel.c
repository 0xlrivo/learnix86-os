#include<stdio.h>
#include <learnix/vga.h>
#include <learnix/idt.h>

void kernel_main() {

    // initialize and remap the PIC for protected mode usage
    pic_init();

    // load the Interrupt Descriptor Table
    idt_init();

    // initialize the vga terminal
    terminal_initialize();

    // hang
    while(1);
}