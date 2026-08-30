#include "syscall.h"

static int str_to_int(const char* s)
{
    int sign = 1;
    if (*s == '-')
    {
        sign = -1;
        s++;
    }

    int n = 0;
    while (*s >= '0' && *s <= '9')
    {
        n = n * 10 + (*s - '0');
        s++;
    }

    return n * sign;
}

static int int_to_str(int value, char* buf)
{
    char tmp[12];
    int i = 0;
    int negative = value < 0;
    if (negative) value = -value;

    if (value == 0) tmp[i++] = '0';
    while (value > 0)
    {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int len = 0;
    if (negative) buf[len++] = '-';
    while (i > 0) buf[len++] = tmp[--i];

    return len;
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        write(STDOUT, "usage: calc <a> <+|-|*|/> <b>\n", 30);
        return 1;
    }

    int a = str_to_int(argv[1]);
    char op = argv[2][0];
    int b = str_to_int(argv[3]);

    if (op != '+' && op != '-' && op != '*' && op != '/')
    {
        write(STDOUT, "usage: calc <a> <+|-|*|/> <b>\n", 30);
        return 1;
    }

    int result = 0;
    switch (op)
    {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': result = a / b; break;
    }

    char buf[16];
    int len = int_to_str(result, buf);
    buf[len++] = '\n';
    write(STDOUT, buf, len);

    return 0;
}
