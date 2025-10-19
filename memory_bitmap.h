#ifndef ALURA_MEMORY_BITMAP_H
#define ALURA_MEMORY_BITMAP_H

#include <stdint.h>
#include <stddef.h>

#include "multiboot.h"

#include "memory.h"

#define BITMAP_ENTRY_SIZE_IN_BITS (sizeof(bitmap_entry_t) * 8)
#define BITMAP_SET(bitmap, page_index) (bitmap[page_index / BITMAP_ENTRY_SIZE_IN_BITS] |=  1 << (page_index % BITMAP_ENTRY_SIZE_IN_BITS))
#define BITMAP_CLEAR(bitmap, page_index) (bitmap[page_index / BITMAP_ENTRY_SIZE_IN_BITS] &= ~(1 << (page_index % BITMAP_ENTRY_SIZE_IN_BITS)))
#define BITMAP_TEST(bitmap, page_index) (bitmap[page_index / BITMAP_ENTRY_SIZE_IN_BITS] & 1 << (page_index % BITMAP_ENTRY_SIZE_IN_BITS))

typedef uint32_t bitmap_entry_t;

typedef struct memory_bitmap_t {
    bitmap_entry_t* entries;
    size_t pages;
    size_t last_allocated_page_index;
} memory_bitmap_t;

extern memory_bitmap_t memory_bitmap;
void memory_bitmap_module_init(multiboot_info_t* multiboot);

void* memory_bitmap_allocate();
void memory_bitmap_free(void* address);

#endif // ALURA_MEMORY_BITMAP_H