#ifndef ALURA_MEMORY_PAGING_H
#define ALURA_MEMORY_PAGING_H

#include <stdint.h>
#include <stddef.h>

#include "memory_bitmap.h"

#define KERNEL_PDE_START (KERNEL_VIRTUAL_SPACE_START >> 22)
#define KERNEL_PDE_ENTRIES (PAGE_SIZE / sizeof(page_entry_t))

#define PAGE_PRESENT 0x1
#define PAGE_READ_WRITE 0x2
#define PAGE_USER 0x4
#define PAGE_4MB 0x80
#define PAGE_MASK 0xFFFFF000

typedef uint32_t page_entry_t;

typedef enum memory_paging_result_t {
    MEMORY_PAGING_RESULT_OK,
    MEMORY_PAGING_RESULT_ALLOCATION_ERROR,
    MEMORY_PAGING_RESULT_BAD_PARAMETER,
} memory_paging_result_t;

extern page_entry_t* kernel_page_directory;
void memory_paging_module_init();

void memory_paging_set(page_entry_t* page_directory);
memory_paging_result_t memory_paging_create_kernel_page_directory();
memory_paging_result_t memory_paging_create_page_directory(page_entry_t** result);
memory_paging_result_t memory_paging_copy_mapped_memory(page_entry_t* src, page_entry_t* dst);
memory_paging_result_t memory_paging_map(page_entry_t* page_directory, uint32_t physical_address, uint32_t virtual_address, uint32_t flags);
memory_paging_result_t memory_paging_unmap(page_entry_t* page_directory, uintptr_t virtual_address);
uintptr_t memory_paging_virtual_to_physical(page_entry_t *page_directory, uintptr_t virtual_address);

#endif // ALURA_MEMORY_PAGING_H