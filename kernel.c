#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_POINTER ((uint16_t*)VGA_BUFFER)

#include <stdint.h>
#include <stddef.h>

#define VGA_COLOR_BLACK 0x00
#define VGA_COLOR_BLUE 0x01
#define VGA_COLOR_GREEN 0x02
#define VGA_COLOR_CYAN 0x03
#define VGA_COLOR_RED 0x04
#define VGA_COLOR_MAGENTA 0x05
#define VGA_COLOR_BROWN 0x06
#define VGA_COLOR_LIGHT_GRAY 0x07
#define VGA_COLOR_DARK_GRAY 0x08
#define VGA_COLOR_LIGHT_BLUE 0x09
#define VGA_COLOR_LIGHT_GREEN 0x0A
#define VGA_COLOR_LIGHT_CYAN 0x0B
#define VGA_COLOR_LIGHT_RED 0x0C
#define VGA_COLOR_PINK 0x0D
#define VGA_COLOR_YELLOW 0x0E
#define VGA_COLOR_WHITE 0x0F

typedef struct vga_color_t {
    uint8_t background : 4;
    uint8_t foreground : 4;
} vga_color_t;

#define VGA_COLOR(color) (color.background << 4 | color.foreground)

void vga_clear_color(vga_color_t color)
{
    for (short y = 0; y < VGA_HEIGHT; ++y)
    {
        for (short x = 0; x < VGA_WIDTH; ++x)
        {
            VGA_POINTER[VGA_WIDTH * y + x] = VGA_COLOR(color) << 8 | 0x0;
        }
    }
}

typedef enum vga_result_t {
    VGA_OK,
    VGA_OUT_OF_BOUNDS
} vga_result_t;

vga_result_t vga_put_char(int x, int y, char character, vga_color_t color)
{
    if (x < 0 || x >= VGA_WIDTH ||
        y < 0 || y >= VGA_HEIGHT) return VGA_OUT_OF_BOUNDS;
    VGA_POINTER[y * VGA_WIDTH + x] = VGA_COLOR(color) << 8 | character;
    return VGA_OK;
}

vga_result_t vga_put_string(int x, int y, const char* string, vga_color_t color)
{
    for (int i = 0; string[i] != '\0'; ++i)
    {
        vga_result_t result = vga_put_char(x + i, y, string[i], color);
        if (result != VGA_OK) return result;
    }
    return VGA_OK;
}

void kernel_main()
{
    const char* message = "BABABOOEY";

    vga_clear_color((vga_color_t){.background = VGA_COLOR_GREEN});

    for (int i = 0; i < 10; ++i)
    {
        vga_put_char(i, i, 0x41 + i, (vga_color_t){.background = VGA_COLOR_GREEN, .foreground = VGA_COLOR_BLACK});
    }

    vga_put_string(20, 5, message, (vga_color_t){.background = VGA_COLOR_GREEN, .foreground = VGA_COLOR_PINK});

    while (1) { __asm__ __volatile__("hlt"); }
}