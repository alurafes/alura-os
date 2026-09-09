#ifndef ALURA_SYS_WAIT_H
#define ALURA_SYS_WAIT_H

#define WIFEXITED(status)   (((status) & 0x7f) == 0)
#define WEXITSTATUS(status) (((status) >> 8) & 0xff)
#define WIFSIGNALED(status) ((((status) & 0x7f) + 1) >> 1 > 0)
#define WTERMSIG(status)    ((status) & 0x7f)

#endif // ALURA_SYS_WAIT_H
