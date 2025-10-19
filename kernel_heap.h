#ifndef ALURA_KERNEL_HEAP_H
#define ALURA_KERNEL_HEAP_H

#include "memory.h"
#include "memory_bitmap.h"
#include "memory_paging.h"

#define KERNEL_HEAP_MINIMUM_HEADERS_TO_ALLOCATE 1024
#define KERNEL_HEAP_VIRTUAL_START 0x05000000
#define KERNEL_HEAP_VIRTUAL_END 0x07000000

void* kernel_heap_malloc(size_t size);
void kernel_heap_free(void* address);

void kernel_heap_module_init(uintptr_t heap_start, uintptr_t heap_end);

typedef struct kernel_heap_header_t {
    struct kernel_heap_header_t* next;
    size_t size;
} kernel_heap_header_t;

typedef struct kernel_heap_t {
    kernel_heap_header_t base;
    kernel_heap_header_t* free_headers_head;
    uintptr_t heap_start;
    uintptr_t heap_end;
    uintptr_t heap_break;
} kernel_heap_t;

#endif // ALURA_KERNEL_HEAP_H