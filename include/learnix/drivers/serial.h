#ifndef SERIAL_H
#define SERIAL_H

// https://wiki.osdev.org/Serial_Ports#Programming_the_Serial_Communications_Port

#define COM1 0x3F8
#define COM1_INTERRUPT_ENABLE_REGISTER (COM1 + 1)
#define COM1_DIVISOR_LSB_REGISTER (COM1)
#define COM1_DIVISOR_MSB_REGISTER (COM1 + 1)
#define COM1_INTERRUPT_VERIFICATION_REGISTER (COM1 + 2)
#define COM1_FIFO_CONTROL_REGISTER (COM1 + 2)
#define COM1_DLAB_REGISTER (COM1 + 3)
#define COM1_MODEM_CONTROL_REGISTER (COM1 + 4)
#define COM1_LINE_STATUS_REGISTER (COM1 + 5)        // useful to check for errors and enable polling
#define COM1_MODEM_STATUS_REGISTER (COM1 + 6)
#define COM1_SCRATCH_REGISTER (COM1 + 7)

// initializes the serial port COM1
int serial_init();

// writes a single byte on the serial stream
void serial_writechar(char c);

// writes a C string on the serial stream
void serial_write(const char* str);

// writes a formatted string on the serial stream
void serial_printf(const char *format, ...);

#endif
