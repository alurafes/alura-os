#ifndef ALURA_LIBC_STRING_H
#define ALURA_LIBC_STRING_H

#include <stddef.h>

int strcmp(const char* s1, const char* s2);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);
void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* dst, char ch, size_t size);
char* strtok(char* str, const char* delimiter);
char* strrchr(const char* str, int c);
size_t strlen(const char* str);

#endif // ALURA_LIBC_STRING_H