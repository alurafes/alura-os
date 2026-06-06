#ifndef ALURA_ELF_EXECUTABLE_H
#define ALURA_ELF_EXECUTABLE_H

#include <stdint.h>
#include <stddef.h>

#include "vfs.h"
#include "task_manager.h"

#include "libc/string.h"

#define ET_EXEC 2

#define EM_386 3

#define EV_CURRENT 1

#define EI_NIDENT 16
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8
#define EI_PAD 9

#define ELFOSABI_NONE 0

#define PT_LOAD 1

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef struct Elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct Elf32_Phdr {
    Elf32_Word    p_type;
    Elf32_Off     p_offset;
    Elf32_Addr    p_vaddr;
    Elf32_Addr    p_paddr;
    Elf32_Word    p_filesz;
    Elf32_Word    p_memsz;
    Elf32_Word    p_flags;
    Elf32_Word    p_align;
} Elf32_Phdr;

typedef enum elf_result_t {
    ELF_RESULT_OK = 0,
    ELF_RESULT_INVALID_ELF,
    ELF_RESULT_BAD_PARAMETER,
    ELF_RESULT_READ_ERROR,
    ELF_RESULT_ALLOCATION_ERROR
} elf_result_t;

elf_result_t elf_load_and_execute(const char* path);

#endif // ALURA_ELF_EXECUTABLE_H