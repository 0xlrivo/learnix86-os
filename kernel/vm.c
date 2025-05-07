#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>
#include <learnix/drivers/serial.h>

// points to the first byte after kernel code
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

void
test_vm_system()
{
	// TEST #1 -> map_va() that triggers pgdir_wa() -> page_alloc()
	// because no page table entry is currently allocated for va
	physical_page_metadata_t* page_dir_phys_page = pages_free_list;
	map_va(kern_pgdir, 0xC0400000, 0x00300000);
	if (pages_free_list == page_dir_phys_page)
		panic("VM TEST #1");
	serial_printf("0xC0400000 -> %x\n", va_to_pa(kern_pgdir, 0xC0400000));
	dbg_dump_pgdir(kern_pgdir, "kernel");
	
	// TEST #2 -> unmap_va() that page_free() the page directory
	// erased
	serial_printf("TEST #2: unmap_va()");
	unmap_va(kern_pgdir, 0xC0400000);
	if (pages_free_list != page_dir_phys_page)
		panic("VM TEST #2");
	dbg_dump_pgdir(kern_pgdir, "kernel");
	serial_printf("pages_free_list: %x", page2pa(pages_free_list));

	printf("[ OK ] VM TEST PASSED!\n");
}

void vm_setup(uint32_t memlower, uint32_t memupper) {
    // setup the kern_pgdir
    kern_pgdir = (pte_t*)boot_page_directory;

	// setup RECURSIVE MAPPING
	// kern_pgdir[1023] -> kern_pgdir[0]
	// this allows the kernel to reference
	// all the page tables
	kern_pgdir[1023] = rcr3() | PTE_P | PTE_W;

    // compute upper memory total of physical pages
    npages = (memupper * 1024) / PGSIZE;
    
    serial_printf("[LOG] memupper: %d KB for a total of %d pages\n", memupper, npages);

    // initialize the physical page tracking structure
    pages_setup();
	
	// TEST
	test_vm_system();
}

// @todo also allocate low-mem pages after parsing multiboot struct
void pages_setup() {
    uint32_t i, va, pa;

    // allocate space for pages[] starting from 1MB physical
    pages = (physical_page_metadata_t*)pa2kva(EXT_MEM_BASE);

    // how many pages are needed to contain the pages[] array... must virtually map those to avoid faults
    uint32_t pages_to_map = ((npages * sizeof(physical_page_metadata_t)) / PGSIZE) + 1;
	
	serial_printf("[LOG] mapping pages[]\n");
    // and map those starting from 1MB physical 
    for (i = 0, va = pages, pa = EXT_MEM_BASE; i < pages_to_map; i++, va += PGSIZE, pa += PGSIZE) {
        map_va(kern_pgdir, va, pa);
    }
	serial_printf("[LOG] finished mapping pages[]\n");

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

physical_page_metadata_t* page_alloc() {
    // fetch the next free page from the linked list
    physical_page_metadata_t* pp = pages_free_list;

    // panic if out-of-pages
    if (pp == NULL)
        panic("page_alloc: out of physical pages");

    // update the free_page_list
    pages_free_list = pp->next;

    // clear pp->next
    pp->next = NULL;

    // return a pointer to the page's metadata
    return pp;
}

physical_page_metadata_t* page_free(physical_page_metadata_t* pp) {
	// pages can only be freed if 
	// 1) they are NOT kernel pages (flags != PPM_KERNEL)
	// 2) they have a reference count of 0
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
		// ISSUE: this too must be mapped in the kernel's virtual address space
		// otherwise any call of this function that triggered page_alloc()
		// will page fault
		// SOLUTION: apparently recursive mapping solves it
        physical_page_metadata_t* pp = page_alloc();
		serial_printf("[DEBUG] pgdir_walk allocated pa %x\n", page2pa(pp));
       	
		// get the physical address of pp
        pt = PTE_ADDR(page2pa(pp));

        // update the page directory
        pgdir[pdx] = pt | PTE_P | PTE_U | PTE_W;
    }
	
	// recursively mapped virtual address
	// to access the page directory at pdx
	pte_t* pt_va = PT_VADDR(pdx);
	// return the actual PTE address
    return &pt_va[ptx];
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

	//serial_printf("va: %x pa: %x -> pte: %x\n", va, pa, pte);
}

/// maps the given physical page to va
void map_pp(pde_t* pgdir, physical_page_metadata_t* pp, uintptr_t va) {
    map_va(pgdir, va, page2pa(pp));
}

// TODO: test this function
// removes the mapping of va from pgdir
void
unmap_va(pde_t* pgdir, uintptr_t va)
{
	// round down va to the nearest page
	va = PGROUNDDOWN(va);
	// try to get va's PTE
	pte_t* pte = pgdir_walk(pgdir, va, 0);
	// if pte == NULL va is not mapped
	// so we return early
	if (pte == NULL)
		return;

	// remove va's mapping
	*pte = 0;

	// extract the page directory index
	// from va -> needed for PT_VADDR
	uint32_t pdx = PDX(va);
	
	pte_t* pgtable = PT_VADDR(pdx);
	// check if the entire page table of va
	// is empty
	for (uint32_t ptx = 0; ptx < 1024; ptx++)
	{
		if (pgtable[ptx] & PTE_P)
		{
			// if at least one page is present
			// we're done since we can't
			// free the page directory
			serial_printf("[LOG] unmap_va() DONE without page_free()\n");
			return;
		}
	}

	// if the for loop completed the
	// page directory is completely
	// empty and thus can be freed
	physaddr_t pgtable_phys = PTE_ADDR(pgdir[pdx]);
	serial_printf("pgtablephys: %x\n", pgtable_phys);
	pgdir[pdx] = 0;
	page_free(pa2pp(pgtable_phys));
	tlbflush();	
}

// debugging utility that dumps the content
// of a given page table over UART
// NOTE: pgdir will be always mapped in kernel virtual address space
// but this is not true for the page tables, for which recursive
// mapping will be used
void
dbg_dump_pgdir(pde_t* pgdir, const char* pgdir_name)
{
	serial_printf("[DEBUG] Dumping page directory: %s\n", pgdir_name);
	
	// for every page directory index
	for (uint32_t pdx = 0; pdx < 1024; pdx++)
	{
		// cache the page directory entry
		// at current pdx
		pde_t pde = pgdir[pdx];

		// check if present
		if (pde & PTE_P)
		{
			// extract ONLY the physical address
			// from the page directory entry
			// by masking out the flags
			uint32_t pt_phys = pde & 0xFFFFF000;
			// convert it to a kernel virtual address
			// using recursive mapping
			pte_t* pgtable = PT_VADDR(pdx); 
			
			serial_printf("PDE[%d] -> PT@%x\n", pdx, pt_phys);

			// for every page table index
			for (uint32_t ptx = 0; ptx < 1024; ptx++)
			{
				// cache the page table entry
				// at the current ptx
				pte_t pte = pgtable[ptx];

				// check if present
				if (pte & PTE_P)
				{
					uintptr_t va = (pdx << 22) | (ptx << 12);
					physaddr_t pa = pte & 0xFFFFF000;
					serial_printf("| PTE[%d] -> VA %x -> PA %x\n", ptx, va, pa);
				}
			}
		}
	}
}
