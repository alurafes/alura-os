elf:
	i686-elf-gcc -m32 -ffreestanding -fno-pic -fno-pie -nostartfiles -nodefaultlibs -nostdlib -c test.c -o test.o
	i686-elf-ld -T linker.ld test.o -o test.elf
	xxd -i test.elf > test_elf.h