#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "pic.h"
#include <stdint.h>

/* The number of entries of the IDT */
#define IDT_ENTRIES 256

/* IDT indexes for IRQ entries */
#define IRQ1_IDX (PIC1_OFFSET + 1)
#define KEYBOARD_PORT 0x60

/* IDTR register */
typedef struct _idt_register {
   uint16_t limit;            // sizeof(IDT) - 1
   uint32_t base;             // base address where the IDT is loaded
} __attribute__((packed)) idtr_t;

/* Interrupt Descriptor Table Entry */
typedef struct _idt_entry {
   uint16_t offset_low;        // offset bits 0..15
   uint16_t selector;          // a code segment selector in GDT or LDT
   uint8_t  zero;              // unused, set to 0
   uint8_t  type_attributes;   // gate type, dpl, and p fields
   uint16_t offset_high;       // offset bits 16..31
} __attribute__((packed)) idte_t;

void idt_init();

void idt_load();

void idt_set_gate(int n, uint32_t handler, uint16_t selector, uint8_t type_attributes);

extern void keyboard_isr_wrapper();
void keyboard_handler();

#endif // ! INTERRUPTS_H
