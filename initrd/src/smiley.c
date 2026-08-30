#include "syscall.h"

int main(int argc, char** argv)
{
    write(1, ":)\n", 3);
    return 0;
}