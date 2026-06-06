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

char* strcpy(char *dst, const char *src)
{
    char* output = dst;
    while (*src != '\0')
    {
        *output++ = *src++;
    }

    *output = '\0';
    
    return dst;
}

char* strncpy(char *dst, const char *src, size_t n)
{
    char* ret = dst;

    while (n > 0 && *src != '\0')
    {
        *dst++ = *src++;
        n--;
    }

    while (n > 0)
    {
        *dst++ = '\0';
        n--;
    }

    return ret;
}

void* memcpy(void* dst, const void* src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    while (n--)
    {
        *d++ = *s++;
    }

    return dst;
}

void* memset(void *dst, char ch, size_t size)
{
    char* ptr = (char*)dst;
    while (size--) {
        *ptr++ = ch;
    }
    return dst;
}