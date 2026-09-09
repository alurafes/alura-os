#include "syscall.h"
#include <errno.h>

static int result_to_errno(int ret)
{
    switch (-ret)
    {
        case SYSCALL_RESULT_BAD_PARAMETER: return EINVAL;
        case SYSCALL_RESULT_BUSY: return EBUSY;
        case SYSCALL_RESULT_OUT_OF_MEMORY: return ENOMEM;
        case SYSCALL_RESULT_WOULD_BLOCK: return EAGAIN;
        default: return EIO;
    }
}

int _read(int fd, char* buf, int len)
{
    int ret = read(fd, buf, (unsigned int)len);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

int _write(int fd, char* buf, int len)
{
    int ret = write(fd, buf, (unsigned int)len);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

int _open(const char* name, int flags, int mode)
{
    (void)mode;
    int ret = open(name, flags);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

int _close(int fd)
{
    int ret = close(fd);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

int _lseek(int fd, int ptr, int dir)
{
    int ret = lseek(fd, ptr, dir);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

int _fstat(int fd, struct stat* st)
{
    int ret = fstat(fd, st);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

int _stat(const char* file, struct stat* st)
{
    int ret = stat(file, st);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

int _isatty(int fd)
{
    return isatty(fd);
}

int _link(const char* old_path, const char* new_path)
{
    int ret = link(old_path, new_path);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

int _unlink(const char* path)
{
    int ret = unlink(path);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

void* _sbrk(int increment)
{
    int ret = (int)sbrk(increment);
    if (ret < 0) { errno = result_to_errno(ret); return (void*)-1; }
    return (void*)ret;
}

int _getpid(void)
{
    return getpid();
}

int _kill(int pid, int sig)
{
    int ret = kill(pid, sig);
    if (ret < 0) { errno = result_to_errno(ret); return -1; }
    return ret;
}

void _exit(int code)
{
    exit(code);
    for (;;) { }
}
