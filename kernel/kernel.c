#include <stdio.h>
#include <stdlib.h>
#include <learnix/vm.h>
#include <learnix/multiboot.h>
#include <learnix/drivers/vga.h>

void kernel_main(uint32_t magic, physaddr_t mbi_phys) {

    // initialize the VGA driver
    terminal_initialize();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("[GRUB] invalid magic number");
    }

    multiboot_info_t* mbi = (multiboot_info_t*)(mbi_phys + KERN_BASE_VRT);
    printf("%d\n", mbi->mem_lower);

	/* at this point we're in protected mode with paging enabled with the first 4MB mapped at 3GB va*/
    puts("Hello Kernel with x86 paging on\n");

    vm_setup(0, 0);

    while(1){};
}