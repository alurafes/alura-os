#include "memory_paging.h"

#include "vga.h"

void memory_paging_reset_entry(page_entry_t* entry)
{
    for (uint32_t i = 0; i < PAGE_SIZE / sizeof(page_entry_t); ++i)
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

static inline void memory_paging_enable()
{
    page_entry_t* page_directory_physical = (page_entry_t*)virtual_to_physical(page_directory);
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory_physical));
    uint32_t cr0 = 0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0)); 
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

void memory_paging_map_higher_half()
{
    for (uint32_t offset = 0; offset < KERNEL_MAPPINGS_END - KERNEL_MAPPINGS_START; offset += PAGE_SIZE)
    {
        memory_paging_map(offset, KERNEL_VIRTUAL_SPACE_START + offset, PAGE_READ_WRITE);
    }
}

page_entry_t* page_directory = NULL;
void memory_paging_module_init()
{
    page_directory = memory_bitmap_allocate();
    if (page_directory == NULL) return; // Panic or somethin
    page_directory = (page_entry_t*)physical_to_virtual(page_directory);
    memory_paging_reset_entry(page_directory);

    memory_paging_map_higher_half();
    
    memory_paging_enable();
    memory_paging_disable_pse();
}

void memory_paging_map(uintptr_t physical_address, uintptr_t virtual_address, uint32_t flags)
{
    size_t page_directory_index = (virtual_address >> 22) & 0x3FF;
    size_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    uint32_t page_table_present = page_directory[page_directory_index] & PAGE_PRESENT;
    if (!page_table_present)
    {
        page_entry_t* new_page_table_entry = (page_entry_t*)memory_bitmap_allocate();
        if (new_page_table_entry == NULL) return; // TODO: Panic

        page_entry_t* new_page_table_entry_virtual = (page_entry_t*)physical_to_virtual(new_page_table_entry);
        memory_paging_reset_entry(new_page_table_entry_virtual);

        page_directory[page_directory_index] = ((uint32_t)new_page_table_entry) | PAGE_PRESENT | PAGE_READ_WRITE;
    }
    page_entry_t* page_table = (page_entry_t*)(page_directory[page_directory_index] & 0xFFFFF000);
    page_entry_t* page_table_virtual = (page_entry_t*)physical_to_virtual(page_table);
    
    page_table_virtual[page_table_index] = (physical_address) | (flags & 0xFFF) | PAGE_PRESENT;
}