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

#define SYSCALL_OPEN 0
#define SYSCALL_CLOSE 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_FORK 4
#define SYSCALL_EXECVE 5
#define SYSCALL_EXIT 6
#define SYSCALL_WAITPID 7

#define STDIN 0
#define STDOUT 1

void _start(void)
{
    int shell_pid = syscall0(SYSCALL_FORK);

    if (shell_pid == 0) {
        syscall1(SYSCALL_EXECVE, (int)"/bin/shell.elf");
    }

    while (1)
    {
        syscall2(SYSCALL_WAITPID, -1, 0);
    }
}
