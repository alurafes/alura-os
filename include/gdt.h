#ifndef ALURA_GDT_H
#define ALURA_GDT_H

#include <stdint.h>

#include "tss.h"

#define GDT_ENTRIES_COUNT 6 // None, Kernel Code, Kernel Data, User Code, User Data, TSS

#define GDT_FLAG_PRESENT 0x80
#define GDT_FLAG_RING0 0x00
#define GDT_FLAG_RING3 0x60
#define GDT_FLAG_SEGMENT 0x10
#define GDT_FLAG_SYSTEM 0x00
#define GDT_FLAG_EXECUTABLE 0x08
#define GDT_FLAG_READ_WRITE 0x02
#define GDT_FLAG_TSS 0x09

#define GDT_GRANULARITY_BYTE 0x00
#define GDT_GRANULARITY_4K 0x80
#define GDT_GRANULARITY_32BIT 0x40

typedef enum gdt_result_t {
    GDT_RESULT_OK,
    GDT_RESULT_OUT_OF_BOUNDS
} gdt_result_t;

typedef struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_pointer_t {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_pointer_t;

typedef struct gdt_t {
    gdt_entry_t entries[GDT_ENTRIES_COUNT];
    gdt_pointer_t pointer;
} gdt_t;

gdt_result_t gdt_create(gdt_t* gdt);
gdt_result_t gdt_set_entry(gdt_t* gdt, int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void gdt_flush(gdt_pointer_t* pointer);

enum {
    GDT_NULL = 0,
    GDT_KERNEL_CODE,
    GDT_KERNEL_DATA,
    GDT_USER_CODE,
    GDT_USER_DATA,
    GDT_TSS,
};

extern gdt_t gdt;
void gdt_module_init();

#endif // ALURA_GDT_H