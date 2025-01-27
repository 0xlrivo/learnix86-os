#ifndef LEARNIXLIBC_STRING_H
#define LEARNIXLIBC_STRING_H

#include <stddef.h>

/* copies size bytes from src to dest without checking for their overlapping */
void* memcpy(void* restrict dst, const void* restrict src, size_t size);

/* sets a memory block to the specified value */
void* memset(void* ptr, int value, size_t num);

/* copies the C string pointed by source into the one pointed by destination including the terminating character */
char* strcpy(char* destination, const char* source);

/*
    compares two C strings
    returns:
        <0 when ptr2 > ptr1
        0  when ptr1 == ptr2
        >0 when ptr1 < ptr2
*/
int strcmp(const char* str1, const char* str2);

/* returns thte lenght of the C string pointed by buf */
size_t strlen(const char* buf);


#endif
