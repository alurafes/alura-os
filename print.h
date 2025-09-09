#ifndef ALURA_PRINT_H
#define ALURA_PRINT_H

#include "terminal.h"

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#define FORMATTED_PRINT_NUMBER_BUFFER_LENGTH 32
#define FORMATTED_PRINT_BUFFER 256

int vsnprintf(char* buffer, size_t buffer_len, const char* fmt, va_list args);
int printf(const char* fmt, ...);

#endif // ALURA_PRINT_H