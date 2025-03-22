#include <string.h>

/*
    movsb is an x86 specific instruction that allows rapid copying of memory blocks
    - ESI points to the source
    - EDI points to the destination
    rep simply repeates the following instruction as specified in the ECX register
*/
void* memcpy(void* restrict dst, const void* restrict src, uint32_t size) {
    asm volatile (
        "rep movsb"
        : "+D" (dst), "+S" (src), "+c" (size)
        :
        : "memory"
    );
    return dst;
}