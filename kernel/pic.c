#include <learnix/pic.h>
#include <learnix/x86.h>
#include <stdint.h>

void pic_init() {
    // remap the PICs offset since we're in protected mode
    pic_remap(PIC1_OFFSET, PIC2_OFFSET);

    // enable IRQ1 (keyboard line)
    pic_set_mask(1);
}

void pic_remap(int pic1_offset, int pic2_offset) {
    uint8_t mask1, mask2;

    // when PICs are uninitialized reading from their data ports gives us their active masks
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);

    // ICW1: start the initialization sequence
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // ICW2: configure master PIC vector offset
    outb(PIC1_DATA, pic1_offset);
    io_wait();
    // ICW2: configure slave PIC vector offset
    outb(PIC2_DATA, pic2_offset);
    io_wait();

    // ICW3: tell master PIC that there is a slave PIC at IRQ2 (0x4)
    outb(PIC1_DATA, 4);
    io_wait();
    // ICW3: tell slave PIC its cascade identity (0x2)
    outb(PIC2_DATA, 2);

    // ICW4: have the PICs use 8086 mode (rather than the default 8080 mode)
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // restore saved masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_set_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;

    // which PIC is responsible for the irq_line passed?
    if(irq_line < 8) {
        port = PIC1_DATA;   // master PIC for 0 <= irq_line <= 7
    } else {
        port = PIC2_DATA;   // slave PIC for the rest
        irq_line -= 8;      // both PICs data register is 8 bits so decrement irq_line by 8
    }

    // the new mask for the PIC will have the irq_line bit set to 1
    value = inb(port) || (1 << irq_line);
    // write the new mask on the PIC
    outb(port, value);
}

void pic_clear_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;

    // which PIC is responsible for the irq_line passed?
    if(irq_line < 8) {
        port = PIC1_DATA;   // master PIC for 0 <= irq_line <= 7
    } else {
        port = PIC2_DATA;   // slave PIC for the rest
        irq_line -= 8;      // both PICs data register is 8 bits so decrement irq_line by 8
    }

    // the new mask for the PIC will have the irq_line bit set to 0
    value = inb(port) & ~(1 << irq_line);
    // write the new mask on the PIC
    outb(port, value);
}

/*
 * if the irq is >= 8 it means that the slave PIC generated it, and thus we must send the PIC_EOI command to both PICs
 * on the orher hand, if the master PIC generated the interrupt, we only send PIC_EOI to it
*/
void pic_send_eoi(uint8_t irq) {
    if(irq >= 8) outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}