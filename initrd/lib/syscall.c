#include "syscall.h"

int open(const char* path, int flags) {
    return syscall2(SYSCALL_OPEN, (int)path, flags);
}

int close(int resource) {
    return syscall1(SYSCALL_CLOSE, resource);
}

int read(int resource, void* buffer, unsigned int length) {
    return syscall3(SYSCALL_READ, resource, (int)buffer, (int)length);
}

int write(int resource, const void* buffer, unsigned int length) {
    return syscall3(SYSCALL_WRITE, resource, (int)buffer, (int)length);
}

int fork(void) {
    return syscall0(SYSCALL_FORK);
}

int execve(const char* path, char* const argv[]) {
    return syscall2(SYSCALL_EXECVE, (int)path, (int)argv);
}

void exit(int code)
{
    syscall1(SYSCALL_EXIT, code);
    for (;;) { }
}

int waitpid(int pid, int* status) {
    return syscall2(SYSCALL_WAITPID, pid, (int)status);
}

void* sbrk(int increment) {
    return (void*)syscall1(SYSCALL_SBRK, increment);
}

int isatty(int fd) {
    return syscall1(SYSCALL_ISATTY, fd);
}

int getpid(void) {
    return syscall0(SYSCALL_GETPID);
}

int lseek(int fd, int offset, int whence) {
    return syscall3(SYSCALL_LSEEK, fd, offset, whence);
}