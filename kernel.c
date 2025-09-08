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

typedef struct vga_point_t {
    int x;
    int y;
} vga_point_t;

typedef struct vga_t {
    vga_point_t cursor;
    vga_color_t color;
} vga_t;

typedef enum vga_result_t {
    VGA_OK,
    VGA_OUT_OF_BOUNDS
} vga_result_t;

vga_result_t vga_set_cursor(vga_t* vga, vga_point_t point)
{
    if (point.x < 0 || point.x >= VGA_WIDTH ||
        point.y < 0 || point.y >= VGA_HEIGHT) return VGA_OUT_OF_BOUNDS;
    vga->cursor = point;
    return VGA_OK;
}

vga_result_t vga_set_color(vga_t* vga, vga_color_t color)
{
    vga->color = color;
    return VGA_OK;
}

void vga_clear_color(vga_t* vga)
{
    vga->cursor.x = 0;
    vga->cursor.y = 0;
    for (vga->cursor.y = 0; vga->cursor.y < VGA_HEIGHT; ++vga->cursor.y)
    {
        for (vga->cursor.x = 0; vga->cursor.x < VGA_WIDTH; ++vga->cursor.x)
        {
            VGA_POINTER[VGA_WIDTH * vga->cursor.y + vga->cursor.x] = VGA_COLOR(vga->color) << 8 | 0x0;
        }
    }
}

vga_result_t vga_put_char(vga_t* vga, char character)
{
    VGA_POINTER[VGA_WIDTH * vga->cursor.y + vga->cursor.x] = VGA_COLOR(vga->color) << 8 | character;
    return VGA_OK;
}

vga_result_t vga_put_string(vga_t* vga, const char* string)
{
    for (int i = 0; string[i] != '\0'; ++i)
    {
        vga_result_t result = vga_put_char(vga, string[i]);
        if (result != VGA_OK) return result;
        vga_point_t next_point = {
            .x = vga->cursor.x + 1,
            .y = vga->cursor.y
        };
        result = vga_set_cursor(vga, next_point);
        if (result != VGA_OK) return result;
    }
    return VGA_OK;
}

void kernel_main()
{
    vga_t vga;
    vga_set_color(&vga, (vga_color_t){.background = VGA_COLOR_GREEN});
    vga_clear_color(&vga);

    for (int i = 0; i < 10; ++i)
    {
        vga_set_color(&vga, (vga_color_t){.background = VGA_COLOR_GREEN, .foreground = VGA_COLOR_BLACK});
        vga_set_cursor(&vga, (vga_point_t){.x = i, .y = i});
        vga_put_char(&vga, 0x41 + i);
    }

    vga_set_color(&vga, (vga_color_t){.background = VGA_COLOR_BLACK, .foreground = VGA_COLOR_PINK});
    vga_set_cursor(&vga, (vga_point_t){.x = 75, .y = 5});
    vga_put_string(&vga, "BABABOOEY");

    while (1) { __asm__ __volatile__("hlt"); }
}