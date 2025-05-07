#include <string.h>

/*
 * stobs is an x86 specific-instruction that enables hardware-optimized memory
 * filling
 */
void *
memset(void *ptr, int value, uint32_t num)
{
	asm volatile("cld; rep stosb\n" ::"D"(ptr), "a"(value), "c"(num)
	             : "cc", "memory");
	return ptr;
}