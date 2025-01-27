#include "inc/vga.h"
#include "inc/interrupts.h"
#include "inc/pic.h"
#include "../learnixlibc/inc/stdio.h";
#include "../learnixlibc/inc/string.h";

void kernel_main(void)
{
	/* initialize the PICs */
	pic_init();

	/* IDT setup */
	idt_init();

    /* Initialize terminal interface */
	terminal_initialize();

	printf("Hello World!\n");

	while(1) {}
}
