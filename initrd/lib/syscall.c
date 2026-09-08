#include <stddef.h>
#include "syscall.h"

int open(const char* path, int flags)
{
    return syscall2(SYSCALL_OPEN, (int)path, flags);
}

int close(int resource)
{
    return syscall1(SYSCALL_CLOSE, resource);
}

int read(int resource, void* buffer, unsigned int length)
{
    return syscall3(SYSCALL_READ, resource, (int)buffer, (int)length);
}

int write(int resource, const void* buffer, unsigned int length)
{
    return syscall3(SYSCALL_WRITE, resource, (int)buffer, (int)length);
}

int fork(void)
{
    return syscall0(SYSCALL_FORK);
}

int execve(const char* path, char* const argv[])
{
    return syscall2(SYSCALL_EXECVE, (int)path, (int)argv);
}

void exit(int code)
{
    syscall1(SYSCALL_EXIT, code);
    for (;;) { }
}

int waitpid(int pid, int* status)
{
    return syscall2(SYSCALL_WAITPID, pid, (int)status);
}

void* sbrk(int increment)
{
    return (void*)syscall1(SYSCALL_SBRK, increment);
}

int isatty(int fd)
{
    return syscall1(SYSCALL_ISATTY, fd);
}

int getpid(void)
{
    return syscall0(SYSCALL_GETPID);
}

int lseek(int fd, int offset, int whence)
{
    return syscall3(SYSCALL_LSEEK, fd, offset, whence);
}

int fcntl(int fd, int cmd, int arg)
{
    return syscall3(SYSCALL_FCNTL, fd, cmd, arg);
}

int ioctl(int fd, int cmd, int arg)
{
    return syscall3(SYSCALL_IOCTL, fd, cmd, arg);
}

int fstat(int fd, struct stat* buf)
{
    return syscall2(SYSCALL_FSTAT, fd, (int)buf);
}

int stat(const char* path, struct stat* buf)
{
    return syscall2(SYSCALL_STAT, (int)path, (int)buf);
}

int link(const char* old_path, const char* new_path)
{
    return syscall2(SYSCALL_LINK, (int)old_path, (int)new_path);
}

int unlink(const char* path)
{
    return syscall1(SYSCALL_UNLINK, (int)path);
}

clock_t times(struct tms* buf)
{
    return syscall1(SYSCALL_TIMES, (int)buf);
}

char* environ[] = { NULL };