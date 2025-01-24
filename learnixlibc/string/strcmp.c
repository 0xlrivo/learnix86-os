#include "../inc/string.h"

int strcmp(const char* str1, const char* str2) {
    const char* dummy1 = str1;
    const char* dummy2 = str2;
    
    // untill both strings have characthers left
    while (*dummy1 != '\0' && *dummy2 != '\0') {
        if (*dummy1 != *dummy2) 
            return *dummy1 < dummy2 ? -1 : 1;
        dummy1++;
        dummy2++;
    }

    // if they both terminated they're equals
    if (*dummy1 == '\0' && *dummy2 == '\0')
        return 0;
    else if (*dummy1 == '\0')
        return -1;
    else
        return 1;
}