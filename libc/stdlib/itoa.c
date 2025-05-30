#include <stdbool.h>
#include <stdlib.h>

char *
itoa(int value, char *str, int base)
{
	char *digits = "0123456789ABCDEF";
	bool is_negative = false;
	int i = 0;

	if (value < 0 && base == 10)
	{
		is_negative = true;
		value = -value;
	}

	do
	{
		str[i++] = digits[value % base];
		value /= base;
	} while (value > 0);

	if (is_negative)
		str[i++] = '-';

	str[i] = '\0';

	// invert the string
	for (int j = 0; j < i / 2; j++)
	{
		char tmp = str[j];
		str[j] = str[i - j - 1];
		str[i - j - 1] = tmp;
	}

	return str;
}

char *
itoal(uint32_t value, char *str, int base)
{
	char *digits = "0123456789ABCDEF";
	int i = 0;

	do
	{
		str[i++] = digits[value % base];
		value /= base;
	} while (value > 0);

	// pad hex numbers to 8 digits (to display 32 bit addresses better)
	if (base == 16)
	{
		while (i < 8)
		{
			str[i++] = '0';
		}
		str[i++] = 'x';
		str[i++] = '0';
	}

	str[i] = '\0';

	// invert the string
	for (int j = 0; j < i / 2; j++)
	{
		char tmp = str[j];
		str[j] = str[i - j - 1];
		str[i - j - 1] = tmp;
	}

	return str;
}
