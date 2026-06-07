#include "memory_paging.h"
#include "memory.h"

void memory_paging_reset_entry(page_entry_t* entry)
{
    for (uint32_t i = 0; i < KERNEL_PDE_ENTRIES; ++i)
    {
        entry[i] = 0;
    }
}
 
static inline void memory_paging_disable_pse() {
    uint32_t cr4 = 0;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 &= ~0x00000010;
    __asm__ volatile("mov %0, %%cr4" :: "r"(cr4));
}

void memory_paging_set(page_entry_t* page_directory)
{
    page_entry_t* page_directory_physical = (page_entry_t*)virtual_to_physical(page_directory);
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory_physical));
    uint32_t cr0 = 0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0)); 
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

void memory_paging_map_higher_half(page_entry_t* page_directory)
{
    for (uint32_t offset = 0; offset < KERNEL_MAPPINGS_END - KERNEL_MAPPINGS_START; offset += PAGE_SIZE)
    {
        memory_paging_map(page_directory, offset, KERNEL_VIRTUAL_SPACE_START + offset, PAGE_READ_WRITE);
    }
}

page_entry_t* kernel_page_directory = NULL;
void memory_paging_module_init()
{
    memory_paging_create_kernel_page_directory();
    memory_paging_set(kernel_page_directory);
    memory_paging_disable_pse();
}

memory_paging_result_t memory_paging_map(page_entry_t* page_directory, uintptr_t physical_address, uintptr_t virtual_address, uint32_t flags)
{
    size_t page_directory_index = (virtual_address >> 22) & 0x3FF;
    size_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    uint32_t page_table_present = page_directory[page_directory_index] & PAGE_PRESENT;
    if (!page_table_present)
    {
        page_entry_t* new_page_table_entry = (page_entry_t*)memory_bitmap_allocate();
        if (new_page_table_entry == NULL) return MEMORY_PAGING_RESULT_ALLOCATION_ERROR;

        page_entry_t* new_page_table_entry_virtual = (page_entry_t*)physical_to_virtual(new_page_table_entry);
        memory_paging_reset_entry(new_page_table_entry_virtual);
        page_directory[page_directory_index] = ((uint32_t)new_page_table_entry) | flags | PAGE_PRESENT;
    }
    if (flags & PAGE_USER)
    {
        page_directory[page_directory_index] |= PAGE_USER;
    }
    page_entry_t* page_table = (page_entry_t*)(page_directory[page_directory_index] & PAGE_MASK);
    page_entry_t* page_table_virtual = (page_entry_t*)physical_to_virtual(page_table);
    
    page_table_virtual[page_table_index] = (physical_address) | flags | PAGE_PRESENT;

    return MEMORY_PAGING_RESULT_OK;
}

memory_paging_result_t memory_paging_create_kernel_page_directory()
{
    kernel_page_directory = memory_bitmap_allocate();
    if (!kernel_page_directory) return MEMORY_PAGING_RESULT_ALLOCATION_ERROR;

    kernel_page_directory = (page_entry_t*)physical_to_virtual(kernel_page_directory);
    memory_paging_reset_entry(kernel_page_directory);

    memory_paging_map_higher_half(kernel_page_directory);
    return MEMORY_PAGING_RESULT_OK;
}

memory_paging_result_t memory_paging_create_page_directory(page_entry_t** result)
{
    page_entry_t* page_directory = memory_bitmap_allocate();
    if (!page_directory) return MEMORY_PAGING_RESULT_ALLOCATION_ERROR;

    page_directory = (page_entry_t*)physical_to_virtual(page_directory);
    memory_paging_reset_entry(page_directory);

    // copy kernel PD mappings so i don't have to initialize them
    for (uint32_t i = KERNEL_PDE_START; i < KERNEL_PDE_ENTRIES; i++)
    {
        page_directory[i] = kernel_page_directory[i];
    }

    *result = page_directory;

    return MEMORY_PAGING_RESULT_OK;
}

void memory_paging_free_page_table(page_entry_t* page_table)
{
    for (size_t page_table_entry = 0; page_table_entry < KERNEL_PDE_ENTRIES; ++page_table_entry)
    {
        if (!(page_table[page_table_entry] & PAGE_PRESENT)) continue;
        void* page_table_entry_physical = (void*)(page_table[page_table_entry] & PAGE_MASK);
        memory_bitmap_free(page_table_entry_physical);
    }
}

void memory_paging_free_page_directory(page_entry_t* page_directory)
{
    if (!page_directory) return;

    // Kernel PDEs are shared
    for (uint32_t page_directory_entry = 0; page_directory_entry < KERNEL_PDE_START; ++page_directory_entry)
    {
        if (!(page_directory[page_directory_entry] & PAGE_PRESENT)) continue;
        void* page_table_physical = (void*)(page_directory[page_directory_entry]);
        page_entry_t* page_table = (page_entry_t*)physical_to_virtual(page_table_physical);

        memory_paging_free_page_table(page_table);
        memory_bitmap_free(page_table_physical);
    }
}