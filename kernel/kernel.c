#include "../inc/vga.h"

void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();

	/* Newline support is left as an exercise. */
	terminal_writestring("A\na\n\n\n\nAAAAAAAA\nBBBBB\n\nCCCCCC\n\n\n\n\n\n\nDDDDDDD\nJJJJJ\nLLLLKKO\n\nRRRR\nR\nJ\nK\nL\nEEEE\nTOp");
	terminal_writestring(" peria\nAAAA");
}