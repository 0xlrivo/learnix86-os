#include <stdio.h>
#include <stdlib.h>

void panic(const char *msg) {
#if defined(__is_libk)
	printf("KERNEL PANIC: %s\n", msg);
#else
    printf("PROCESS PANIC: %s\n", msg);
#endif
    while(1);
}