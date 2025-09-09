#ifndef ALURA_KERNEL_H
#define ALURA_KERNEL_H

#include "gdt.h"
#include "terminal.h"
#include "vga.h"
#include "print.h"

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum kernel_result_t {
    KERNEL_RESULT_OK
} kernel_result_t;

kernel_result_t kernel_initialize();
void kernel_main();

#endif // ALURA_KERNEL_H