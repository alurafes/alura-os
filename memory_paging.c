#include "memory_paging.h"

#include "print.h"

void memory_paging_reset_entry(page_entry_t* entry)
{
    for (int i = 0; i < PAGE_SIZE / sizeof(page_entry_t); ++i)
    {
        entry[i] = 0;
    }
}
 
void memory_paging_enable()
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0)); 
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

void memory_paging_map_first_megabyte()
{
    for (uintptr_t address = 0; address < 0x100000; address += PAGE_SIZE)
    {
        memory_paging_map(address, address, PAGE_READ_WRITE);
    }
}

void memory_paging_map_kernel()
{
    uintptr_t kernel_start = (uintptr_t)&_kernel_start;
    uintptr_t kernel_end = (uintptr_t)&_kernel_end;
    uintptr_t address = ALIGN_DOWN(kernel_start);
    for (; address < kernel_end; address += PAGE_SIZE)
    {
        memory_paging_map(address, address, PAGE_READ_WRITE);
    }
}

void memory_paging_map_memory_bitmap(memory_bitmap_t* bitmap)
{
    uintptr_t bitmap_start = (uintptr_t)bitmap->entries;
    size_t bitmap_size = (bitmap->pages + 7) / 8;
    uintptr_t bitmap_end = ALIGN_UP(bitmap_start + bitmap_size);

    uintptr_t address = ALIGN_DOWN(bitmap_start);
    for (; address < bitmap_end; address += PAGE_SIZE)
    {
        memory_paging_map(address, address, PAGE_READ_WRITE);
    }
}

page_entry_t* page_directory = NULL;
void memory_paging_module_init(memory_bitmap_t* bitmap)
{
    page_directory = memory_bitmap_allocate();
    if (page_directory == NULL) return; // Panic or somethin
    memory_paging_reset_entry(page_directory);

    memory_paging_map_first_megabyte();
    memory_paging_map_kernel();
    memory_paging_map_memory_bitmap(bitmap);
    
    memory_paging_enable();
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
        memory_paging_reset_entry(new_page_table_entry);
        page_directory[page_directory_index] = ((uint32_t)new_page_table_entry) | PAGE_PRESENT | PAGE_READ_WRITE;
    }
    page_entry_t* page_table = (page_entry_t*)(page_directory[page_directory_index] & 0xFFFFF000);
    page_table[page_table_index] = (physical_address) | (flags & 0xFFF) | PAGE_PRESENT;
}