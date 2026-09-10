#ifndef ALURA_SYS_IOCTL_H
#define ALURA_SYS_IOCTL_H

#include <stdint.h>

#define FBIOGET_INFO 1

struct fb_info {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t size;
    uint8_t red_field_position;
    uint8_t green_field_position;
    uint8_t blue_field_position;
};

#endif // ALURA_SYS_IOCTL_H
