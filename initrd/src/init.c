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

static inline int syscall3(int n, int a1, int a2, int a3)
{
    int ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1), "c"(a2), "d"(a3)
        : "memory"
    );

    return ret;
}

#define SYS_OPEN 0
#define SYS_READ 2
#define SYS_FORK 4
#define SYS_EXECVE 5
#define SYS_WAITPID 7
#define SYS_PRINT 10

void _start(void)
{
    int keyboard = syscall1(SYS_OPEN, (int)"/dev/keyboard");
    while (1)
    {
        unsigned char scancode;
        int read = syscall3(SYS_READ, keyboard, (int)&scancode, 1);
        if (read <= 0) continue;
        syscall2(SYS_PRINT, (int)"Scancode: %x\n", (int)scancode);
    }
}
