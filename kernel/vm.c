#include <stdio.h>
#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>

// physical page metadata array
physical_page_metadata_t pages[1024];

// points to the next free physical page to service allocation requests
physical_page_metadata_t* pages_next_free;

// @question should this contain the physical or virtual address of the pgdir?
// kernel page directory
pde_t* kern_pgdir;

void vm_setup(uint32_t memlower, uint32_t memupper) {
    kern_pgdir = (pte_t*)rcr3();
}

pte_t* pgdir_walk(pde_t* pgdir, uintptr_t va, int create) {
    // extract page directory index and page table index from the virtual address
    uint32_t pdx = PDX(va), ptx = PTX(va);
    
    // optimistically compute the page table address
    physaddr_t pt = PTE_ADDR(pgdir[pdx]);

    // if no page table exists for the provided page directory index
    if (!(pgdir[pdx] & PTE_P)) {
        // just return NULL if !create to let the caller know that va is not mapped
        if (!create) return NULL;
        
        // TODO
    }

    return (pte_t*)(pt + sizeof(pte_t) * ptx);
}