#include "drivers/framebuffer.h"

#include "memory.h"
#include "memory_paging.h"

framebuffer_t framebuffer;
void framebuffer_driver_init(multiboot_info_t* multiboot)
{
    uint32_t physical_address = (uint32_t)multiboot->framebuffer_addr;
    uint32_t pitch = multiboot->framebuffer_pitch;
    uint32_t height = multiboot->framebuffer_height;
    uint32_t size = pitch * height;

    framebuffer.address = memory_paging_map_physical_range(physical_address, size, KERNEL_FRAMEBUFFER_VIRTUAL_START, PAGE_READ_WRITE);
    framebuffer.pitch = pitch;
    framebuffer.width = multiboot->framebuffer_width;
    framebuffer.height = height;
    framebuffer.bpp = multiboot->framebuffer_bpp;
    framebuffer.size = size;
}