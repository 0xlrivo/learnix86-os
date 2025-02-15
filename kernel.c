#include<stdio.h>
#include <learnix/vga.h>
#include <learnix/idt.h>

void kernel_main() {

    // initialize the PIC
    pic_init();

    // load the Interrupt Descriptor Table
    idt_init();

    // initialize vga terminal
    terminal_initialize();

    printf("Hello Kernel!");

    // hang
    while(1);
}