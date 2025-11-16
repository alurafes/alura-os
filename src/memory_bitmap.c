#include "memory_bitmap.h"

void memory_bitmap_lock_first_megabyte(memory_bitmap_t* bitmap)
{
    size_t last_page = 0x100000 / PAGE_SIZE;
    for (size_t page = 0; page < last_page; ++page)
    {
        BITMAP_SET(bitmap->entries, page);
    }
}

void memory_bitmap_lock_kernel(memory_bitmap_t* bitmap)
{
    size_t page = (uint32_t)&_kernel_physical_start / PAGE_SIZE;
    size_t last_page = ((uint32_t)&_kernel_physical_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (; page < last_page; ++page)
    {
        BITMAP_SET(bitmap->entries, page);
    }
}
void memory_bitmap_lock_bitmap(memory_bitmap_t* bitmap)
{
    uintptr_t bitmap_phys_start = ALIGN_UP((uintptr_t)&_kernel_physical_end);

    size_t bitmap_size_bytes = ALIGN_UP(bitmap->pages / 8);
    size_t bitmap_pages = bitmap_size_bytes / PAGE_SIZE;

    size_t start_page = bitmap_phys_start / PAGE_SIZE;
    for (size_t i = 0; i < bitmap_pages; ++i)
    {
        BITMAP_SET(bitmap->entries, start_page + i);
    }
}

memory_bitmap_t memory_bitmap;
void memory_bitmap_module_init(multiboot_info_t* multiboot)
{
    size_t total_memory_bytes = 0;
    for (uint32_t i = 0; i < multiboot->mmap_length; i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* memory_map = (multiboot_memory_map_t*)(multiboot->mmap_addr + i); 
        size_t region_end = memory_map->len + memory_map->addr;
        if (region_end > total_memory_bytes) total_memory_bytes = region_end;   
    }

    memory_bitmap.pages = total_memory_bytes / PAGE_SIZE;
    memory_bitmap.entries = (uint32_t*)(ALIGN_UP((uint32_t)&_kernel_physical_end + KERNEL_VIRTUAL_SPACE_START));

    for (uint32_t page_index = 0; page_index < memory_bitmap.pages; ++page_index)
    {
        BITMAP_SET(memory_bitmap.entries, page_index);
    }

    for (uint32_t i = 0; i < multiboot->mmap_length; i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* memory_map = (multiboot_memory_map_t*)(multiboot->mmap_addr + i); 
        if (memory_map->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            size_t page = memory_map->addr / PAGE_SIZE;
            size_t last_page = (memory_map->addr + memory_map->len + PAGE_SIZE - 1) / PAGE_SIZE;
            for (; page < last_page; ++page)
            {
                BITMAP_CLEAR(memory_bitmap.entries, page);
            }
        }
    }

    memory_bitmap_lock_first_megabyte(&memory_bitmap);
    memory_bitmap_lock_kernel(&memory_bitmap);
    memory_bitmap_lock_bitmap(&memory_bitmap);
    
    memory_bitmap.last_allocated_page_index = 0;
}

void* memory_bitmap_allocate()
{
    size_t page_index = memory_bitmap.last_allocated_page_index;
    uint32_t free_page_found = 0;
    for (; page_index < memory_bitmap.pages; ++page_index)
    {
        if (!BITMAP_TEST(memory_bitmap.entries, page_index)) {
            free_page_found = 1;
            break;
        }
    }
    if (!free_page_found)
    {
        for (page_index = 0; page_index < memory_bitmap.last_allocated_page_index; ++page_index)
        {
            if (!BITMAP_TEST(memory_bitmap.entries, page_index)) {
                free_page_found = 1;
                break;
            }
        }
    }
    if (!free_page_found) return NULL;

    BITMAP_SET(memory_bitmap.entries, page_index);
    memory_bitmap.last_allocated_page_index = page_index;
    return (void*)(page_index * PAGE_SIZE);
}

void memory_bitmap_free(void* address)
{
    uint32_t page_index = (uint32_t)address / PAGE_SIZE;
    if (BITMAP_TEST(memory_bitmap.entries, page_index)) BITMAP_CLEAR(memory_bitmap.entries, page_index); 
    if (page_index < memory_bitmap.last_allocated_page_index) memory_bitmap.last_allocated_page_index = page_index; // next allocation will be a bit faster :)
}