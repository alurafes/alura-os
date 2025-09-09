#ifndef ALURA_GDT_H
#define ALURA_GDT_H

#include <stdint.h>

#define GDT_ENTRIES_COUNT 3 // None, Kernel Code, Kernel Data

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
extern void gdt_flush(uint32_t);

extern gdt_t gdt;
void gdt_module_init();

#endif // ALURA_GDT_H