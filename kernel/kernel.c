#include <learnix/vm.h>
#include <learnix/pic.h>
#include <learnix/idt.h>
#include <learnix/multiboot.h>
#include <learnix/drivers/vga.h>
#include <learnix/drivers/serial.h>
#include <stdio.h>

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {

    // initialize the VGA driver
    terminal_initialize();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("[GRUB] invalid magic number");
    }
    if (!(mbi->flags >> 6 & 0x1)) {
        panic("[GRUB] invalid memory map");
    }

    // initialize the PIC
    pic_init();

    // initialize the COM1 serial port
    serial_init();

    // initialize the Interrupt Descriptor Table (IDT)
    idt_init();

    // setup the virtual memory manager
    vm_setup(mbi->mem_lower, mbi->mem_upper);

	printf("%d\n", *(int*)KERN_BASE_PHYS);

    while(1) {};
}
