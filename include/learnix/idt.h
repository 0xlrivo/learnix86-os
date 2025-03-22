#ifndef IDT_H
#define IDT_H

#include <learnix/pic.h>
#include <stdint.h>

/* The number of entries of the IDT */
#define IDT_ENTRIES 256

/* IDT Indexes */
#define IRQ0_IDX (PIC1_OFFSET)   // due to protected mode PIC remapping
#define IRQ1_IDX (IRQ0_IDX + 1)

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

static inline void idt_load();

static inline void idt_set_gate(int n, uint32_t handler, uint16_t selector, uint8_t type_attributes);

void irq1_handler();

#endif // ! INTERRUPTS_H