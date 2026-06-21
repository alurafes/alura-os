static inline int syscall1(int n, int a1)
{
    int ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1)
        : "ecx", "edx", "memory"
    );

    return ret;
}

void _start(void)
{
    while (1)
    {
        syscall1(4, (int)"Hello from ELF!\n");
    }
}