#include <stddef.h>
#include "syscall.h"

int main(int argc, char** argv)
{
    int shell_pid = fork();

    if (shell_pid == 0)
    {
        char* shell_argv[] = {"/bin/shell.elf", NULL};
        execve("/bin/shell.elf", shell_argv);
    }

    while (1)
    {
        waitpid(-1, 0);
    }
    
    return 0;
}