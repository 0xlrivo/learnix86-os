#include <stdio.h>
#include <stdlib.h>

void
panic(const char *reason)
{
	puts(reason);
	while (1)
	{
	};
}