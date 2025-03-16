#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <learnix/x86/mmu.h>

/*
    Memory Map:

    0  -- LOW MEM --
    ...
    1MB -- UPPER MEM --
    ... unsed
    2MB -- KERNEL CODE --
*/

#define UPPER_MEM_BASE 0x00100000 // 1 MB

#define KERN_BASE 0x00200000    // kernel link address (2MB)

// define a pde_t type for readability purposes in vm.c
typedef uint32_t pde_t;
typedef uint32_t pte_t;

typedef uint32_t physaddr_t;

// defines a physical page metadata
typedef struct __physical_page_info {
    struct __physical_page_info* next;      // the next page on the free list
    uint16_t ref_count;                     // reference count for this physical page
} physicalPageInfo_t;

extern physicalPageInfo_t* pages;

// returns the physical address of a given physicalPageInfo_t
// @note currently only the upper memory is mapped so we start at 1MB
static inline physaddr_t page2pa(physicalPageInfo_t *pp) {
	return UPPER_MEM_BASE + ((pp - pages) * PGSIZE);
}

static inline uint32_t page_num(physaddr_t pa) {
    return (pa - 0x100000) >> 12;
}

// returns the physicalPageInfo_t of a given physical address
// @note currently only the upper memory is mapped so we start at 1MB
static inline physicalPageInfo_t* pa2page(physaddr_t pa) {
    return &pages[(pa - 0x100000) >> 12];
}

// called once in kernel_main to setup virtual memory
void vm_setup(uint32_t memlower, uint32_t memupper);

// loads the provided pgdir into the CR3 register
void pgdir_load(pde_t* pgdir);

// minimal physical memory allocator that MUST only be used before the actual paging system is running
void* boot_alloc(uint32_t n);

// allocates a physical page and returns the physicalPageInfo
physicalPageInfo_t* page_alloc();

// maps va (virtual address) at given physical address (pa)
void map_va(pde_t* pgdir, void* va, uint32_t pa);

// returns the corresponding PTE to va in the provided pgdir
// if alloc != 0 create such page if not existing
pte_t *pgdir_walk(pde_t* pgdir, void* va, int alloc);

#endif // !VM_H