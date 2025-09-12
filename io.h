#ifndef ALURA_IO_H
#define ALURA_IO_H

#include "stdint.h"

static inline void io_outb(uint16_t port, uint8_t data)
{
    __asm__ volatile (
        "outb %b0, %w1" : : "a"(data), "Nd"(port) : "memory"
    );
}

static inline uint8_t io_inb(uint16_t port)
{
    uint8_t data;
    __asm__ volatile (
        "inb %w1, %b0" : "=a"(data) : "Nd"(port) : "memory"
    );
    return data;
}

static inline void io_wait()
{
    io_outb(0x80, 0);
}

#endif // ALURA_IO_H