#include <learnix/vm.h>
#include <learnix/x86/x86.h>
#include <learnix/x86/mmu.h>

/*
 * @brief x86 virtual memory implementation
*/

pde_t *kern_pgdir;  // kernel's page directory