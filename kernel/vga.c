#include "inc/vga.h"
#include "../learnixlibc/inc/string.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

static inline uint8_t
vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t
vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

/*
	initializes a 25x80 terminal
*/
void
terminal_initialize()
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void
terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

/* puts a characther at the specified position */
void
terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

// terminal_column is already 0 when called
void terminal_scroll() {
	// set active row to the last one
	terminal_row = VGA_HEIGHT - 1;

	// move all rows, except the first one, up by 1
	for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
		// using hardware optimized memcpy implementation for faster performances
		memcpy(terminal_buffer + y * VGA_WIDTH, terminal_buffer + (y + 1) * VGA_WIDTH, VGA_WIDTH * sizeof(uint16_t));
	}

	// clean last row
	for (size_t x = 0; x < VGA_WIDTH; x++) {
		size_t cur = terminal_row * VGA_WIDTH + x;
		terminal_buffer[cur] = vga_entry(' ', terminal_color); // @todo refactor with a memset optimized implementation
	}
}

/* writes a single characther at the current position on the terminal */
void
terminal_putchar(char c)
{
    switch(c) {
        case '\n': {
            terminal_column = 0;
            if(++terminal_row == VGA_HEIGHT)
                terminal_scroll();
            break;
        }
        case '\b': {
            if(terminal_column == 0) {
                terminal_row--;
                terminal_column = VGA_WIDTH;
            }
            terminal_putentryat(' ', terminal_color, --terminal_column, terminal_row);
            break;
        }
        default: {
            terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
            if (++terminal_column == VGA_WIDTH) { // end of row
                terminal_column = 0;
                if (++terminal_row == VGA_HEIGHT) // last row
                    terminal_scroll();
            }
            break;
        }
    }

}

/* writes a buffer of size bytes on the terminal */
void
terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

/* writes the provided string on the terminal */
void
terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}
