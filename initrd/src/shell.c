#include <stddef.h>
#include "syscall.h"

static const char keymap[128] =
{
    0,
    0,
    '1','2','3','4','5','6','7','8','9','0',
    '-','=',
    '\b',
    '\t',
    'q','w','e','r','t','y','u','i','o','p',
    '[',']',
    '\n',
    0,
    'a','s','d','f','g','h','j','k','l',
    ';','\'','`',
    0,
    '\\',
    'z','x','c','v','b','n','m',
    ',', '.', '/',
    0,
    '*',
    0,
    ' ',
};

static const char keymap_shift[128] =
{
    0,
    0,
    '!','@','#','$','%','^','&','*','(',')',
    '_','+',
    '\b',
    '\t',
    'Q','W','E','R','T','Y','U','I','O','P',
    '{','}',
    '\n',
    0,
    'A','S','D','F','G','H','J','K','L',
    ':','"','~',
    0,
    '|',
    'Z','X','C','V','B','N','M',
    '<','>','?',
    0,
    '*',
    0,
    ' ',
};

#define LINE_MAX 256

static void write_char(int fd, char c)
{
    write(STDOUT, &c, 1);
}

static int str_len(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void write_str(int fd, const char* s)
{
    write(STDOUT, s, str_len(s));
}

static int str_eq(const char* a, const char* b)
{
    while (*a && *b)
    {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static void run_command(int fd, const char* line)
{
    int pid = fork();
    if (pid == 0)
    {
        char* argv[] = {line, NULL};
        int exec = execve(line, argv);
        if (exec < 0)
        {
            write_str(fd, "unknown command: ");
            write_str(fd, line);
            write_char(fd, '\n');
            exit(exec);
        }
    }
    else
    {
        waitpid(-1, 0);
    }
}

int main(int argc, char** argv)
{
    write_str(STDOUT, "> ");

    char line[LINE_MAX];
    int line_length = 0;
    int shift = 0;

    for (;;)
    {
        unsigned char scancode;
        int r = read(STDIN, &scancode, 1);
        if (r <= 0) continue;
        
        unsigned char released = scancode & 0x80;
        unsigned char key = scancode & 0x7F;

        if (key == 42 || key == 54)
        {
            shift = !released;
            continue;
        }

        if (released) continue;

        char character = shift ? keymap_shift[key] : keymap[key];
        if (character == 0) continue;

        if (character == '\b')
        {
            if (line_length > 0)
            {
                line_length--;
                write_char(STDOUT, character);
            }
            continue;
        }

        if (character == '\n')
        {
            line[line_length] = 0;
            write_char(STDOUT, character);

            run_command(STDOUT, line);

            line_length = 0;
            write_str(STDOUT, "> ");
            continue;
        }

        if (line_length < LINE_MAX - 1)
        {
            line[line_length++] = character;
            write_char(STDOUT, character);
        }
    }

    return 0;
}