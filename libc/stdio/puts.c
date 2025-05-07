#include <stdio.h>

int
puts(const char *string)
{
	for (int i = 0; string[i] != '\0'; i++)
		putchar(string[i]);
}