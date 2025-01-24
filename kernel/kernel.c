#include "inc/vga.h"
#include "../learnixlibc/inc/stdio.h";
#include "../learnixlibc/inc/string.h";

void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();

	char* str1 = "Ciao Mondo";
	char str2[20];
	memcpy(str2, str1, 4);
	str2[5] = '\0';
	printf("a\nstr1: %s\nstr2: %s\n%d", str1, str2, strcmp(str1, str2));
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\naaaaaa\n\n\nbbb\n\nrr\naa\n77");
}