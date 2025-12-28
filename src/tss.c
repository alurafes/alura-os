#include "tss.h"

#include "gdt.h"

tss_entry_t tss = {0};
void tss_module_init()
{
    tss.ss0 = (GDT_KERNEL_DATA << 3) | 0;
    tss.iopb = sizeof(tss_entry_t);
    tss_flush();
}


void tss_flush()
{
    __asm__ volatile (
        "mov $0x28, %%ax\n\t" // 0x28 = fifth entry in gdt * 8
        "ltr %%ax\n\t"
        :
        :
        : "ax", "memory"
    );
}
