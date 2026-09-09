#include <stdio.h>
#include <stdlib.h>
#include "syscall.h"

static void sigfpe_handler(int sig)
{
    (void)sig;
    fprintf(stderr, "calc: division by zero\n");
    exit(1);
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "usage: calc <a> <+|-|*|/> <b>\n");
        return 1;
    }

    int a = atoi(argv[1]);
    char op = argv[2][0];
    int b = atoi(argv[3]);

    if (op != '+' && op != '-' && op != '*' && op != '/')
    {
        fprintf(stderr, "usage: calc <a> <+|-|*|/> <b>\n");
        return 1;
    }

    signal(SIGFPE, sigfpe_handler);

    int result = 0;
    switch (op)
    {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': result = a / b; break;
    }

    printf("%d\n", result);
    return 0;
}
