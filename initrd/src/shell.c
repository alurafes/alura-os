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

int main(int argc, char** argv)
{
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

        write(STDOUT, &character, 1);
    }

    return 0;
}