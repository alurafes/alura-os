static inline int syscall0(int n)
{
    int ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(n)
        : "ecx", "edx", "memory"
    );

    return ret;
}

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
    int created = syscall0(3);
    if (created == 0) {
        syscall1(4, (int)"Hello from child process\n");
    } else {
        syscall1(4, (int)"Created a child process\n");
    }
    while (1)
    {
        
    }
}