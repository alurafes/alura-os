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


static const char keymap[128] =
{
    0,
    0,
    '1','2','3','4','5','6','7','8','9','0',
    '-','=',
    '\b',
    '\t',
    'q','w','e','r','t','y','u','i','o','p',
    '[',']',
    '\n',
    0,
    'a','s','d','f','g','h','j','k','l',
    ';','\'','`',
    0,
    '\\',
    'z','x','c','v','b','n','m',
    ',', '.', '/',
    0,
    '*',
    0,
    ' ',
};

static const char keymap_shift[128] =
{
    0,
    0,
    '!','@','#','$','%','^','&','*','(',')',
    '_','+',
    '\b',
    '\t',
    'Q','W','E','R','T','Y','U','I','O','P',
    '{','}',
    '\n',
    0,
    'A','S','D','F','G','H','J','K','L',
    ':','"','~',
    0,
    '|',
    'Z','X','C','V','B','N','M',
    '<','>','?',
    0,
    '*',
    0,
    ' ',
};

void _start(void)
{
    int shift = 0;
    for (;;)
    {
        unsigned char scancode;
        int read = syscall3(SYSCALL_READ, STDIN, (int)&scancode, 1);
        if (read <= 0) continue;
        
        unsigned char released = scancode & 0x80;
        unsigned char key = scancode & 0x7F;

        if (key == 42 || key == 54)
        {
            shift = !released;
            continue;
        }

        if (released) continue;

        char character = shift ? keymap_shift[key] : keymap[key];
        if (character == 0) continue;

        syscall3(SYSCALL_WRITE, STDOUT, (int)&character, 1);
    }

    syscall1(SYSCALL_EXIT, 0);
}