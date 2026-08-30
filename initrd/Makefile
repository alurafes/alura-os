CC      := gcc
LD      := ld
AS      := nasm

CFLAGS  := -m32 -ffreestanding -fno-pic -fno-pie -nostartfiles -nodefaultlibs -nostdlib -Iinclude
ASFLAGS := -f elf32
LDFLAGS := -m elf_i386 -n -T linker.ld

BUILDDIR := build
BINDIR   := initrd/bin

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,$(BUILDDIR)/%.o,$(SRCS))
ELFS := $(patsubst src/%.c,$(BINDIR)/%.elf,$(SRCS))

CRT0_OBJ := $(BUILDDIR)/lib/crt0.o
LIBC_OBJ := $(BUILDDIR)/lib/syscall.o
STARTUP_OBJS := $(CRT0_OBJ) $(LIBC_OBJ)

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
	$(LD) $(LDFLAGS) $(CRT0_OBJ) $< $(LIBC_OBJ) -o $@

clean:
	rm -rf $(BUILDDIR) $(BINDIR)