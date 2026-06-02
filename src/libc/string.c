#include "libc/string.h"

int strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    int result = *(const unsigned char*)s1 - *(const unsigned char*)s2;
    if (result < 0) return -1;
    if (result > 0) return 1;
    return 0;
}

char *strcpy(char *dst, const char *src)
{
    char* output = dst;
    while (*src != '\0')
    {
        *output++ = *src++;
    }

    *output = '\0';
    
    return dst;
}