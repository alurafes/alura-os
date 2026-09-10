#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "syscall.h"
#include "sys/ioctl.h"

static void pack_pixel(uint8_t* dest, uint32_t color, uint8_t bpp)
{
    switch (bpp)
    {
        case 32:
        {
            dest[0] = color & 0xFF;
            dest[1] = (color >> 8) & 0xFF;
            dest[2] = (color >> 16) & 0xFF;
            dest[3] = (color >> 24) & 0xFF;
            break;
        }
        case 24:
        {
            dest[0] = color & 0xFF;
            dest[1] = (color >> 8) & 0xFF;
            dest[2] = (color >> 16) & 0xFF;
            break;
        }
        case 16:
        {
            dest[0] = color & 0xFF;
            dest[1] = (color >> 8) & 0xFF;
            break;
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 8)
    {
        fprintf(stderr, "usage: square <x> <y> <w> <h> <r> <g> <b>\n");
        return 1;
    }

    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    int w = atoi(argv[3]);
    int h = atoi(argv[4]);
    uint8_t r = (uint8_t)atoi(argv[5]);
    uint8_t g = (uint8_t)atoi(argv[6]);
    uint8_t b = (uint8_t)atoi(argv[7]);

    int fd = open("/dev/framebuffer", O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "failed to open /dev/framebuffer\n");
        return 1;
    }

    struct fb_info info;
    if (ioctl(fd, FBIOGET_INFO, (int)(intptr_t)&info) < 0)
    {
        fprintf(stderr, "failed to query framebuffer info\n");
        close(fd);
        return 1;
    }

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > (int)info.width) w = (int)info.width - x;
    if (y + h > (int)info.height) h = (int)info.height - y;

    if (w <= 0 || h <= 0)
    {
        close(fd);
        return 0;
    }

    uint32_t bytes_per_pixel = info.bpp / 8;
    uint32_t color = ((uint32_t)r << info.red_field_position) |
                      ((uint32_t)g << info.green_field_position) |
                      ((uint32_t)b << info.blue_field_position);

    uint8_t* row = malloc((size_t)w * bytes_per_pixel);
    if (row == NULL)
    {
        fprintf(stderr, "out of memory\n");
        close(fd);
        return 1;
    }

    for (int col = 0; col < w; ++col)
    {
        pack_pixel(row + (size_t)col * bytes_per_pixel, color, info.bpp);
    }

    for (int line = 0; line < h; ++line)
    {
        int offset = (y + line) * (int)info.pitch + x * (int)bytes_per_pixel;
        lseek(fd, offset, SEEK_SET);
        write(fd, row, (unsigned int)((size_t)w * bytes_per_pixel));
    }

    free(row);
    close(fd);

    return 0;
}
