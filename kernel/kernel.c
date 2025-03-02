#include <stdio.h>
#include <stdlib.h>
#include <learnix/multiboot.h>
#include <learnix/vm.h>
#include <learnix/vga.h>
#include <learnix/idt.h>

void kernel_main(uint32_t magic, multiboot_info_t *mbi) {

    // initialize the vga terminal
    terminal_initialize();

    // multiboot controls
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("[GRUB] invalid magic number");
    }
    if (!(mbi->flags >> 6 & 0x1)) {
        panic("[GRUB] invalid memory map");
    }

    // initialize virtual memory
    //vm_setup();

    // initialize and remap the PIC for protected mode usage
    pic_init();

    // load the Interrupt Descriptor Table
    idt_init();

    // hang
    while(1);
}