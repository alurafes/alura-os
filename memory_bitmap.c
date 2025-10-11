#include "memory_bitmap.h"

memory_bitmap_t memory_bitmap;

extern char _kernel_start;
extern char _kernel_end;

void memory_bitmap_module_init(multiboot_info_t* multiboot)
{
    size_t total_memory_bytes = 0;
    for (int i = 0; i < multiboot->mmap_length; i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* memory_map = (multiboot_memory_map_t*)(multiboot->mmap_addr + i); 
        size_t region_end = memory_map->len + memory_map->addr;
        if (region_end > total_memory_bytes) total_memory_bytes = region_end;   
    }

    memory_bitmap.pages = total_memory_bytes / PAGE_SIZE;
    memory_bitmap.entries = (uint32_t*)(ALIGN_TO_PAGE((uint32_t)&_kernel_end, PAGE_SIZE));

    for (int page_index = 0; page_index < memory_bitmap.pages; ++page_index)
    {
        BITMAP_SET(memory_bitmap.entries, page_index);
    }

    for (int i = 0; i < multiboot->mmap_length; i += sizeof(multiboot_memory_map_t)) 
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

    size_t page = 0;
    size_t last_page = 0;
    // lock first megabyte
    last_page = 0x100000 / PAGE_SIZE;
    for (size_t page = 0; page < last_page; ++page)
    {
        BITMAP_SET(memory_bitmap.entries, page);
    }

    // lock kernel
    page = (uint32_t)&_kernel_start / PAGE_SIZE;
    last_page = ((uint32_t)&_kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (; page < last_page; ++page)
    {
        BITMAP_SET(memory_bitmap.entries, page);
    }

    // lock memory bitmap
    size_t bitmap_start_page = (uint32_t)memory_bitmap.entries / PAGE_SIZE;
    for (page = 0; page < memory_bitmap.pages; ++page)
    {
        BITMAP_SET(memory_bitmap.entries, bitmap_start_page + page);
    }
}

void* memory_bitmap_allocate()
{
    int page_index = 0;
    for (; page_index < memory_bitmap.pages; ++page_index)
    {
        if (!BITMAP_TEST(memory_bitmap.entries, page_index)) break;
    }
    BITMAP_SET(memory_bitmap.entries, page_index);
    return (void*)(page_index * PAGE_SIZE);
}

void memory_bitmap_free(void* address)
{
    uint32_t page_index = (uint32_t)address / PAGE_SIZE;
    if (BITMAP_TEST(memory_bitmap.entries, page_index)) BITMAP_CLEAR(memory_bitmap.entries, page_index); 
}