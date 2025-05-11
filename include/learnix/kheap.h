#pragma once

#include <stdint.h>

/*
* NOTE: the kernel heap works with virtual addresses
*
* INTERFACE:
* void kheap_init()
* void kheap_grow()
* void* kmalloc(uint32_t size)
* void kfree(void* ptr)
*/

// chunk metadata
typedef struct __kheap_chunk_t
{
	uint32_t flags;
	uint32_t size;
	struct __kheap_chunk_t* next;
} kheap_chunk_t;

// called by the virtual memory system to
// initialize the kernel heap
// kernel_end: linker symbol that points to
// the next free byte after kernel code
void 
kheap_init(uintptr_t kernel_end);

// called by kmalloc to request more
// physical memory from the vm system
void
kheap_grow();

void*
kmalloc(uint32_t size);

void
kfree(void* ptr);

// prints all the kheap
// chunks
void
dbg_print_kheap();
