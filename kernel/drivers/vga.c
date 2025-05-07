#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <learnix/drivers/vga.h>

static uint32_t terminal_row;
static uint32_t terminal_column;
static uint8_t terminal_color;
static uint16_t *terminal_buffer;

void
terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color
	    = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	for (uint32_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (uint32_t x = 0; x < VGA_WIDTH; x++)
		{
			const uint32_t index = y * VGA_WIDTH + x;
			terminal_buffer[index]
			    = vga_entry(' ', terminal_color);
		}
	}
}

void
terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void
terminal_putentryat(unsigned char c, uint8_t color, uint32_t x, uint32_t y)
{
	const uint32_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void
terminal_scroll()
{
	terminal_row = VGA_HEIGHT - 1; // set active row to the last one

	// move all rows except the first up by 1
	for (uint32_t y = 0; y < VGA_HEIGHT - 1; y++)
	{
		memcpy(terminal_buffer + y * VGA_WIDTH,
		       terminal_buffer + (y + 1) * VGA_WIDTH,
		       VGA_WIDTH * sizeof(uint16_t));
	}

	// clean last row
	for (uint32_t x = 0; x < VGA_WIDTH; x++)
	{
		uint32_t curr = terminal_row * VGA_WIDTH + x;
		terminal_buffer[curr] = vga_entry(' ', terminal_color);
	}
}

void
terminal_putchar(char c)
{
	switch (c)
	{
	// newline
	case '\n':
	{
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_scroll();
		break;
	}
	// delete
	case '\b':
	{
		if (terminal_column == 0)
		{
			terminal_row--;
			terminal_column = VGA_WIDTH;
		}
		terminal_putentryat(' ', terminal_color, --terminal_column,
		                    terminal_row);
		break;
	}
	// everything else
	default:
	{
		terminal_putentryat((unsigned char)c, terminal_color,
		                    terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH)
		{
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_scroll();
		}
		break;
	}
	}
}

void
terminal_write(const char *data, uint32_t size)
{
	for (uint32_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void
terminal_writestring(const char *data)
{
	terminal_write(data, strlen(data));
}