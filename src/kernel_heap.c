#include "kernel_heap.h"

static kernel_heap_t kernel_heap;

void* kernel_heap_allocate_physical_memory(size_t bytes)
{
    size_t bytes_to_allocate = ALIGN_UP(bytes);
    size_t pages_to_allocate = bytes_to_allocate / PAGE_SIZE;
    if (kernel_heap.heap_break + bytes_to_allocate > kernel_heap.heap_end) return NULL;

    uintptr_t address = kernel_heap.heap_break;
    for (size_t i = 0; i < pages_to_allocate; ++i)
    {
        void* physical_allocated = memory_bitmap_allocate();
        if (!physical_allocated) return NULL;
        memory_paging_map((uint32_t)physical_allocated, (uint32_t)address, PAGE_READ_WRITE);
        address += PAGE_SIZE;
    }
    void* allocated_address = (void*)kernel_heap.heap_break;
    kernel_heap.heap_break += bytes_to_allocate;
    return allocated_address;
}

kernel_heap_header_t* kernel_heap_allocate_more_headers(size_t headers_to_allocate)
{
    size_t bytes_to_allocate = headers_to_allocate * sizeof(kernel_heap_header_t);
    if (bytes_to_allocate < KERNEL_HEAP_MINIMUM_HEADERS_TO_ALLOCATE * sizeof(kernel_heap_header_t)) 
        bytes_to_allocate = KERNEL_HEAP_MINIMUM_HEADERS_TO_ALLOCATE * sizeof(kernel_heap_header_t);
    void* allocated_memory = (kernel_heap_header_t*)kernel_heap_allocate_physical_memory(bytes_to_allocate);
    if (allocated_memory == NULL) return NULL;
    kernel_heap_header_t* header = (kernel_heap_header_t*)allocated_memory;
    header->size = bytes_to_allocate / sizeof(kernel_heap_header_t);
    header->next = header;
    kernel_heap_free((void*)(header + 1));
    return kernel_heap.free_headers_head;
}

void* kernel_heap_malloc(size_t size)
{
    if (size == 0) return NULL;
    size_t headers_to_allocate = (size + sizeof(kernel_heap_header_t) - 1) / sizeof(kernel_heap_header_t) + 1;
    
    kernel_heap_header_t* previous_header = kernel_heap.free_headers_head;
    kernel_heap_header_t* current_header = previous_header->next;
    while (1)
    {
        if (current_header->size >= headers_to_allocate)
        {
            if (current_header->size == headers_to_allocate) previous_header->next = current_header->next;
            else 
            {
                current_header->size -= headers_to_allocate;
                current_header += current_header->size;
                current_header->size = headers_to_allocate;
            }
            kernel_heap.free_headers_head = previous_header;
            void* allocated_memory = (void*)(current_header + 1);
            return allocated_memory;
        }
        if (current_header == kernel_heap.free_headers_head)
        {
            if (kernel_heap_allocate_more_headers(headers_to_allocate) == NULL) return NULL;
        }
        previous_header = current_header;
        current_header = current_header->next;
    }
    return NULL;
}

void* kernel_heap_calloc(size_t size)
{
    void* memory = kernel_heap_malloc(size);
    if (memory == NULL) return NULL;
    // todo: move this to memset or something
    char* ptr = (char*)memory;
    while (size--) {
        *ptr++ = 0;
    }
    return memory;
}

void kernel_heap_free(void* address)
{
    if (address == NULL) return;

    kernel_heap_header_t* freed_block_header = (kernel_heap_header_t*)address - 1;
    kernel_heap_header_t* freed_block_new_location = kernel_heap.free_headers_head;
    while (1)
    {
        // we must put the freed block in a proper order (address based order)
        uint32_t freed_block_after_current = freed_block_header > freed_block_new_location;
        uint32_t freed_block_before_next = freed_block_header < freed_block_new_location->next;
        if (freed_block_after_current && freed_block_before_next) break;
        if (freed_block_new_location >= freed_block_new_location->next && (freed_block_after_current || freed_block_before_next)) break;
        freed_block_new_location = freed_block_new_location->next;
    }

    // merge with next if adjacent
    if (freed_block_header + freed_block_header->size == freed_block_new_location->next)
    {
        freed_block_header->size += freed_block_new_location->next->size;
        freed_block_header->next = freed_block_new_location->next->next;
    }
    else freed_block_header->next = freed_block_new_location->next;

    // merge with previous if adjacent
    if (freed_block_new_location + freed_block_new_location->size == freed_block_header)
    {
        freed_block_new_location->size += freed_block_header->size;
        freed_block_new_location->next = freed_block_header->next;
    } else freed_block_new_location->next = freed_block_header;

    kernel_heap.free_headers_head = freed_block_new_location;
}

void kernel_heap_module_init()
{
    kernel_heap.base.next = &kernel_heap.base;
    kernel_heap.base.size = 0;
    kernel_heap.free_headers_head = &kernel_heap.base;
    kernel_heap.heap_start = ALIGN_UP(KERNEL_HEAP_VIRTUAL_START);
    kernel_heap.heap_end = ALIGN_DOWN(KERNEL_HEAP_VIRTUAL_END);
    kernel_heap.heap_break = kernel_heap.heap_start;
}