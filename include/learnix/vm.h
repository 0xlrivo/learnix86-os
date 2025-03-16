#ifndef VM_H
#define VM_H

#include <stdint.h>

/*
    Memory Map:

    0  -- LOW MEM --
    ...
    1MB -- UPPER MEM --
    ... unsed
    2MB -- KERNEL CODE --
*/

// define a pde_t type for readability purposes in vm.c
typedef uint32_t pde_t;
typedef uint32_t pte_t;

// defines a physical page metadata
typedef struct __physical_page_info {
    struct __physical_page_info* next;      // the next page on the free list
    uint16_t ref_count;                     // reference count for this physical page
} physicalPageInfo_t;

// called once in kernel_main to setup virtual memory
void vm_setup(uint32_t memlower, uint32_t memupper);

// minimal physical memory allocator that MUST only be used before the actual paging system is running
void* boot_alloc(uint32_t n);

// returns the corresponding PTE to va in the provided pgdir
// if alloc != 0 create such page if not existing
pte_t *pgdir_walk(pde_t *pgdir, void *va, int alloc);

#endif // !VM_H