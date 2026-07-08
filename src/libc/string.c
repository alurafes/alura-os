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

char* strcpy(char* dst, const char* src)
{
    char* output = dst;
    while (*src != '\0')
    {
        *output++ = *src++;
    }

    *output = '\0';
    
    return dst;
}

char* strncpy(char* dst, const char* src, size_t n)
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
    unsigned char* d = (unsigned char *)dst;
    const unsigned char* s = (const unsigned char *)src;

    while (n--)
    {
        *d++ = *s++;
    }

    return dst;
}

void* memset(void* dst, char ch, size_t size)
{
    char* ptr = (char*)dst;
    while (size--) {
        *ptr++ = ch;
    }
    return dst;
}

static int is_delimiter(char c, const char* delimiter)
{
    while (*delimiter) {
        if (c == *delimiter) {
            return 1;
        }
        delimiter++;
    }
    return 0;
}

char* strtok(char* str, const char* delimiter) {
    static char* last_pos = NULL;
    if (str != NULL) last_pos = str;

    if (last_pos == NULL || *last_pos == '\0') return NULL;

    while (*last_pos && is_delimiter(*last_pos, delimiter))
    {
        last_pos++;
    }

    if (*last_pos == '\0') return NULL;
    char* token_start = last_pos;

    while (*last_pos)
    {
        if (is_delimiter(*last_pos, delimiter))
        {
            *last_pos = '\0';
            last_pos++;
            return token_start;
        }
        last_pos++;
    }

    return token_start;
}


char* strrchr(const char* str, int c)
{
    char* last_occurrence = NULL;
    
    char target = (char)c; 
    
    do 
    {
        if (*str == target) last_occurrence = (char *)str;
    } while (*str++);
    
    return last_occurrence;
}

size_t strlen(const char* str)
{
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}