#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/*
    SOURCES:
    - https://wiki.osdev.org/8259_PIC#Programming_the_PIC_chips
    - https://pdos.csail.mit.edu/6.828/2005/readings/hardware/8259A.pdf
*/

/* 8259 PIC magic values */
#define PIC1_COMMAND 0x20   // master PIC
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0   // slave PIC
#define PIC2_DATA    0xA1

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define PIC1_OFFSET 0x20                // master PIC offset for protected mode
#define PIC2_OFFSET (PIC1_OFFSET + 8)   // slave PIC offset for protected mode

/* 8259 PIC commands */
#define PIC_EOI 0x20

/* called by kernel_main during setup to initialize the PICs */
void pic_init();

/* remaps the PIC offsets to enable its use in protected mode */
void pic_remap(int master_offset, int slave_offset);

/* enables the specified IRQ line */
void pic_set_mask(uint8_t irq_line);

/* disables the specified IRQ line */
void pic_clear_mask(uint8_t irq_line);

/* sends the End-Of-Interrupt command */
void pic_send_eoi(uint8_t irq);

#endif // !PIC_H: