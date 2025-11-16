#ifndef ALURA_KERNEL_H
#define ALURA_KERNEL_H

#include "multiboot.h"

#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "terminal.h"
#include "vga.h"
#include "print.h"
#include "pic.h"
#include "memory_bitmap.h"
#include "memory_paging.h"
#include "kernel_heap.h"
#include "task_manager.h"
#include "timer.h"

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum kernel_result_t {
    KERNEL_RESULT_OK
} kernel_result_t;

kernel_result_t kernel_initialize(multiboot_info_t* multiboot);
void kernel_main(multiboot_info_t* multiboot);

#endif // ALURA_KERNEL_H