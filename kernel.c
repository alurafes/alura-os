#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#include <stdint.h>
#include <stddef.h>

void vga_clear_color(uint8_t color)
{
    for (short y = 0; y < VGA_HEIGHT; ++y)
    {
        for (short x = 0; x < VGA_WIDTH; ++x)
        {
            ((uint16_t*)VGA_BUFFER)[VGA_WIDTH * y + x] = color << 8 | 0x0;
        }
    }
}

void kernel_main()
{
    const char* message = "BABABOOEY";

    vga_clear_color(0x20);

    for (int i = 0; message[i] != '\0'; ++i)
    {
        *((char*)VGA_BUFFER + (i * 2)) = message[i];
        *((char*)VGA_BUFFER + (i * 2 + 1)) = 0x17;
    }

    while (1) { __asm__ __volatile__("hlt"); }
}