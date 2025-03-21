#include <stdio.h>
#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>

// symbol defined in boot/boot.S which points to the kernel's page directory
extern char boot_page_directory[];

// total number of physical pages
uint32_t npages;

// physical page metadata array of size npages * sizeof(physical_page_metadata_t)
physical_page_metadata_t* pages;

// points to the next free physical page to service allocation requests
physical_page_metadata_t* pages_next_free;

// kernel page directory's virtual address
pde_t* kern_pgdir;

void vm_setup(uint32_t memlower, uint32_t memupper) {
    // setup the kern_pgdir
    kern_pgdir = (pte_t*)boot_page_directory;

    // compute upper memory total of physical pages
    npages = (memupper * 1024) / PGSIZE;

    printf("[LOG] memupper: %d KB for a total of %d pages\n", memupper, npages);

    printf("%x\n", va_to_pa(kern_pgdir, KERN_BASE_VRT + KERN_BASE_PHYS));
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
        
        // TODO allocate physical page for the page table if needed and create is set
        return NULL;
    }

    return (pte_t*)(pa2kva(pt) + sizeof(pte_t) * ptx);
}

physaddr_t va_to_pa(pde_t* pgdir, uintptr_t va) {
    // extract the page table without creating if not mapped
    pte_t* pte = pgdir_walk(pgdir, va, 0);
    // if pte is NULL it means that va is not mapped
    if (pte == NULL)
        return NULL;
    // if the present bit is set thatn add the offset otherwise NULL
    return *pte & PTE_P
        ? PTE_ADDR(*pte) | PGOFFSET(va)
        : NULL;
}