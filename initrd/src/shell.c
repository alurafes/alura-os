#include <stdio.h>
#include <string.h>
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
#define ARGV_MAX 16

static void put_char(char c)
{
    write(STDOUT, &c, 1);
}

static void put_str(const char* s)
{
    write(STDOUT, s, strlen(s));
}

static int parse_args(char* line, char* argv[])
{
    int argc = 0;
    char* tok = strtok(line, " ");
    while (tok != NULL && argc < ARGV_MAX - 1)
    {
        argv[argc++] = tok;
        tok = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    return argc;
}

static void run_command(char* line)
{
    char* argv[ARGV_MAX];
    int argc = parse_args(line, argv);
    if (argc == 0) return;

    if (strcmp(argv[0], "exit") == 0)
    {
        exit(0);
    }

    int pid = fork();
    if (pid == 0)
    {
        int result = execve(argv[0], argv);
        if (result < 0)
        {
            fprintf(stderr, "unknown command: %s\n", argv[0]);
            exit(result);
        }
    }
    else
    {
        waitpid(-1, 0);
    }
}

int main(int argc, char** argv)
{
    put_str("> ");

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
                put_char(character);
            }
            continue;
        }

        if (character == '\n')
        {
            line[line_length] = 0;
            put_char(character);

            run_command(line);

            line_length = 0;
            put_str("> ");
            continue;
        }

        if (line_length < LINE_MAX - 1)
        {
            line[line_length++] = character;
            put_char(character);
        }
    }

    return 0;
}
