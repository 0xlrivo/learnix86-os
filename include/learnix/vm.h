#ifndef VM_H
#define VM_H

#include <stdint.h>

// define a pde_t type for readability purposes in vm.c
typedef uint32_t pde_t;
typedef uint32_t pte_t;

// called once in kernel_main to setup virtual memory
void vm_setup();

// returns the corresponding PTE to va in the provided pgdir
// if alloc != 0 create such page if not existing
pte_t *pgdir_walk(pde_t *pgdir, void *va, int alloc);

#endif // !VM_H