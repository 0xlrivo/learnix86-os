#include <stdio.h>
#include <stdlib.h>
#include <learnix/vm.h>
#include <learnix/multiboot.h>
#include <learnix/drivers/vga.h>

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {

    // initialize the VGA driver
    terminal_initialize();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("[GRUB] invalid magic number");
    }
    if (!(mbi->flags >> 6 & 0x1)) {
        panic("[GRUB] invalid memory map");
    }

    // setup the virtual memory manager
    vm_setup(mbi->mem_lower, mbi->mem_upper);

    while(1) {};
}