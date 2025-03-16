#include <stdio.h>
#include <stdlib.h>
#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>

/*
 * @brief x86 virtual memory implementation
*/ 

extern char endkernel[];

// points to the next byte of free physical memory, used by boot_alloc()
static char* nextfree;

// number of physical pages
static uint32_t npages;

// linked list of physicalPageInfo which contains metadata for each physical page
static physicalPageInfo_t* pages;

// kernel's page directory
pde_t* kern_pgdir;  

// @todo maybe use memlower in future
void vm_setup(uint32_t memlower, uint32_t memupper) {    
    // number of avaiable physical page in the extended memory (above 1MB)
    npages = (memupper * 1024) / PGSIZE;

    // allocate the pages linked list
    pages = (physicalPageInfo_t*)boot_alloc(npages * sizeof(physicalPageInfo_t));

    kern_pgdir = (pde_t*)boot_alloc(PGSIZE);    // allocate a physical page to hold the kernel page directory
    memset((void*)kern_pgdir, 0x00, PGSIZE);    // zero-out the page directory's memory

}

void* boot_alloc(uint32_t n) {
    // the first address of the allocated region
    void* result;

    // intially nextfree = 0, so it must be initialized the first time boot_alloc() is called
    // at the first page-scaled address after the kernel's code
    if(!nextfree) {
        nextfree = PGROUNDUP((uint32_t)endkernel); // must be casted as integer because PGROUNDUP expects an integer argument
    }

    // scales the n bytes to allocate to a multiple of PGSIZE (4096)
    uint32_t bytesAlloc = PGROUNDUP(n);

    // since we need to return the first allocated address we cache the value of nexfree
    // before incrementing it
    result = nextfree;

    // increment nextfree to mark bytesAlloc bytes of physical memory as allocated
    nextfree += bytesAlloc;

    return result;
}