#include <string.h>
#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>
#include <learnix/drivers/serial.h>

/*
 * @brief x86 virtual memory implementation
*/ 


extern char endkernel[];    // linker symbol that points at the end of kernel's compiled code

static char* nextfree;      // points to the next byte of free physical memory, used by boot_alloc()

uint32_t npages;     // number of physical pages in extended memory

physical_page_metadata_t* pages;    // linked list of physicalPageInfo which contains metadata for each physical page

physical_page_metadata_t* free_page_list;    // points to the next free physical page to allocate

pde_t* kern_pgdir;  // kernel's page directory

void vm_setup(uint32_t memlower, uint32_t memupper) {
    // compute the number of physical pages in the extended memory (above 1MB)
    npages = (memupper * 1024) / PGSIZE;

    serial_printf("-- Avaiable Memory --\nmemlower: %d Kb\nmemupper: %d Kb\nnpages: %d\n", memlower, memupper, npages);

    serial_printf("boot_alloc(kern_pgdir) = %x | physaddr_t casting = %x\n", boot_alloc(0), (physaddr_t)boot_alloc(0));
    // allocate a page for the kernel's page directory
    kern_pgdir = boot_alloc(PGSIZE);
    memset(kern_pgdir, 0, PGSIZE);
    kern_pgdir[PDX(KERN_BASE_VRT-PGSIZE)] = (physaddr_t)kern_pgdir | PTE_U | PTE_P;
    serial_printf("pgdir[KERN_BASE_VRT-PGSIZE] = %x\n", kern_pgdir[PDX(KERN_BASE_VRT-PGSIZE)]);

    // allocate enough physical pages to contain the pages[] array
    pages = (physical_page_metadata_t*)boot_alloc(npages * sizeof(physical_page_metadata_t));

    // initialize mappable physical pages and page_free_list to avoid allocating kernel code pages
    page_init();

    pte_t* t = pgdir_walk(kern_pgdir, KERN_BASE_VRT-PGSIZE);

    t = pgdir_walk(kern_pgdir, KERN_BASE_VRT);
    serial_printf("t2 = %x\n", &t);
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

// the fist free page will be the last (npages-1)
void page_init() {
    uint32_t i;
    physaddr_t lastkernelpage = PGROUNDUP((physaddr_t)endkernel + (npages * sizeof(physical_page_metadata_t)));

    // 1MB - 2MB is free
    for(i = 0; i < page_num(KERN_BASE_PHYS); i++) {
        pages[i].next = free_page_list;
        pages[i].ref_count = 0;
        free_page_list = &pages[i];
    }

    // 2MB - lastkernelpage is occupied and MUST never be mapped
    for (i = page_num(KERN_BASE_PHYS); i < page_num(lastkernelpage); i++) {
        pages[i].next = NULL;
        pages[i].ref_count = 0;
    }

    // the rest is free
    for(i = page_num(lastkernelpage); i < npages; i++) {
        pages[i].next = free_page_list;
        pages[i].ref_count = 0;
        free_page_list = &pages[i];
    }
}

physical_page_metadata_t* kalloc(int zero) {
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
        memset(page2kva(pp), 0, PGSIZE);
    }

    return pp;
}

void kfree(physical_page_metadata_t* pp) {
    // if such page has no reference counts
    if (pp->ref_count == 0) {
        pp->next = free_page_list;  // set his next to the current free page
        free_page_list = pp;        // it becomes the next free page allocatable
    }
}

pte_t* pgdir_walk(pde_t* pgdir, uintptr_t va) {
    pte_t* pte = NULL;

    // page-align va to avoid weird mappings
    va = PGROUNDDOWN(va);

    // assert that check if va is not already mapped in this page directory
    if (!(pgdir[PDX(va)] & PTE_P)) {

        // allocate a zeroed physical page for the pte
        physical_page_metadata_t* pp = kalloc(1);

        // return early on unsuccessful allocation
        if (pp == NULL)
            return NULL;
        serial_printf("not already mapped, pte at physaddr: %x\n", page2pa(pp));
        // increment pp reference count since we're mapping it
        pp->ref_count++;

        // update the page directory to contain the top 20 bits of the page physical address and the present flag (PTE_P)
        // @note page2pa() gives us a 32-bit physical address GUARANTEED to be page aligned, so we can safely store it there
        // |-> the MMU will only use the topmost 20-bits of it + PTX(va) to find the correct pte
        pgdir[PDX(va)] = page2pa(pp) | PTE_P;

        pte = page2kva(pp);
        return pte + PTX(va) * 4;
    }

    // in case it is already mapped
    serial_write("already mapped\n");

    return NULL;
}

void pgdir_load(pde_t* pgdir) {
    lcr3((uint32_t)&pgdir);   // load the pgdir address into CR3
}