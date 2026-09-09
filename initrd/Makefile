TOOLCHAIN := $(HOME)/opt/cross-alura/bin
CC        := $(TOOLCHAIN)/i686-elf-gcc
AS        := nasm

CFLAGS  := -ffreestanding -nostartfiles -Iinclude
ASFLAGS := -f elf32
LDFLAGS := -ffreestanding -nostartfiles -T linker.ld

BUILDDIR := build
BINDIR   := initrd/bin

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,$(BUILDDIR)/%.o,$(SRCS))
ELFS := $(patsubst src/%.c,$(BINDIR)/%.elf,$(SRCS))

CRT0_OBJ := $(BUILDDIR)/lib/crt0.o
LIBC_OBJS := $(BUILDDIR)/lib/syscall.o $(BUILDDIR)/lib/newlib_glue.o
STARTUP_OBJS := $(CRT0_OBJ) $(LIBC_OBJS)

all: $(ELFS)

$(BUILDDIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/lib/%.o: lib/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/lib/%.o: lib/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BINDIR)/%.elf: $(BUILDDIR)/%.o $(STARTUP_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(CRT0_OBJ) $< $(LIBC_OBJS) -o $@

clean:
	rm -rf $(BUILDDIR) $(BINDIR)