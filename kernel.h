#ifndef ALURA_KERNEL_H
#define ALURA_KERNEL_H

#include "gdt.h"
#include "terminal.h"
#include "vga.h"

typedef enum kernel_result_t {
    KERNEL_RESULT_OK
} kernel_result_t;

typedef struct kernel_t {
    terminal_t terminal;
    vga_t vga;
    gdt_t gdt;
} kernel_t;

static kernel_t kernel;

kernel_result_t kernel_initialize();

#endif // ALURA_KERNEL_H