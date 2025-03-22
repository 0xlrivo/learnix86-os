#include <stdarg.h>
#include <string.h>
#include <learnix/drivers/serial.h>
#include <learnix/x86/x86.h>

// @note MUST be called after pic_init() to avoid weird outputs
int serial_init() {
	// disable interrupts
	outb(COM1_INTERRUPT_ENABLE_REGISTER, 0x00);

	// set DLAB most significant bit to 1 to allow access to DLAB register
	outb(COM1_DLAB_REGISTER, 0x80);	
	
	// set divisor to 3 -> 38400 baud
	outb(COM1_DIVISOR_LSB_REGISTER, 0x03);
	outb(COM1_DIVISOR_MSB_REGISTER, 0x00);

	// 8 bits, no parity, one stop bit
	// set bits 2-1-3 of DLAB register to 1
	outb(COM1_DLAB_REGISTER, 0x03);
	
	// set FIFO, clear transmit, clear receive
	outb(COM1_FIFO_CONTROL_REGISTER, 0xC7);

	// IRQs enabled, RTS/DSR set
	outb(COM1_MODEM_CONTROL_REGISTER, 0x0B);

	// set in loopback mode to test it
	outb(COM1_MODEM_CONTROL_REGISTER, 0x1E);
	// send byte 0xAE
	outb(COM1, 0xAE);
	// check if serial is faulty (i.e: byte received != 0xAE)
	if(inb(COM1) != 0xAE) return -1;

	// if COM1 is working set it in normal operation mode
	// (no loopback, IRQs enabled, OUT#1 and OUT#2 bits enabled)
	outb(COM1_MODEM_CONTROL_REGISTER, 0x0F);

	return 0;
}

void serial_writechar(char c) {
	// spin-wait untill TEMT bit is set (Transmitter Empty)
	while((inb(COM1_LINE_STATUS_REGISTER) & 0x20) == 0);
	
	// write characther to COM1 port
	outb(COM1, (uint8_t)c);
}

void serial_write(const char* str) {
	while(*str != '\0') {
		serial_writechar((uint8_t)*str);
		str++;
	}
}

void serial_printf(const char *format, ...) {
	// starg.h defined list of arguments
    va_list args;
    // initialize the list of arguments with the latest known argument (format at the beginning)
    va_start(args, format);

    // used to store itoa translations
    char buffer[32];

    // loop through the format string
    for (const char *c = format; *c != '\0'; c++) {
        // if c isn't '%' simply print it as it is
        if (*c != '%') {
            serial_writechar(*c);
        } 
        // otherwise read the next character to determine the format of the next argument
        else {
            c++;
            switch (*c) {
                // characther
                case 'c': {     
                    serial_writechar(va_arg(args, int));
                    break;
                }
                // string
                case 's': {
                    serial_write(va_arg(args, char*));
                    break;
                }
                // int
                case 'd': {
                    itoa(va_arg(args, int), buffer, 10);
                    serial_write(buffer);
                    break;
                }
                // hex
                case 'x': {
                    itoa(va_arg(args, int), buffer, 16);
                    serial_write(buffer);
                    break;
                }
                default: {
                    serial_writechar('%');
                    serial_writechar(*c);
                    break;
                }
            }
        }
    }
}