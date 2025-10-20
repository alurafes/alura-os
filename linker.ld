OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

PHYSICAL_START = 0x00100000;
VIRTUAL_START = 0xC0000000;

SECTIONS {
    . = PHYSICAL_START;

    _kernel_start = .;

    .multiboot ALIGN(4) : {
        KEEP(*(.multiboot))
    }

    .bootstrap.text ALIGN(16) : {
        *(.bootstrap.text)
        *(.start)
    }

    .bootstrap.data ALIGN(4096) : {
        *(.bootstrap.data)
        *(.bootstrap.stack)
    }

    _bootstrap_phys_end = .;

    . = ALIGN(4096);

    . = VIRTUAL_START + (_bootstrap_phys_end - PHYSICAL_START);

    .text ALIGN(4096) : AT(ADDR(.text) - VIRTUAL_START + PHYSICAL_START) {
        *(.text*)
    }

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - VIRTUAL_START + PHYSICAL_START) {
        *(.rodata*)
    }

    .data ALIGN(4096) : AT(ADDR(.data) - VIRTUAL_START + PHYSICAL_START) {
        *(.data*)
        *(.data.*)
    }

    .bss ALIGN(4096) : AT(ADDR(.bss) - VIRTUAL_START + PHYSICAL_START) {
        *(COMMON)
        *(.bss*)
    }

  _kernel_end = .;
}