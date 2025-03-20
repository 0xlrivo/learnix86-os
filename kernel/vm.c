#include <string.h>
#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>
#include <learnix/drivers/serial.h>

/*
 * @brief x86 virtual memory implementation
*/ 

// linker symbol that points at the end of kernel's compiled code
extern char endkernel[];    

// points to the next byte of free physical memory, used by boot_alloc()
char* nextfree;      

// number of physical pages in extended memory
uint32_t npages;     

// linked list of physical pages metadata
physical_page_metadata_t* pages;    

// points to the next free physical page to allocate
physical_page_metadata_t* free_page_list;    

// kernel's page directory
pde_t* kern_pgdir;  

void vm_setup(uint32_t memlower, uint32_t memupper) {
    // compute the number of physical pages in the extended memory (above 1MB)
    npages = (memupper * 1024) / PGSIZE;

    // allocate a page for the kernel's page directory
    kern_pgdir = boot_alloc(PGSIZE);
    memset(kern_pgdir, 0, PGSIZE);

    // allocate enough physical pages to contain the pages[] array
    pages = (physical_page_metadata_t*)boot_alloc(npages * sizeof(physical_page_metadata_t));

    // initialize mappable physical pages and page_free_list to avoid allocating kernel code pages
    page_init();

    // initialize the kernel's page directory
    kern_pgdir_init();

    // load the kernel page directory into CR3
    pgdir_load(kern_pgdir);

    // activate paging
    uint32_t cr0 = rcr0();
    cr0 |= 0x80000001;
    lcr0(cr0);

    // jump to the next instruction's kernel virtual address
    uint32_t first_paged_instruction = pa2kva(0x200bb3);
    asm volatile("jmp %0" : : "r" (first_paged_instruction));
}

// the fist free page will be the last (npages-1)
void page_init() {
    uint32_t i;

    // 1MB - 2MB is free
    for(i = 0; i < page_num(KERN_BASE_PHYS); i++) {
        pages[i].next = free_page_list;
        pages[i].ref_count = 0;
        free_page_list = &pages[i];
    }

    // 2MB - nextfree is occupied by kernel code, pages[] and kern_pgdir
    for (i = page_num(KERN_BASE_PHYS); i < page_num(nextfree); i++) {
        pages[i].next = NULL;
        pages[i].ref_count = 0;
    }

    // the rest is free
    for(i = page_num(nextfree); i < npages; i++) {
        pages[i].next = free_page_list;
        pages[i].ref_count = 0;
        free_page_list = &pages[i];
    }
}

void kern_pgdir_init() {
    physaddr_t pa;
    uintptr_t va;

    // 1. kernel's code pages will be mapped from KERN_BASE_VRT
    for (pa = KERN_BASE_PHYS, va = KERN_BASE_VRT; pa < nextfree; pa += PGSIZE, va += PGSIZE) {
        map_pp_to_va(kern_pgdir, pa2page(pa), va);
        serial_printf("va %x mapped at %x\n", va, va_to_pa(kern_pgdir, va));
    }

    // 2. recursively map the kern_pgdir into itself at KERN_BASE_VRT - 4096
    map_pp_to_va(kern_pgdir, pa2page((physaddr_t)kern_pgdir), KERN_BASE_VRT - PGSIZE);
    serial_printf("va %x mapped at %x\n", KERN_BASE_VRT - PGSIZE, va_to_pa(kern_pgdir, KERN_BASE_VRT - PGSIZE));
}

void* boot_alloc(uint32_t n) {
    // the first physical address of the allocated region
    void* result;

    // initially nextfree == 0, so it must be initialized the first time boot_alloc() is called
    // at the first page-scaled address after the kernel's code
    if (!nextfree) {
        nextfree = (char*)PGROUNDUP((physaddr_t)endkernel);
    }

    // since we need to return the first allocated address we cache the value of nexfree before incrementing it
    result = nextfree;

    if (n > 0) {
        nextfree += PGROUNDUP(n);   // increment nextfree to n bytes page-aligned
    }

    return result;
}

// @note with usekva you can also use this function in the vm_setup where we're using physical addresses since paging is off
physical_page_metadata_t* kalloc(int zero, int usekva) {
    // pointer to the current free page
    physical_page_metadata_t* pp = free_page_list;

    // do nothing in case no free pages are left
    if (!pp) {
        return NULL;
    }

    // update free_page_list to point to the next of the current free page
    free_page_list = pp->next;

    // clear the next of pp
    pp->next = NULL;

    // if zero is set then fill the entire physical page with 0s
    if (zero) {
        usekva > 0 ? memset(page2kva(pp), 0, PGSIZE) : memset(page2pa(pp), 0, PGSIZE);
    }

    // returns a pointer to the physical_page_metadata, that you can treat differently is paging is on/off
    return pp;
}

void kfree(physical_page_metadata_t* pp) {
    // if such page has no reference counts
    if (pp->ref_count == 0) {
        pp->next = free_page_list;  // set his next to the current free page
        free_page_list = pp;        // it becomes the next free page allocatable
    }
}

// super useful function that returns the physical address of the page table entry of a given virtual address
// we return ALWAYS the physical address so that, if paging is on, you can simply obtain the kernel virtual address with pa2kva() otherwise just use it
pte_t* pgdir_walk(pde_t* pgdir, uintptr_t va, int create) {
    // optimistically compute the pte physical address
    pte_t pte = PTE_ADDR(pgdir[PDX(va)]);

    // assert that check if va is not already mapped in this page directory
    if (!(pgdir[PDX(va)] & PTE_P)) {
        
        // do not create the page table
        if (!create) return NULL;

        // allocate a zeroed physical page for the pte
        physical_page_metadata_t* pp = kalloc(1, 0);

        // return early on unsuccessful allocation
        if (pp == NULL)
            return NULL;

        // physical address of the allocated page
        pte = page2pa(pp);

        // update the page directory to contain the top 20 bits of the page physical address and the present flag (PTE_P)
        // @note page2pa() gives us a 32-bit physical address GUARANTEED to be page aligned, so we can safely store it there
        // |-> the MMU will only use the topmost 20-bits of it + PTX(va) to find the correct pte
        pgdir[PDX(va)] = pte | PTE_P;
        
    }

    // pte contains the physical address of the start of the page table physical page so, to obtain the correct PTE,
    // we simply add 4 * page directory index (since pte is an array of 4 bytes ptes)
    return (pte_t*)(pte + 4 * PTX(va));
}

physaddr_t va_to_pa(pde_t* pgdir, uintptr_t va) {
    // get the page table entry for this virtual address - don't create if not mapped
    pte_t* pte = pgdir_walk(pgdir, va, 0);
    // if pte is NULL then va is not mapped
    if (pte == NULL)
        return NULL;
    // if present bit is set add the offset otherwise return NULL since va is not mapped
    return *pte & PTE_P 
        ? PTE_ADDR(*pte) | PGOFFSET(va) 
        : NULL;
}

int map_pp_to_va(pde_t* pgdir, physical_page_metadata_t* pp, uintptr_t va) {
    // round down va to the nearest page multiple
    va = PGROUNDDOWN(va);
    // get the page table entry for va - create if not already mapped
    pte_t* pte = pgdir_walk(pgdir, va, 1);
    // reuturn on pgdir_walk allocation failure
    if (pte == NULL)
        return 0;
    // update the page table entry
    *pte = page2pa(pp) | PTE_P | PTE_W;
    // return success
    return 1;
}

void pgdir_load(pde_t* pgdir) {
    lcr3((uint32_t)&pgdir);   // load the pgdir address into CR3
}