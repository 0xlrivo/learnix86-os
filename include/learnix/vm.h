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

// readability types:
typedef uint32_t pde_t;           // page directory entry
typedef uint32_t pte_t;           // page table entry
typedef uint32_t physaddr_t;      // physical address

// known memory locations:
#define EXT_MEM_BASE 0x00100000     // extended physical memory address (1MB)
#define KERN_BASE_PHYS 0x00200000   // kernel physical link address (2MB)
#define KERN_BASE_VRT 0xC0000000    // kernel base virtual address (3 GB)

// physical page metadata flags:
#define PPM_KERN 0x000F             // is a kernel's code physical page

/// returns the corresponding kernel virtual address (>3GB) of the physical address provided
/// @param pa the physical address to translate
inline void* pa2kva(physaddr_t pa) {
    return (void*)(pa + KERN_BASE_VRT);
}

/// physical page metadata
/// @param next pointer to the next free page
/// @param ref_count number of virtual memory mappings to this physical page
typedef struct __physical_page_info {
    struct __physical_page_info* next;
    uint16_t ref_count;
    uint16_t flags;
} physical_page_metadata_t;

/// called once in kernel_main to initialize the virtual memory system
void vm_setup(uint32_t memlower, uint32_t memupper);

/// called by vm_setup() to initialize the pages[] array
void pages_setup();

/// returns a pointer to the pte (page table entry) of given virtual address (va)
/// @param pgdir pointer to the page directory to use
/// @param va virtual address
/// @param create if set, in case va isn't mapped, allocates a physical page for the page table
/// @return kernel virtual address of the pte
pte_t* pgdir_walk(pde_t* pgdir, uintptr_t va, int create);

/// returns the physical address of va
physaddr_t va_to_pa(pde_t* pgdir, uintptr_t va);

#endif // !VM_H