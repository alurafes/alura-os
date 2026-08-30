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

static inline int syscall2(int n, int a1, int a2)
{
    int ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1), "c"(a2)
        : "edx", "memory"
    );

    return ret;
}

#define SYS_FORK 3
#define SYS_EXECVE 4
#define SYS_WAITPID 6

void _start(void)
{
    while (1)
    {
        syscall2(SYS_WAITPID, -1, 0);
    }
}
