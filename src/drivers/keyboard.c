#include "drivers/keyboard.h"

#include "print.h"

resource_result_t keyboard_read(resource_t* resource, size_t offset, void* buffer, size_t length, size_t* read_bytes)
{
    keyboard_t* keyboard = (keyboard_t*)resource->data;

    uint8_t* out = (uint8_t*)buffer;
    size_t written = 0;

    while (written < length && keyboard->buffer_count > 0)
    {
        out[written++] = keyboard->buffer[keyboard->buffer_tail];
        keyboard->buffer_tail = (keyboard->buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
        keyboard->buffer_count--;
    }

    if (written == 0)
    {
        *read_bytes = 0;
        return RESOURCE_RESULT_WILL_BLOCK;
    }

    *read_bytes = written;
    return RESOURCE_RESULT_OK;
}

void keyboard_buffer_write(uint8_t scancode)
{
    if (keyboard.buffer_count == KEYBOARD_BUFFER_SIZE) return;

    keyboard.buffer[keyboard.buffer_head] = scancode;
    keyboard.buffer_head = (keyboard.buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard.buffer_count++;
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
            break;

        case 29: // ctrl
            keyboard.special_state.ctrl = !released;
            break;

        case 56: // alt
            keyboard.special_state.alt = !released;
            break;
    }

    keyboard_buffer_write(scancode);
    task_manager_wake_blocked_on(&task_manager, TASK_WAIT_REASON_IO, &keyboard);
}

keyboard_t keyboard;
void keyboard_driver_init()
{
    irq_register_handler(&irq, 1, keyboard_irq_handler);
}

resource_operations_t keyboard_operations = {
    .read = keyboard_read
};


resource_result_t keyboard_open(task_t* task, size_t* result)
{
    return resource_register(task, RESOURCE_TYPE_KEYBOARD, &keyboard, &keyboard_operations, result);
}