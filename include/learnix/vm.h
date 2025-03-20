#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdlib.h>
#include <learnix/x86/mmu.h>

/*
    x86 physical memory map (u = usable):

    +---------------------+     <- 0xFFFFFFFF (4 GB)
    |    32-bit memory    |
    |   mapped devices    |
    +---------------------+     <- depends on installed RAM
    |                     |
    |   Extended Memory   |
    |         (u)         |     <- 0x00200000 (2 MB)        kernel's code is loaded from here
    +---------------------+     <- 0x00100000 (1 MB)     *
    |      BIOS ROM       |                              |
    +---------------------+     <- 0x000C0000 (960 KB)   |
    |   16-bit devices    |                              |  UMA (upper memory area)
    |   expansions ROM    |                              |  reserved for system usage
    +---------------------+     <- 0x000C0000 (768 KB)   |
    |     VGA Display     |                              |
    +---------------------+     <- 0x000A0000 (640 KB)   *
    |    Low Memory (u)   |
    +---------------------+     <- 0x00000000   - the first 1KB + 256 bytes are reserved

    when you power on the computer the processor will always jump to 0xFFFF0 (1MB-1)
    - then it will search an initialize any attached devices
    - search for a bootable disk
    - pass control to the bootloader (in our case GRUB)

    x86 compatible processors always starts executing in real mode where
    - they have full privileges
    - 20-bit segmented memory addressing which can only access the first 1MB of physical RAM
    note: in our case GRUB has already enabled protected mode which allows us to address the entire physical RAM up to 4GB

    @note as in linux, the kernel is VIRTUALLY MAPPED in the HIGHER HALF of his virtual address space, starting from 0xC0000000 (3GB)
    |-> ex pa 0x00200000 (2MB) is mapped to va 0xC0000000 (3GB)
*/

#define EXT_MEM_BASE 0x00100000     // extended physical memory address (1MB)
#define KERN_BASE_PHYS 0x00200000   // kernel physical link address (2MB)
#define KERN_BASE_VRT 0xC0000000    // kernel base virtual address (3 GB)

// readability types:
typedef uint32_t pde_t;           // page directory entry
typedef uint32_t pte_t;           // page table entry
typedef uint32_t physaddr_t;      // physical address

/// physical page metadata
/// @param next pointer to the next free page
/// @param ref_count number of virtual memory mappings to this physical page
typedef struct __physical_page_info {
    struct __physical_page_info* next;
    uint16_t ref_count;
} physical_page_metadata_t;

// reference to the pages array defined in kernel/vm.c
extern physical_page_metadata_t* pages;
extern uint32_t npages;

/// returns the page number of a given physical address
static uint32_t page_num(physaddr_t pa) {
    return (PGROUNDDOWN(pa) - EXT_MEM_BASE) >> 12;
}

/// returns the physical address of a given physical page
/// @note currently only the upper memory is mapped so we start at 1MB
static physaddr_t page2pa(physical_page_metadata_t *pp) {
	return EXT_MEM_BASE + ((pp - pages) * PGSIZE);
}

/// returns the metadata of a given physical address
/// @note currently only the upper memory is mapped so we start at 1MB
static physical_page_metadata_t* pa2page(physaddr_t pa) {
    return &pages[(pa - EXT_MEM_BASE) >> 12];
}

/// returns the kernel virtual address (above 3GB) of a given physical address
static void* pa2kva(physaddr_t pa) {
    // panics if you pass a physical address that would map to an invalid physical page
    if (page_num(pa) >= npages) {
        panic("pa2kva");
    }
    // so that kva starts from 3GB
    return (void*)(pa - KERN_BASE_PHYS + KERN_BASE_VRT);
}

/// returns the kernel virtual address (above 3GB) of a given physical page metadata
static void* page2kva(physical_page_metadata_t* pp) {
    return pa2kva(page2pa(pp));
}

/// returns the physical address corresponding to the given kernel virtual address (>3GB)
static physaddr_t kva2pa(void* kva) {
    // panics if you pass virtual addresses < 3GB, which are not kernel virtual addresses
    if ((uintptr_t)kva < KERN_BASE_VRT) {
        panic("kva2pa");
    }
    // so that physical addresses starts from 2MB
    return (physaddr_t)(kva - KERN_BASE_VRT + KERN_BASE_PHYS);
}

/// called once in kernel_main to initialize the virtual memory system
/// @note only called with paging disabled so it uses physical addresses
void vm_setup(uint32_t memlower, uint32_t memupper);

/// called by vm_setup to initialize the physical_page_metadata_t[] pages
/// @note only called with paging disabled so it uses physical addresses
void page_init();

/// called by vm_setup to initiaze the kernel's page directory
void kern_pgdir_init();

/// minimal physical memory allocator that MUST only be used before the actual paging system is running
/// @note only called with paging disabled so it uses physical addresses
/// @param n bytes to allocate (rounded up to the next page)
void* boot_alloc(uint32_t n);

/// returns the next free physical page
/// @note works with kernel virtual addresses
/// @param zero if set zero-out the entire physical page
/// @param usekva if set uses the kernel virtual address of the page (assuming paging is turned on)
physical_page_metadata_t* kalloc(int zero, int usekva);

/// attempts to free the given physical page if ref_count == 0, does nothing if not possible to free it
/// @note works with kernel virtual addresses
/// @param pp physical page to free
void kfree(physical_page_metadata_t* pp);

/// returns a pointer to the pte (page table entry) of given virtual address (va)
/// @param pgdir pointer to the page directory to use
/// @param va virtual address
/// @param create if set, in case va isn't mapped, allocates a physical page for the page table
/// @return kernel virtual address of the pte
pte_t* pgdir_walk(pde_t* pgdir, uintptr_t va, int create);

/// returns the physical address of a given virtual address
/// @param pgdir pointer to the page directory to use
/// @param va virtual address to translate
/// @return the physical address if va is mapped in pgdir otherwise NULL
physaddr_t va_to_pa(pde_t* pgdir, uintptr_t va);

/// maps the given physical page at virtual address va in the provided page directory
/// @param pgdir pointer to the page directory to use
/// @param pp pointer to the physical page metadata to map
/// @param va virtual address
/// @return 1 on success and 0 on failure
int map_pp_to_va(pde_t* pgdir, physical_page_metadata_t* pp, uintptr_t va);

/// loads the provided pgdir into the CR3 register, used in scheduling to context-switch
/// @param pgdir pointer to the page directory to load
void pgdir_load(pde_t* pgdir);

#endif // !VM_H