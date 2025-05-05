#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <learnix/idt.h>
#include <learnix/pic.h>
#include <learnix/x86/x86.h>
#include <learnix/drivers/keyboard.h>

idtr_t idtr;
idte_t idt[IDT_ENTRIES]; // IDT

// ISR0: division by zero
__attribute__((interrupt))
void 
division_by_zero_exception(interrupt_frame_t* frame)
{
	printf("[Exception] division by zero at %x\n", frame->ip);
	// halt CPU
	asm volatile ("hlt");
}

// IRS3: breakpoint (INT3 instruction)
// frame->ip points to the byte after the INT3 instruction
__attribute__((interrupt))
void
breakpoint_exception(interrupt_frame_t* frame)
{
	printf("Breakpoint at %x", frame->ip);
}

// ISR 14: page fault
__attribute__((interrupt))
void
page_fault_exception(interrupt_frame_t* frame, uint32_t error_code)
{
	printf("Page fault at %x\n", frame->ip);
	// check P bit (0th of error_code)
	if ((error_code & 0x1) == 0)
	{
		puts("non-present page\n");
	}
	else
	{
		puts("page-protection violation\n");
	}
	asm volatile ("hlt");
}

extern void irq1_wrapper();

void 
irq1_handler() {
    keyboard_main();
    pic_send_eoi(1);
}

static inline void idt_load() {
    // setup IDTR register
    idtr.base = (uint32_t)&idt;
    idtr.limit = (sizeof(idte_t) * IDT_ENTRIES) - 1;

    // load the table with the lidt instruction
    asm volatile ("lidt (%0)" : : "r" (&idtr));

    // enable interrupts
    asm volatile ("sti");
}

static inline void idt_set_gate(int n, uint32_t handler, uint16_t selector, uint8_t type_attributes) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = selector;
    idt[n].zero = 0;
    idt[n].type_attributes = type_attributes;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

// called by kernel_main once during OS setup
void idt_init() {
    // zero-out the IDT entries
    memset(&idt, 0, sizeof(idte_t) * IDT_ENTRIES);
	
	// 0: division by zero
	idt_set_gate(0, (uint32_t)division_by_zero_exception, 0x08, 0x8E);
	// 3: breakpoint (INT3 instruction)
	idt_set_gate(3, (uint32_t)breakpoint_exception, 0x08, 0x8E);
	// 14: page fault
	idt_set_gate(14, (uint32_t)page_fault_exception, 0x08, 0x8E);

    // setup interrupt service routines
    idt_set_gate(IRQ1_IDX, (uint32_t)irq1_wrapper, 0x08, 0x8E); // keyboard handler (IRQ1)

    // and finally load the IDT into the IDTR register
    idt_load();
}
