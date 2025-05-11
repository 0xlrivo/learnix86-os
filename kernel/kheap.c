#include "learnix/drivers/serial.h"
#include "learnix/vm.h"
#include "learnix/x86/mmu.h"
#include <learnix/kheap.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// first virtual address of the
// kernel heap
kheap_chunk_t* chunks_free_list;

// currently last mapped virtual
// address of kernel heap
uintptr_t kheap_end;

static void
kheap_test()
{
	// 1) EZ: allocate 3 integers arrays of 50 items	
	int* arr1 = (int*)kmalloc(50 * sizeof(int));
	int* arr2 = (int*)kmalloc(100 * sizeof(int));
	int* arr3 = (int*)kmalloc(20 * sizeof(int));
	
	serial_printf("\nTEST 1:\n");
	dbg_print_kheap();
	serial_printf("arr1: %x, arr2: %x, arr3: %x\n", arr1, arr2, arr3);

	// 2) free arr2 and try to allocate another
	// 100 int arrays... should get same va
	serial_printf("\nTEST 2:\n");
	kfree(arr2);
	dbg_print_kheap();
	int* arr4 = (int*)kmalloc(100 * sizeof(int));
	serial_printf("\nkmalloc arr4:\n");
	dbg_print_kheap();
	if (arr2 != arr4)
		panic("[ KHEAP] test 2 failed");
	
	// 3) COALESCING
	// free arr4 and then arr1
	serial_printf("\nTEST 3:\n");
	kfree(arr4);
	kfree(arr1);
	dbg_print_kheap();

	// 4) KHEAP GROWING
	serial_printf("\nTEST 4:\n");
	char* str1 = (char*)kmalloc(3400 * sizeof(char));
	printf("%x\n", str1);
	dbg_print_kheap();

	return;
}

void
kheap_init(uintptr_t kernel_end)
{
	// request a physical page
	physical_page_metadata_t* pp = page_alloc();

	// map such page at the next
	// virtual page after kernel_end
	map_pp(kern_pgdir, pp, PGROUNDUP(kernel_end));

	// the first chunk will begin
	// at such page
	chunks_free_list = (kheap_chunk_t*)PGROUNDUP(kernel_end);
	
	// initially, just 1 phyisical page
	// is allocated for this heap
	chunks_free_list->flags = 0;
	chunks_free_list->size = PGSIZE - sizeof(kheap_chunk_t);
	chunks_free_list->next = NULL;

	// initialize kheap_end as the last
	// virtual address of chunks_free_list's page
	kheap_end = (uintptr_t)chunks_free_list + PGSIZE - 1;

	// run tests
	kheap_test();
}

void
kheap_grow()
{
	// request a physical page
	// from the allocator
	physical_page_metadata_t* pp = page_alloc();

	// and map it as a contiguous
	// kernel heap address
	map_pp(kern_pgdir, pp, kheap_end + 1);

	// how kheap_end must point
	// at the last address of this
	// new virtual page
	kheap_end += PGSIZE;
}

void*
kmalloc(uint32_t size)
{
	// search for a free and large enough
	// block using first-fit
	kheap_chunk_t* curr = chunks_free_list;
	while (curr != NULL)
	{
		if (!curr->flags && curr->size >= size)
		{
			// we've found our chunk
			break;
		}
		
		// in case we're at the end of the list
		// without any free and large enough chunks
		// we must grow the heap
		if (curr->next == NULL)
		{
			kheap_grow();
			
			// update size of current chunk
			// without advancing to the next
			// so that we keep growing the heap
			// untill we can either service the request
			// or run out of physical memory
			curr->size += PGSIZE;
		}
		else
		{
			// when curr isn't the last chunk
			// we can just go to the next one
			curr = curr->next;
		}
	}

	// if curr is NULL we can't service
	// the request
	if (curr == NULL) return NULL;

	// virtual address of the next chunk
	kheap_chunk_t* next = (kheap_chunk_t*)((uint32_t)curr + size + sizeof(kheap_chunk_t));
	
	// initialize the next chunk only if
	// it is not already allocated
	if (!next->flags)
	{
		// TODO: is -sizeof(kheap_chunk_t correct)??
		next->size = curr->size - size - sizeof(kheap_chunk_t);
		next->flags = 0; 	// not allocated
		next->next = NULL;
	}

	// and update the current
	// chunk (now allocated)
	curr->size = size;
	curr->flags = 1;
	curr->next = next;

	// return the first byte after
	// the chunk header
	return (void*)(curr + 1);
}

void
kfree(void *ptr)
{
	// get a pointer to the
	// chunk header
	kheap_chunk_t* chunk = (kheap_chunk_t*)(ptr - sizeof(kheap_chunk_t));
	
	// reject not-allocated
	// chunks
	if (!chunk->flags) return;

	// mark chunk as not allocated
	// anymore
	chunk->flags = 0;

	// coalesce the free list of chunks
	// NOTE: we need a full traversal
	chunk = chunks_free_list;
	while (chunk->next != NULL)
	{
		// is his neighboor chunk free too?
		// if yes then coalesce those
		if (!chunk->flags && !chunk->next->flags)
		{
			// update size of the first chunk
			// to "include" the second one
			chunk->size += chunk->next->size + sizeof(kheap_chunk_t);
			// actually delete the second chunk
			chunk->next = chunk->next->next;
		}
		else 
		{
			// otherwise advance to the next chunk
			chunk = chunk->next;
		}
	}
}

void
dbg_print_kheap()
{
	// start from the first chunk
	kheap_chunk_t* curr = chunks_free_list;
	uintptr_t start, end;
	uint32_t size;
	int i = 0;
	
	while(curr != NULL)
	{
		start = (uintptr_t)curr + sizeof(kheap_chunk_t);
		size = curr->size;
		end = start + size - 1; // actual end
		
		serial_printf("== CHUNK %d ==\n", i);
		serial_printf("start: %x\nsize:%d\nend: %x\n", start, size, end);

		if (curr->flags)
			serial_printf("allocated\n");
		else
		 	serial_printf("unallocated\n");

		// advance to next chunk
		curr = curr->next;
		++i;
	}
}
