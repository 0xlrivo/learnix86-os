#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>

extern char _kernel_end[];

// symbol defined in boot/boot.S which points to the kernel's page directory
extern char boot_page_directory[];

// total number of physical pages
uint32_t npages;

// physical page metadata array of size npages * sizeof(physical_page_metadata_t)
physical_page_metadata_t* pages;

// points to the next free physical page to service allocation requests
physical_page_metadata_t* pages_free_list;

// kernel page directory's virtual address
pde_t* kern_pgdir;

void vm_setup(uint32_t memlower, uint32_t memupper) {
    // setup the kern_pgdir
    kern_pgdir = (pte_t*)boot_page_directory;

    // compute upper memory total of physical pages
    npages = (memupper * 1024) / PGSIZE;
    
    printf("[LOG] memupper: %d KB for a total of %d pages\n", memupper, npages);

    // initialize the physical page tracking structure
    pages_setup();

    physical_page_metadata_t* p = kalloc(1);
    printf("got physical page: %d with pa %x", page_num(page2pa(p)), page2pa(p));
}

// @todo also allocate low-mem pages after parsing multiboot struct
void pages_setup() {
    uint32_t i, va, pa;

    // allocate space for pages[] starting from 1MB physical
    pages = (physical_page_metadata_t*)pa2kva(EXT_MEM_BASE);

    // how many pages are needed to contain the pages[] array... must virtually map those to avoid faults
    uint32_t pages_to_map = ((npages * sizeof(physical_page_metadata_t)) / PGSIZE) + 1;

    // and map those starting from 1MB physical 
    for (i = 0, va = pages, pa = EXT_MEM_BASE; i < pages_to_map; i++, va += PGSIZE, pa += PGSIZE) {
        map_va(kern_pgdir, va, pa);
    }

    // 1MB - pages_to_map is occupied by pages[]
    for (i = page_num(EXT_MEM_BASE); i < pages_to_map; i++) {
        pages[i].ref_count = 0;
        pages[i].flags = PPM_KERN;
        pages[i].next = NULL;
    }

    // pages_to_map - 2MB is free
    for (i = pages_to_map; i < page_num(KERN_BASE_PHYS); i++) {
        pages[i].ref_count = 0;
        pages[i].flags = 0;
        pages[i].next = pages_free_list;
        pages_free_list = &pages[i];
    }

    // 2MB - endkernel is occupied by kernel code
    for (i = page_num(KERN_BASE_PHYS); i < page_num(kva2pa(_kernel_end)); i++) {
        pages[i].ref_count = 0;
        pages[i].flags = PPM_KERN;
        pages[i].next = NULL;
    }

    // the rest is free
    for (i; i < npages; i++) {
        pages[i].ref_count = 0;
        pages[i].flags = 0;
        pages[i].next = pages_free_list;
        pages_free_list = &pages[i];
    }
}

physical_page_metadata_t* kalloc() {
    // fetch the next free page from the linked list
    physical_page_metadata_t* pp = pages_free_list;

    // panic if out-of-pages
    if (pp == NULL)
        panic("[KALLOC] out-of-pages");

    // update the free_page_list
    pages_free_list = pp->next;

    // clear pp->next
    pp->next = NULL;

    // return a pointer to the page's metadata
    return pp;
}

physical_page_metadata_t* kfree(physical_page_metadata_t* pp) {
    if (pp->flags != PPM_KERN && pp->ref_count == 0) {
        pp->next = pages_free_list;
        pages_free_list = pp;
        return pp;
    }
    return NULL;
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
        
        // allocate a physical page to use as the page table
        physical_page_metadata_t* pp = kalloc();
        
        // extract the page table address
        pt = PTE_ADDR(page2pa(pp));

        // update the page directory
        pgdir[pdx] = pt | PTE_P | PTE_U | PTE_W;
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

void map_va(pde_t* pgdir, uintptr_t va, physaddr_t pa) {
    // round down both addresses to the nearest page
    va = PGROUNDDOWN(va);
    pa = PGROUNDDOWN(pa);
    // fetch pte for va
    pte_t* pte = pgdir_walk(pgdir, va, 1);
    // return early on error
    if (pte == NULL) 
        return;
    // update the PTE
    *pte = PTE_ADDR(pa) | PTE_P | PTE_W;
}

/// maps the given physical page to va
void map_pp(pde_t* pgdir, physical_page_metadata_t* pp, uintptr_t va) {
    map_va(pgdir, va, page2pa(pp));
}