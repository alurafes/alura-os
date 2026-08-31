#include "drivers/framebuffer.h"
#include "drivers/font8x8.h"

#include "memory.h"
#include "memory_paging.h"

framebuffer_t framebuffer;
void framebuffer_driver_init(multiboot_info_t* multiboot)
{
    uint32_t physical_address = (uint32_t)multiboot->framebuffer_addr;
    uint32_t pitch = multiboot->framebuffer_pitch;
    uint32_t height = multiboot->framebuffer_height;
    uint32_t size = pitch * height;

    framebuffer_create(&framebuffer);

    framebuffer.address = memory_paging_map_physical_range(physical_address, size, KERNEL_FRAMEBUFFER_VIRTUAL_START, PAGE_READ_WRITE);
    framebuffer.pitch = pitch;
    framebuffer.width = multiboot->framebuffer_width;
    framebuffer.height = height;
    framebuffer.bpp = multiboot->framebuffer_bpp;
    framebuffer.size = size;

    framebuffer.red_field_position = multiboot->framebuffer_red_field_position;
    framebuffer.green_field_position = multiboot->framebuffer_green_field_position;
    framebuffer.blue_field_position = multiboot->framebuffer_blue_field_position;
}

framebuffer_result_t framebuffer_create(framebuffer_t* out)
{
    out->driver.put_char = framebuffer_put_char;
    out->driver.set_cursor = framebuffer_set_cursor;
    out->driver.get_dimensions = framebuffer_get_dimensions;
    out->foreground = (framebuffer_color_t){ .r = 255, .g = 255, .b = 255 };
    out->background = (framebuffer_color_t){ .r = 0x42, .g = 0x42, .b = 0x42 };
    return FRAMEBUFFER_RESULT_OK;
}

framebuffer_result_t framebuffer_set_color(framebuffer_t* framebuffer, framebuffer_color_t foreground, framebuffer_color_t background)
{
    framebuffer->foreground = foreground;
    framebuffer->background = background;
    return FRAMEBUFFER_RESULT_OK;
}

void framebuffer_put_pixel(framebuffer_t* framebuffer, unsigned int x, unsigned int y, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t* pixel = (uint8_t*)framebuffer->address + (y * framebuffer->pitch) + (x * (framebuffer->bpp / 8));
    uint32_t color = (r << framebuffer->red_field_position) | (g << framebuffer->green_field_position) | (b << framebuffer->blue_field_position);
    if (framebuffer->bpp == 32)
    {
        *pixel = color;
    }
    else if (framebuffer->bpp == 24)
    {
        pixel[0] = color & 0xFF;
        pixel[1] = (color >> 8) & 0xFF;
        pixel[2] = (color >> 16) & 0xFF;
    }
    else if (framebuffer->bpp == 16)
    {
        *(uint16_t*)pixel = (uint16_t)color;
    }
}

void framebuffer_blit(framebuffer_t* framebuffer, int dest_x, int dest_y, framebuffer_sprite_t* source)
{
    if (source == NULL) return;
    if (dest_x >= (int)framebuffer->width || dest_y >= (int)framebuffer->height) return;
    if (dest_x + (int)source->width < 0 || dest_y + (int)source->height < 0) return;

    int src_x = 0;
    int src_y = 0;
    int src_w = source->width;
    int src_h = source->height;

    if (dest_x < 0)
    {
        src_x = -dest_x;
        src_w += dest_x;
        dest_x = 0;
    }

    if (dest_y < 0)
    {
        src_y = -dest_y;
        src_h += dest_y;
        dest_y = 0;
    }

    if (dest_x + src_w > (int)framebuffer->width)
    {
        src_w = framebuffer->width - dest_x;
    }
    if (dest_y + src_h > (int)framebuffer->height)
    { 
        src_h = framebuffer->height - dest_y;
    }

    uint32_t bpp = framebuffer->bpp;
    uint32_t bytes_per_pixel = bpp / 8;

    uint8_t* dest_row = (uint8_t*)framebuffer->address + (dest_y * framebuffer->pitch) + (dest_x * bytes_per_pixel);
    uint32_t* src_row = source->pixels + (src_y * source->width) + src_x;

    for (int y = 0; y < src_h; y++)
    {
        uint8_t* dest_pixel = dest_row;
        uint32_t* src_pixel = src_row;

        for (int x = 0; x < src_w; x++)
        {
            uint32_t raw_color = src_pixel[x];
            uint8_t r = (raw_color >> 16) & 0xFF;
            uint8_t g = (raw_color >> 8) & 0xFF;
            uint8_t b = raw_color & 0xFF;

            uint32_t color = (r << framebuffer->red_field_position) | (g << framebuffer->green_field_position) | (b << framebuffer->blue_field_position);

            if (bpp == 32)
            {
                *(uint32_t*)dest_pixel = color;
                dest_pixel += 4;
            } 
            else if (bpp == 24)
            {
                dest_pixel[0] = color & 0xFF;
                dest_pixel[1] = (color >> 8) & 0xFF;
                dest_pixel[2] = (color >> 16) & 0xFF;
                dest_pixel += 3;
            } 
            else if (bpp == 16)
            {
                *(uint16_t*)dest_pixel = (uint16_t)color;
                dest_pixel += 2;
            }
        }

        dest_row += framebuffer->pitch;
        src_row += source->width;
    }
}

void framebuffer_put_char(text_display_driver_t* driver, char character, unsigned int x, unsigned int y)
{
    framebuffer_t* framebuffer = (framebuffer_t*)driver;

    uint8_t code = (uint8_t)character;
    if (code >= 128) code = '?';
    const uint8_t* glyph = font8x8_basic[code];

    uint32_t fg = ((uint32_t)framebuffer->foreground.r << 16) | ((uint32_t)framebuffer->foreground.g << 8) | framebuffer->foreground.b;
    uint32_t bg = ((uint32_t)framebuffer->background.r << 16) | ((uint32_t)framebuffer->background.g << 8) | framebuffer->background.b;

    uint32_t pixels[FONT8X8_HEIGHT * FONT8X8_WIDTH];
    for (int row = 0; row < FONT8X8_HEIGHT; ++row)
    {
        uint8_t bits = glyph[row];
        for (int col = 0; col < FONT8X8_WIDTH; ++col)
        {
            pixels[row * FONT8X8_WIDTH + col] = (bits & (1 << col)) ? fg : bg;
        }
    }

    framebuffer_sprite_t sprite = {
        .pixels = pixels,
        .width = FONT8X8_WIDTH,
        .height = FONT8X8_HEIGHT
    };

    framebuffer_blit(framebuffer, x * FONT8X8_WIDTH, y * FONT8X8_HEIGHT, &sprite);
}

void framebuffer_set_cursor(text_display_driver_t* driver, unsigned int x, unsigned int y)
{
    framebuffer_t* framebuffer = (framebuffer_t*)driver;
    framebuffer->cursor_x = x;
    framebuffer->cursor_y = y;
    framebuffer_put_char(driver, '_', framebuffer->cursor_x, framebuffer->cursor_y);
}

void framebuffer_get_dimensions(text_display_driver_t* driver, unsigned int* width, unsigned int* height)
{
    framebuffer_t* framebuffer = (framebuffer_t*)driver;

    if (width != NULL) *width = framebuffer->width / FONT8X8_WIDTH;
    if (height != NULL) *height = framebuffer->height / FONT8X8_HEIGHT;
}