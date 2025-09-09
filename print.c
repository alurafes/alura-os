#include "print.h"

int vsnprintf(char* buffer, size_t buffer_len, const char* fmt, va_list args)
{
    size_t written = 0;
    for (const char* c = fmt; *c; ++c)
    {
        if (written + 1 >= buffer_len) break;
        if (*c != '%')
        {
            buffer[written] = *c;
            written++;
            continue;
        }
        c++;
        switch (*c)
        {
            case 's': {
                const char* string = va_arg(args, const char*);
                while (*string && written + 1 < buffer_len) {
                    buffer[written] = *string;
                    written++;
                    string++;
                }
                goto parameter_switch_end;
            }
            case 'd': {
                int number = va_arg(args, int);
                if (number < 0)
                {
                    number = -number;
                    buffer[written] = '-';
                    written++;
                }
                char reverse_number_buffer[FORMATTED_PRINT_NUMBER_BUFFER_LENGTH];
                int reverse_number_buffer_iterator = 0;
                do
                {
                    reverse_number_buffer[reverse_number_buffer_iterator] = '0' + (number % 10);
                    reverse_number_buffer_iterator++;
                    number /= 10;
                }
                while (number && reverse_number_buffer_iterator + 1 < FORMATTED_PRINT_NUMBER_BUFFER_LENGTH);
                while (reverse_number_buffer_iterator && written + 1 < buffer_len)
                {
                    reverse_number_buffer_iterator--;
                    buffer[written] = reverse_number_buffer[reverse_number_buffer_iterator];
                    written++;
                }
                goto parameter_switch_end;
            }
            case 'c': {
                buffer[written] = (char)va_arg(args, int);
                written++;
                goto parameter_switch_end;
            }
            case 'x': {
                unsigned int number = va_arg(args, unsigned int);
                char reverse_number_buffer[FORMATTED_PRINT_NUMBER_BUFFER_LENGTH];
                int reverse_number_buffer_iterator = 0;
                do
                {
                    unsigned int digit = number % 16;
                    reverse_number_buffer[reverse_number_buffer_iterator] = digit < 10 ? '0' + digit : 'A' + digit - 10;
                    reverse_number_buffer_iterator++;
                    number /= 16;
                }
                while (number && reverse_number_buffer_iterator + 1 < FORMATTED_PRINT_NUMBER_BUFFER_LENGTH);
                buffer[written] = '0';
                buffer[written + 1] = 'x';
                written += 2;
                while (reverse_number_buffer_iterator && written + 1 < buffer_len)
                {
                    reverse_number_buffer_iterator--;
                    buffer[written] = reverse_number_buffer[reverse_number_buffer_iterator];
                    written++;
                }
                goto parameter_switch_end;
            }
            default: {
                buffer[written] = '%';
                written++;
                if (written + 1 < buffer_len) buffer[written] = *c;
                written++;
                goto parameter_switch_end;
            }
            parameter_switch_end:
        }
    }
    buffer[written] = '\0';
    return written;
}

int printf(const char* fmt, ...)
{
    char buffer[FORMATTED_PRINT_BUFFER];
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    terminal_put_string(&terminal, buffer);
    return written;
}