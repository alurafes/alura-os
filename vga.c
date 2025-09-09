#include "vga.h"

#include "stddef.h"

vga_result_t vga_create(vga_t* vga)
{
    if (vga == NULL) return VGA_RESULT_NULL;
    vga->color = (vga_color_t){.background = VGA_COLOR_BLACK, .foreground = VGA_COLOR_WHITE};
    vga->cursor = (vga_point_t){.x = 0, .y = 0};
    vga->overflow = VGA_OVERFLOW_NONE;
    vga->scroll = VGA_SCROLL_NONE;
    return VGA_RESULT_OK;
}

vga_result_t vga_set_cursor(vga_t* vga, vga_point_t point)
{
    if (point.x < 0 || point.x >= VGA_WIDTH ||
        point.y < 0 || point.y >= VGA_HEIGHT) 
        {
            switch (vga->overflow)
            {
            case VGA_OVERFLOW_NONE:
                return VGA_RESULT_OUT_OF_BOUNDS;
            case VGA_OVERFLOW_WRAP:
                point.x = 0;
                break;
            case VGA_OVERFLOW_NEW_LINE:
                point.x = 0;
                point.y = point.y + 1 >= VGA_HEIGHT ? 0 : point.y + 1;
                break;
            }
        }
    vga->cursor = point;
    return VGA_RESULT_OK;
}

vga_result_t vga_set_color(vga_t* vga, vga_color_t color)
{
    vga->color = color;
    return VGA_RESULT_OK;
}

vga_result_t vga_clear_color(vga_t* vga)
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
    return VGA_RESULT_OK;
}

vga_result_t vga_put_char(vga_t* vga, char character)
{
    VGA_POINTER[VGA_WIDTH * vga->cursor.y + vga->cursor.x] = VGA_COLOR(vga->color) << 8 | character;
    return VGA_RESULT_OK;
}

vga_result_t vga_put_string(vga_t* vga, const char* string)
{
    for (int i = 0; string[i] != '\0'; ++i)
    {
        vga_result_t result = vga_put_char(vga, string[i]);
        if (result != VGA_RESULT_OK) return result;
        vga_point_t next_point = {
            .x = vga->cursor.x + 1,
            .y = vga->cursor.y
        };
        result = vga_set_cursor(vga, next_point);
        if (result != VGA_RESULT_OK) return result;
    }
    return VGA_RESULT_OK;
}

vga_result_t vga_set_overflow(vga_t* vga, vga_overflow_t overflow)
{
    vga->overflow = overflow;
    return VGA_RESULT_OK;
}

vga_result_t vga_set_scroll(vga_t* vga, vga_scroll_t scroll)
{
    vga->scroll = scroll;
    return VGA_RESULT_OK;
}

vga_result_t vga_scroll(vga_t* vga)
{
    switch (vga->scroll)
    {
        case (VGA_SCROLL_VERTICAL): {
            
            break;
        }
    }
}