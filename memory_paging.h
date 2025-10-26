#ifndef ALURA_MEMORY_PAGING_H
#define ALURA_MEMORY_PAGING_H

#include <stdint.h>
#include <stddef.h>

#include "memory.h"
#include "memory_bitmap.h"

#define PAGE_PRESENT 0x1
#define PAGE_READ_WRITE 0x2
#define PAGE_USER 0x4
#define PAGE_4MB 0x80

typedef uint32_t page_entry_t;

extern page_entry_t* page_directory;
void memory_paging_module_init();
void memory_paging_map(uint32_t physical_address, uint32_t virtual_address, uint32_t flags);

#endif // ALURA_MEMORY_PAGING_H