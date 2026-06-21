#include "drivers/keyboard.h"

#include "print.h"

// todo: switch to a keycode system or something like that. Hardcoding latin for now
static const char keyboard_map[128] =
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

static const char keyboard_map_shift[128] =
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

char keyboard_translate(keyboard_t* keyboard, uint8_t scancode)
{
    if (scancode >= 128)
    {
        return 0;
    }

    if (keyboard->special_state.shift)
    {
        return keyboard_map_shift[scancode];
    }

    return keyboard_map[scancode];
}

void keyboard_irq_handler(register_interrupt_data_t* data)
{
    (void)(data);
    uint8_t scancode = io_inb(0x60);
    uint8_t released = scancode & 0x80;
    uint8_t key = scancode & 0x7F;

    switch (key)
    {
        case 42: // left shift
        case 54: // right shift
            keyboard.special_state.shift = !released;
            return;

        case 29: // ctrl
            keyboard.special_state.ctrl = !released;
            return;

        case 56: // alt
            keyboard.special_state.alt = !released;
            return;
    }

    if (released)
    {
        return;
    }
    
    char character = keyboard_translate(&keyboard, scancode);
    printf("%c", character);
}

keyboard_t keyboard;
void keyboard_driver_init()
{
    irq_register_handler(&irq, 1, keyboard_irq_handler);
}