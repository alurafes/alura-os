#ifndef ALURA_MEMORY_H
#define ALURA_MEMORY_H

extern char _kernel_start;
extern char _kernel_end;

#define ALIGN_UP_TO_SPECIFIC_PAGE(value, page_size) (value + page_size - 1) & ~(page_size - 1)
#define ALIGN_DOWN_TO_SPECIFIC_PAGE(value, page_size) ((value) & ~(page_size - 1))
#define PAGE_SIZE 4096
#define ALIGN_UP(value) (ALIGN_UP_TO_SPECIFIC_PAGE(value, PAGE_SIZE))
#define ALIGN_DOWN(value) (ALIGN_DOWN_TO_SPECIFIC_PAGE(value, PAGE_SIZE))

#endif // ALURA_MEMORY_H