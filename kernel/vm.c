#include <string.h>
#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>
#include <learnix/drivers/serial.h>

/*
 * @brief x86 virtual memory implementation
*/ 

extern char endkernel[];

// points to the next byte of free physical memory, used by boot_alloc()
static char* nextfree;

// number of physical pages
static uint32_t npages;

// linked list of physicalPageInfo which contains metadata for each physical page
physicalPageInfo_t* pages;

// points to the next free physical page to allocate
physicalPageInfo_t* free_list;

// kernel's page directory
pde_t* kern_pgdir;  

void vm_setup(uint32_t memlower, uint32_t memupper) {
    // number of avaiable physical page in the extended memory (above 1MB)
    npages = (memupper * 1024) / PGSIZE;

    serial_printf("-- Avaiable Memory --\nmemlower: %d Kb\nmemupper: %d Kb\nnpages: %d\n", memlower, memupper, npages);

    // allocate the pages linked list
    pages = (physicalPageInfo_t*)boot_alloc(npages * sizeof(physicalPageInfo_t));

    page_init();
}

void pgdir_load(pde_t* pgdir) {
    lcr3((uint32_t)&pgdir);   // load the pgdir address into CR3
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

// initialize the physical page array (pages)
// the fist free page will be the last (npages-1)
void page_init() {
    // 1MB - 2MB is free
    for(uint32_t i = 0; i < page_num(KERN_BASE); i++) {
        pages[i].next = free_list;
        pages[i].ref_count = 0;
        free_list = &pages[i];
    }
    // 2MB - endkernel + sizeof(pages) is occupied and MUST never be mapped
    physaddr_t nxt = PGROUNDUP((physaddr_t)endkernel + (npages * sizeof(physicalPageInfo_t)));
    // the rest is free
    for(uint32_t i = page_num(nxt); i < npages; i++) {
        pages[i].next = free_list;
        pages[i].ref_count = 0;
        free_list = &pages[i];
    }
}

void map_va(pde_t* pgdir, void* va, uint32_t pa);