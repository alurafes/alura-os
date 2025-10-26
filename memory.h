#ifndef ALURA_MEMORY_H
#define ALURA_MEMORY_H

#include <stdint.h>

extern char _kernel_physical_start;
extern char _kernel_physical_end;

#define KERNEL_VIRTUAL_SPACE_START 0xC0000000
#define KERNEL_VIRTUAL_START KERNEL_VIRTUAL_SPACE_START
#define KERNEL_PHYSICAL_START ((uintptr_t)(&_kernel_physical_start))

#define ALIGN_UP_TO_SPECIFIC_PAGE(value, page_size) (value + page_size - 1) & ~(page_size - 1)
#define ALIGN_DOWN_TO_SPECIFIC_PAGE(value, page_size) ((value) & ~(page_size - 1))
#define PAGE_SIZE 4096
#define ALIGN_UP(value) (ALIGN_UP_TO_SPECIFIC_PAGE(value, PAGE_SIZE))
#define ALIGN_DOWN(value) (ALIGN_DOWN_TO_SPECIFIC_PAGE(value, PAGE_SIZE))

static inline uintptr_t physical_to_virtual(void* physical_address) {
    return (uintptr_t)physical_address + KERNEL_VIRTUAL_SPACE_START;
}

static inline uintptr_t virtual_to_physical(void* virtual_address) {
    return (uintptr_t)virtual_address - KERNEL_VIRTUAL_SPACE_START;
}

#endif // ALURA_MEMORY_H