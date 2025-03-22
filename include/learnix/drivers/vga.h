#ifndef LEARNIX_VGA_H
#define LEARNIX_VGA_H

#include <stdint.h>

static const uint32_t VGA_WIDTH = 80;
static const uint32_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*)0xC03FF000;	// @note paging mapping of the VGA buffer's physical location

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

/// @brief called by kernel_main to initialize the VGA terminal
void terminal_initialize(void);

/// @brief prints a single characther on screen
/// @param c character to print
void terminal_putchar(char c);

/// @brief prints a string given its lenght
/// @param data characthers buffer
/// @param size buffer length
void terminal_write(const char* data, uint32_t size);

/// @brief prints a NULL-terminated C string 
/// @param data NULL-terminated C string
void terminal_writestring(const char* data);

#endif