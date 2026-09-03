#ifndef ALURA_USER_SYSCALL_H
#define ALURA_USER_SYSCALL_H

#define SYSCALL_OPEN 0
#define SYSCALL_CLOSE 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_FORK 4
#define SYSCALL_EXECVE 5
#define SYSCALL_EXIT 6
#define SYSCALL_WAITPID 7
#define SYSCALL_SBRK 8
#define SYSCALL_ISATTY 9
#define SYSCALL_GETPID 10
#define SYSCALL_LSEEK 11

#define STDIN 0
#define STDOUT 1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR   0x0002
#define O_APPEND 0x0008
#define O_CREAT  0x0200
#define O_TRUNC  0x0400
#define O_EXCL   0x0800

static inline int syscall0(int n)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(n) : "ecx", "edx", "memory");
    return ret;
}

static inline int syscall1(int n, int a1)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(n), "b"(a1) : "ecx", "edx", "memory");
    return ret;
}

static inline int syscall2(int n, int a1, int a2)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(n), "b"(a1), "c"(a2) : "edx", "memory");
    return ret;
}

static inline int syscall3(int n, int a1, int a2, int a3)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(n), "b"(a1), "c"(a2), "d"(a3) : "memory");
    return ret;
}

int open(const char* path, int flags);
int close(int fd);
int read(int fd, void* buf, unsigned int len);
int write(int fd, const void* buf, unsigned int len);
int fork(void);
int execve(const char* path, char* const argv[]);
void exit(int code) __attribute__((noreturn));
int waitpid(int pid, int* status);
void* sbrk(int increment);
int isatty(int fd);
int getpid(void);
int lseek(int fd, int offset, int whence);

#endif