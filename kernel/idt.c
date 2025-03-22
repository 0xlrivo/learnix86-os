#include <string.h>
#include <stdint.h>
#include <learnix/idt.h>
#include <learnix/pic.h>
#include <learnix/x86/x86.h>
#include <learnix/drivers/keyboard.h>

idtr_t idtr;
idte_t idt[IDT_ENTRIES]; // IDT

extern void irq1_wrapper();

// called by kernel_main once during OS setup
void idt_init() {
    // zero-out the IDT entries
    memset(&idt, 0, sizeof(idte_t) * IDT_ENTRIES);

    // setup interrupt service routines
    idt_set_gate(IRQ1_IDX, (uint32_t)irq1_wrapper, 0x08, 0x8E); // keyboard handler (IRQ1)

    // and finally load the IDT into the IDTR register
    idt_load();
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

void irq1_handler() {
    keyboard_main();
    pic_send_eoi(1);
}