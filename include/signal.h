#ifndef ALURA_SIGNAL_H
#define ALURA_SIGNAL_H

#define SYSCALL_SIGHUP  1
#define SYSCALL_SIGINT  2
#define SYSCALL_SIGQUIT 3
#define SYSCALL_SIGILL  4
#define SYSCALL_SIGTRAP 5
#define SYSCALL_SIGABRT 6
#define SYSCALL_SIGBUS  7
#define SYSCALL_SIGFPE  8
#define SYSCALL_SIGKILL 9
#define SYSCALL_SIGUSR1 10
#define SYSCALL_SIGSEGV 11
#define SYSCALL_SIGUSR2 12
#define SYSCALL_SIGPIPE 13
#define SYSCALL_SIGALRM 14
#define SYSCALL_SIGTERM 15

#define SYSCALL_NSIG 32

#define SYSCALL_SIG_DFL ((void (*)(int))0)
#define SYSCALL_SIG_IGN ((void (*)(int))1)

// no sa_mask and sa_flags support
struct sigaction {
    void (*sa_handler)(int);
};

#endif // ALURA_SIGNAL_H
