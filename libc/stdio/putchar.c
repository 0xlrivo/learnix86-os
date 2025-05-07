#include <stdio.h>

#if defined(__is_libk)
#include <learnix/drivers/vga.h>
#endif

int
putchar(int c)
{
#if defined(__is_libk)
	terminal_putchar((char)c); // kernel's libc calls the VGA driver
#else
	// TODO: userland's libc will use the write syscall
#endif
	return c;
}