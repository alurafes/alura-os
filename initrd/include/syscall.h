#ifndef ALURA_USER_SYSCALL_H
#define ALURA_USER_SYSCALL_H

#include "sys/stat.h"
#include "sys/times.h"
#include "sys/wait.h"
#include "signal.h"

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
#define SYSCALL_FCNTL 12
#define SYSCALL_IOCTL 13
#define SYSCALL_FSTAT 14
#define SYSCALL_STAT 15
#define SYSCALL_LINK 16
#define SYSCALL_UNLINK 17
#define SYSCALL_TIMES 18
#define SYSCALL_KILL 19
#define SYSCALL_SIGACTION 20
#define SYSCALL_SIGRETURN 21

#define SYSCALL_RESULT_OK 0
#define SYSCALL_RESULT_FAIL 1
#define SYSCALL_RESULT_BAD_PARAMETER 2
#define SYSCALL_RESULT_BUSY 3
#define SYSCALL_RESULT_OUT_OF_MEMORY 4
#define SYSCALL_RESULT_WOULD_BLOCK 5

#define STDIN 0
#define STDOUT 1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define F_GETFL 1
#define F_SETFL 2

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_ACCMODE 0x0003
#define O_NONBLOCK 0x0004
#define O_APPEND 0x0008
#define O_CREAT 0x0200
#define O_TRUNC 0x0400
#define O_EXCL 0x0800

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
int fcntl(int fd, int cmd, int arg);
int ioctl(int fd, int cmd, int arg);
int fstat(int fd, struct stat* buf);
int stat(const char* path, struct stat* buf);
int link(const char* old_path, const char* new_path);
int unlink(const char* path);
clock_t times(struct tms* buf);
int kill(int pid, int sig);
int raise(int sig);
int sigaction(int sig, const struct sigaction* act, struct sigaction* oldact);
void (*signal(int sig, void (*handler)(int)))(int);

extern char* environ[];

#endif