#include "memory_paging.h"
#include "memory.h"

#include "libc/string.h"
#include "kernel_heap.h"
#include "print.h"

// bounce pages
static uint8_t bounce_pages_used[KERNEL_BOUNCE_PAGE_SIZE];
void bounce_init(void)
{
    for (uint32_t i = 0; i < KERNEL_BOUNCE_PAGE_SIZE; i++)
    {
        bounce_pages_used[i] = 0;
    }
}

void* bounce_alloc(uintptr_t physical_address)
{
    for (uint32_t i = 0; i < KERNEL_BOUNCE_PAGE_SIZE; i++)
    {
        if (!bounce_pages_used[i])
        {
            uintptr_t virtual_address = KERNEL_BOUNCE_PAGE_START + (i * PAGE_SIZE);

            if (memory_paging_map_current(physical_address, virtual_address, PAGE_READ_WRITE) != MEMORY_PAGING_RESULT_OK)
            {
                return NULL;
            }
            bounce_pages_used[i] = 1;
            return (void*)virtual_address;
        }
    }

    return NULL;
}

void bounce_free(uintptr_t virtual_address)
{
    if (virtual_address < KERNEL_BOUNCE_PAGE_START) return;

    uint32_t bounce_index = (virtual_address - KERNEL_BOUNCE_PAGE_START) / PAGE_SIZE;
    if (bounce_index >= KERNEL_BOUNCE_PAGE_SIZE) return;

    memory_paging_unmap_current(virtual_address);
    bounce_pages_used[bounce_index] = 0;
}

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

void memory_paging_set(page_entry_t* page_directory_physical)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory_physical));
    uint32_t cr0 = 0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0)); 
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

memory_paging_result_t memory_paging_map_kernel(uint32_t physical_address, uint32_t virtual_address, uint32_t flags)
{
    size_t page_directory_index = (virtual_address >> 22) & 0x3FF;
    size_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    uint32_t page_table_present = kernel_page_directory[page_directory_index] & PAGE_PRESENT;
    if (!page_table_present)
    {
        page_entry_t* new_page_table_entry = (page_entry_t*)memory_bitmap_allocate();
        if (new_page_table_entry == NULL) return MEMORY_PAGING_RESULT_ALLOCATION_ERROR;

        page_entry_t* new_page_table_entry_virtual = (page_entry_t*)physical_to_virtual(new_page_table_entry);
        memory_paging_reset_entry(new_page_table_entry_virtual);
        kernel_page_directory[page_directory_index] = ((uint32_t)new_page_table_entry) | flags | PAGE_PRESENT;
    }
    if (flags & PAGE_USER)
    {
        kernel_page_directory[page_directory_index] |= PAGE_USER;
    }
    page_entry_t* page_table = (page_entry_t*)(kernel_page_directory[page_directory_index] & PAGE_MASK);
    page_entry_t* page_table_virtual = (page_entry_t*)physical_to_virtual(page_table);
    
    page_table_virtual[page_table_index] = (physical_address) | flags | PAGE_PRESENT;

    asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");

    return MEMORY_PAGING_RESULT_OK;
}

void memory_paging_map_higher_half()
{
    for (uint32_t offset = 0; offset < KERNEL_MAPPINGS_END - KERNEL_MAPPINGS_START; offset += PAGE_SIZE)
    {
        memory_paging_map_kernel(offset, KERNEL_VIRTUAL_SPACE_START + offset, PAGE_READ_WRITE);
    }
}

page_entry_t* kernel_page_directory = NULL;
page_entry_t* kernel_page_directory_phys = NULL;

void memory_paging_module_init()
{
    bounce_init();
    memory_paging_create_kernel_page_directory();
    memory_paging_set(kernel_page_directory_phys);
    memory_paging_disable_pse();
}

memory_paging_result_t memory_paging_copy_mapped_memory(page_entry_t* dst)
{
    page_entry_t* src = (page_entry_t*)PAGE_DIRECTORY_VADDR;

    if (src == dst) return MEMORY_PAGING_RESULT_OK;
    if (dst == NULL) return MEMORY_PAGING_RESULT_BAD_PARAMETER;

    for (uint32_t page_directory_index = 0; page_directory_index < KERNEL_PDE_ENTRIES; ++page_directory_index)
    {
        uintptr_t virtual_address = page_directory_index << 22;
        if (virtual_address >= KERNEL_VIRTUAL_SPACE_START) break;

        if (!(src[page_directory_index] & PAGE_PRESENT)) continue;

        page_entry_t* page_table = PAGE_TABLE_VADDR(page_directory_index);

        for (uint32_t page_table_index = 0; page_table_index < KERNEL_PDE_ENTRIES; ++page_table_index)
        {
            if (!(page_table[page_table_index] & PAGE_PRESENT)) continue;

            uintptr_t virtual_addr = (page_directory_index << 22) | (page_table_index << 12);

            void* phys_dst = memory_bitmap_allocate();
            void* bitmap_dst = bounce_alloc((uintptr_t)phys_dst);

            memcpy(bitmap_dst, (void*)virtual_addr, PAGE_SIZE);

            uint32_t flags = page_table[page_table_index] & 0xFFF;
            memory_paging_map(dst, (uintptr_t)phys_dst, virtual_addr, flags);
            
            bounce_free((uintptr_t)bitmap_dst);
        }

    }

    return MEMORY_PAGING_RESULT_OK;
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

        page_entry_t* new_page_table_entry_virtual = (page_entry_t*)bounce_alloc((uintptr_t)new_page_table_entry);
        memory_paging_reset_entry(new_page_table_entry_virtual);
        bounce_free((uintptr_t)new_page_table_entry_virtual);
        page_directory[page_directory_index] = ((uint32_t)new_page_table_entry) | PAGE_READ_WRITE | PAGE_PRESENT;
    }
    if (flags & PAGE_USER)
    {
        page_directory[page_directory_index] |= PAGE_USER;
    }
    page_entry_t* page_table = (page_entry_t*)(page_directory[page_directory_index] & PAGE_MASK);
    page_entry_t* page_table_virtual = (page_entry_t*)bounce_alloc((uintptr_t)page_table);
    
    page_table_virtual[page_table_index] = (physical_address) | flags | PAGE_PRESENT;

    bounce_free((uintptr_t)page_table_virtual);

    asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");

    return MEMORY_PAGING_RESULT_OK;
}

memory_paging_result_t memory_paging_unmap(page_entry_t* page_directory, uintptr_t virtual_address)
{
    size_t page_directory_index = (virtual_address >> 22) & 0x3FF;
    size_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    uint32_t page_table_present = page_directory[page_directory_index] & PAGE_PRESENT;
    if (page_table_present)
    {
        page_entry_t* page_table = (page_entry_t*)(page_directory[page_directory_index] & PAGE_MASK);
        page_entry_t* page_table_virtual = (page_entry_t*)bounce_alloc((uintptr_t)page_table);
        page_table_virtual[page_table_index] = 0;
        bounce_free((uintptr_t)page_table_virtual);
    }

    asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");

    return MEMORY_PAGING_RESULT_OK;
}

memory_paging_result_t memory_paging_map_current(uint32_t physical_address, uint32_t virtual_address, uint32_t flags)
{
    page_entry_t* page_directory = (page_entry_t*)PAGE_DIRECTORY_VADDR;

    size_t page_directory_index = (virtual_address >> 22) & 0x3FF;
    size_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    uint32_t page_table_present = page_directory[page_directory_index] & PAGE_PRESENT;
    if (!page_table_present)
    {
        page_entry_t* new_page_table_entry = (page_entry_t*)memory_bitmap_allocate();
        if (new_page_table_entry == NULL) return MEMORY_PAGING_RESULT_ALLOCATION_ERROR;

        page_directory[page_directory_index] = ((uint32_t)new_page_table_entry) | PAGE_READ_WRITE | PAGE_PRESENT;

        asm volatile("mov %%cr3, %%eax\n\t"
                     "mov %%eax, %%cr3"
                     :
                     :
                     : "eax", "memory");

        memory_paging_reset_entry(PAGE_TABLE_VADDR(page_directory_index));
    }
    if (flags & PAGE_USER)
    {
        page_directory[page_directory_index] |= PAGE_USER;
    }
    page_entry_t* page_table = PAGE_TABLE_VADDR(page_directory_index);
    page_table[page_table_index] = (physical_address & PAGE_MASK) | flags | PAGE_PRESENT;

    asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");

    return MEMORY_PAGING_RESULT_OK;
}

memory_paging_result_t memory_paging_unmap_current(uintptr_t virtual_address)
{
    page_entry_t* page_directory = (page_entry_t*)PAGE_DIRECTORY_VADDR;
    size_t page_directory_index = (virtual_address >> 22) & 0x3FF;
    size_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    uint32_t page_table_present = page_directory[page_directory_index] & PAGE_PRESENT;
    if (page_table_present)
    {
        page_entry_t* page_table = PAGE_TABLE_VADDR(page_directory_index);
        page_table[page_table_index] = 0;
    }

    asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");

    return MEMORY_PAGING_RESULT_OK;
}

memory_paging_result_t memory_paging_create_kernel_page_directory()
{
    kernel_page_directory_phys = memory_bitmap_allocate();
    if (!kernel_page_directory_phys) return MEMORY_PAGING_RESULT_ALLOCATION_ERROR;

    kernel_page_directory = (page_entry_t*)((uintptr_t)kernel_page_directory_phys + KERNEL_VIRTUAL_SPACE_START);
    memory_paging_reset_entry(kernel_page_directory);
    memory_paging_map_higher_half();

    kernel_page_directory[1023] = (uintptr_t)kernel_page_directory_phys | PAGE_PRESENT | PAGE_READ_WRITE;

    return MEMORY_PAGING_RESULT_OK;
}

memory_paging_result_t memory_paging_create_page_directory(uint32_t* result)
{
    void* new_page_directory_phys = memory_bitmap_allocate();
    if (!new_page_directory_phys) return MEMORY_PAGING_RESULT_ALLOCATION_ERROR;

    page_entry_t* new_page_directory = (page_entry_t*)bounce_alloc((uint32_t)new_page_directory_phys);
    memory_paging_reset_entry(new_page_directory);

    // copy kernel PD mappings so i don't have to initialize them
    page_entry_t* current = (page_entry_t*)PAGE_DIRECTORY_VADDR;
    for (uint32_t i = KERNEL_PDE_START; i < KERNEL_PDE_ENTRIES; i++)
    {
        new_page_directory[i] = kernel_page_directory[i];
    }

    new_page_directory[1023] = (uintptr_t)new_page_directory_phys | PAGE_PRESENT | PAGE_READ_WRITE;

    bounce_free((uintptr_t)new_page_directory);

    *result = (uint32_t)new_page_directory_phys;
    
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
        page_entry_t* page_table = (page_entry_t*)bounce_alloc((uintptr_t)page_table_physical);
        memory_paging_free_page_table(page_table);
        memory_bitmap_free(page_table_physical);
        bounce_free((uintptr_t)page_table);
    }
}

uintptr_t memory_paging_virtual_to_physical(page_entry_t* page_directory_phys, uintptr_t virtual_address)
{
    uint32_t pde_index = (virtual_address >> 22) & 0x3FF;
    uint32_t pte_index = (virtual_address >> 12) & 0x3FF;

    page_entry_t* page_directory = (page_entry_t*)bounce_alloc((uintptr_t)page_directory_phys);

    uint32_t pde = page_directory[pde_index];

    if (!(pde & PAGE_PRESENT)) return 0;

    uintptr_t pt_phys = pde & PAGE_MASK;

    bounce_free((uintptr_t)page_directory);

    page_entry_t* page_table = (page_entry_t *)bounce_alloc((uintptr_t)pt_phys);
    uint32_t pte = page_table[pte_index];

    if (!(pte & PAGE_PRESENT)) return 0;

    uintptr_t page_physical = pte & PAGE_MASK;
    uintptr_t offset = virtual_address & 0xFFF;

    bounce_free((uintptr_t)page_table);

    return page_physical + offset;
}

page_entry_t* memory_paging_get_current_page_directory_physical()
{
    uint32_t cr3 = 0;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return (page_entry_t*)cr3; 
}

typedef struct page_directory_node_t {
    page_entry_t* page_directory;
    struct page_directory_node_t* next;
} page_directory_node_t;

page_directory_node_t* page_directories_to_destroy = NULL;
page_directory_node_t* page_directories_to_destroy_tail = NULL;

memory_paging_result_t memory_paging_queue_to_destroy(page_entry_t* page_directory)
{
    if (page_directory == kernel_page_directory_phys) return MEMORY_PAGING_RESULT_OK;
    page_directory_node_t* node = (page_directory_node_t*)kernel_heap_malloc(sizeof(page_directory_node_t));
    node->next = NULL;
    node->page_directory = page_directory;

    if (page_directories_to_destroy == NULL) 
    {
        page_directories_to_destroy = node;
        page_directories_to_destroy_tail = node;
        return MEMORY_PAGING_RESULT_OK;
    }
    page_directories_to_destroy_tail->next = node;
    page_directories_to_destroy_tail = node;
    return MEMORY_PAGING_RESULT_OK;
}

void memory_paging_destroy_queued()
{
    page_directory_node_t* head = page_directories_to_destroy;
    while (head != NULL)
    {
        page_entry_t* page_directory = (page_entry_t*)bounce_alloc((uintptr_t)head->page_directory);
        memory_paging_free_page_directory(page_directory);
        bounce_free((uintptr_t)page_directory);
        head = head->next;
        kernel_heap_free(head);
    }
}