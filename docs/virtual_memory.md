# Virtual Memory
The virtual memory system provides an **abstraction over physical memory**, allowing the kernel (and user programs) to operate in isolated and protected address spaces.<br>
It enables features such as memory protection, paging, and dynamic allocation by mapping virtual addresses to physical memory using page tables

## Intro to x86 Paging
LearnixOS uses the standard x86 Intel paging system where each virtual address is split into 3 parts:
- PDX (Page Directory Index): specifies the index of the page directory entry
- PTX (Page Table Index): specifies the index of the page table entry
- OFFSET: specifies the 4KB offset inside the selected page

Visually:
```c
    ----------------------
    | PDX | PTX | OFFSET |
    ----------------------
   31    22    12        0
```

## Memory Layout

### Physical Address Space

```c
    +---------------------+  0x00000000
    |   real mode stuff   |
    +---------------------+  0x00100000 (1 MB)
    |   pages[]           |  <- metada of physical page allocator
    |   ...               |
    +---------------------+ 0x002000000 (2 MB)
    |   kernel code       |
    |   ...               |
    +---------------------+ end of physical RAM
```

### Virtual Address Space

```c
    +---------------------+  0x00000000
    |                     |
    |      userland       |
    |     (in future)     |
    |                     |
    +---------------------+  0xC0000000
    |                     |
    |    kernel code      |
    |       pages         |
    |                     |
    +---------------------+ PGROUNDUP(kern_end)
    |                     |
    |   kernel heap       |
    |                     |
    +---------------------+ 0xFFFFFFFF
```

## Page Allocator
This section describes in details how the virtual memory system of Learnix86 works.

### Physical Page Management
LearnixOS tracks physical memory using a global `pages` array, where each entry corresponds to a 4 KiB physical page frame. It is placed early in physical memory (after 1 MB) so it can be identity-mapped during early boot.

Each entry of `pages` is a `physical_page_metadata_t`, defined as follows in vm.h:
```c
typedef struct __physical_page_info {
    struct __physical_page_info* next;
    uint16_t ref_count;
    uint16_t flags;
} physical_page_metadata_t;
```

Aa simple **free list** algorithm is used to service page allocation requests:
- pages are linked via the `next` field of `physical_page_metadata_t`
- the kernel only needs to track the next free physical page through the `pages_free_list` variable

It exposes the following two functions:
```c
physical_page_metadata_t* page_alloc();
```
Takes the next free physical page pointed by `pages_free_list`, marks it as allocated and updates the free list.

<br>

```c
physical_page_metadata_t* page_free(physical_page_metadata_t *pp);
```
Attempts to free the provided physical page and returns it, on error it returns `NULL`.
- a page can only be freed if it is not a kernel code page and his reference count is 0

### Virtual Memory APIs
LearnixOS provides a small set of functions to manage virtual-to-physical mappings by manipulating page directory and page table entries. These functions form the core of the virtual memory system and are used by higher-level memory allocators and system initialization code.


```c
pte_t* pgdir_walk(pde_t* pgdir, uintptr_t va, int create);
```
Traverse the provided page directory (`pgdir`) and returns a pointer to the page table entry (PTE) of the virtual address `va`.
- if `va` is not mapped in `pgdir` and `create` is set, a physical page gets allocated and `va` gets mapped

<br>

```c
physaddr_t va_to_pa(pde_t* pgdir, uintptr_t va);
```
Returns the physical address associated to the virtual address `va` inside the page directory `pgdir`.
- internally uses `pgdir_walk()` with `create = 0` to avoid mapping `va` if not present
- it returns `NULL` if `va` is not mapped in `pgdir`

<br>

```c
void map_va(pde_t* pgdir, uintptr_t va, physaddr_t pa);
```
Maps the physical address `pa` at virtual address `va` inside the page directory `pgdir`.
- internally it uses `pgdir_walk()` with `create = 1` to guarantee that `va` is mapped
- silently fails on error

<br>

```c
void map_pp(pde_t* pgdir, uintptr_t va, physical_page_metadata_t* pp);
```
Maps the physical page `pp` at virtual address `va` inside the page directory `pgdir`.
- just a wrapped around `map_va` that uses `page2pa(pp)` as the `pa` parameter 

<br>

```c
void unmap_va(pde_t* pgdir, uintptr_t va);
```
Removes the mapping of the virtual address `va` and frees the physical page allocated for his PTE.

## Sources
- https://wiki.osdev.org/Paging#32-bit_Paging_(Protected_Mode)