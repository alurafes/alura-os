OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

PHYSICAL_START = 0x00100000;
VIRTUAL_START = 0xC0000000;
VIRTUAL_KERNEL_START = VIRTUAL_START - PHYSICAL_START;

SECTIONS {
    . = PHYSICAL_START;

    _kernel_physical_start = .;

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

    . += VIRTUAL_START;

    .text ALIGN(4096) : AT(ADDR(.text) - VIRTUAL_KERNEL_START) {
        *(.text*)
    }

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - VIRTUAL_KERNEL_START) {
        *(.rodata*)
    }

    .data ALIGN(4096) : AT(ADDR(.data) - VIRTUAL_KERNEL_START) {
        *(.data*)
        *(.data.*)
    }

    .bss ALIGN(4096) : AT(ADDR(.bss) - VIRTUAL_KERNEL_START) {
        *(COMMON)
        *(.bss*)
    }

  _kernel_physical_end = . - VIRTUAL_KERNEL_START;
}