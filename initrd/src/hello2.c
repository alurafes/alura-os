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
    syscall1(10, (int)"i have been execve'd\n");
    syscall1(5, (int)99);
    while (1)
    {
        syscall1(10, (int)"i am task two\n");
    }
}