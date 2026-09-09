#include "idt.h"

#include "print.h"

#include "syscall.h"

idt_result_t idt_create(idt_t* idt)
{
    idt->pointer.limit = sizeof(idt->entries) - 1;
    idt->pointer.base = (uint32_t)&idt->entries;

    for (int i = 0; i < IDT_BASE_ENTRIES_COUNT; ++i)
    {
        idt_set_entry(idt, i, (uint32_t)isr_stubs[i], 0x08, 0x8E); // 0b10001111 - 1 - present | 00 - kernel | 0 - zero | 1110 - interrupt gate
    }

    idt_set_entry(idt, IDT_SYSCALL, (uint32_t)isr_syscall, 0x08, 0xEE);
    idt_flush(&idt->pointer);
    return IDT_RESULT_OK;
}

idt_result_t idt_set_entry(idt_t* idt, int index, uint32_t offset, uint16_t segment_selector, uint8_t type_attributes)
{
    if (index >= IDT_MAX_ENTRIES_COUNT) return IDT_RESULT_OUT_OF_BOUNDS;

    idt->entries[index].offset_low = offset & 0xFFFF;
    idt->entries[index].offset_high = (offset >> 16) & 0xFFFF;
    idt->entries[index].reserved = 0;
    idt->entries[index].segment_selector = segment_selector;
    idt->entries[index].type_attributes = type_attributes;

    return IDT_RESULT_OK;
}

void idt_flush(idt_pointer_t* pointer)
{
    __asm__ volatile ("lidt (%0)" : : "r"(pointer));
}

static const int32_t exception_signals[IDT_BASE_ENTRIES_COUNT] = {
    /* 0 Divide error             */ SYSCALL_SIGFPE,
    /* 1 Debug                    */ SYSCALL_SIGTRAP,
    /* 2 NMI                      */ SYSCALL_SIGABRT,
    /* 3 Breakpoint               */ SYSCALL_SIGTRAP,
    /* 4 Overflow                 */ SYSCALL_SIGFPE,
    /* 5 Bound range exceeded     */ SYSCALL_SIGSEGV,
    /* 6 Invalid opcode           */ SYSCALL_SIGILL,
    /* 7 Device not available     */ SYSCALL_SIGFPE,
    /* 8 Double fault             */ SYSCALL_SIGABRT,
    /* 9 Coprocessor overrun      */ SYSCALL_SIGFPE,
    /* 10 Invalid TSS             */ SYSCALL_SIGSEGV,
    /* 11 Segment not present     */ SYSCALL_SIGBUS,
    /* 12 Stack-segment fault     */ SYSCALL_SIGBUS,
    /* 13 General protection      */ SYSCALL_SIGSEGV,
    /* 14 Page fault              */ SYSCALL_SIGSEGV,
    /* 15 reserved                */ SYSCALL_SIGABRT,
    /* 16 x87 FPU error           */ SYSCALL_SIGFPE,
    /* 17 Alignment check         */ SYSCALL_SIGBUS,
    /* 18 Machine check           */ SYSCALL_SIGABRT,
    /* 19 SIMD FP exception       */ SYSCALL_SIGFPE,
    /* 20 Virtualization          */ SYSCALL_SIGSEGV,
    /* 21-31 reserved             */ SYSCALL_SIGABRT, SYSCALL_SIGABRT, SYSCALL_SIGABRT, SYSCALL_SIGABRT,
                                     SYSCALL_SIGABRT, SYSCALL_SIGABRT, SYSCALL_SIGABRT, SYSCALL_SIGABRT,
                                     SYSCALL_SIGABRT, SYSCALL_SIGABRT, SYSCALL_SIGABRT,
};

void isr_handler(register_interrupt_data_t* data)
{
    if (data->interrupt_index < PIC1_REMAPPED_VECTOR)
    {
        if ((data->cs & 0x3) != 3)
        {
            printf("\n\nException #%x: Error Code: %x\nRegisters:\ngs = %x, fs = %x, es = %x, ds = %x\nedi = %x, esi = %x, ebp = %x, ebx = %x, edx = %x, ecx = %x, eax = %x\neip = %x, cs = %x, eflags = %x, useresp = %x, ss = %x",
                data->interrupt_index,
                data->error_code,
                data->gs,
                data->fs,
                data->es,
                data->ds,
                data->edi,
                data->esi,
                data->ebp,
                data->ebx,
                data->edx,
                data->ecx,
                data->eax,
                data->eip,
                data->cs,
                data->eflags,
                data->useresp,
                data->ss);

            // halt if this happened in the kernel
            for(;;) asm("hlt");
        }

        task_t* faulting_task = task_manager.task_current;
        if (faulting_task == NULL) return;

        int32_t sig = exception_signals[data->interrupt_index];
        void (*handler)(int) = faulting_task->signal_handlers[sig];

        if (handler != SYSCALL_SIG_DFL && handler != SYSCALL_SIG_IGN)
        {
            task_manager_deliver_signal(faulting_task, sig, handler);
            return;
        }

        printf("\n\nException #%x: Error Code: %x\nRegisters:\ngs = %x, fs = %x, es = %x, ds = %x\nedi = %x, esi = %x, ebp = %x, ebx = %x, edx = %x, ecx = %x, eax = %x\neip = %x, cs = %x, eflags = %x, useresp = %x, ss = %x",
            data->interrupt_index,
            data->error_code,
            data->gs,
            data->fs,
            data->es,
            data->ds,
            data->edi,
            data->esi,
            data->ebp,
            data->ebx,
            data->edx,
            data->ecx,
            data->eax,
            data->eip,
            data->cs,
            data->eflags,
            data->useresp,
            data->ss);
        printf("\n[terminating user task]\n");

        task_manager_exit_task(&task_manager, faulting_task, sig & 0x7f);
        task_manager_yield_current(&task_manager);

        return;
    }
    if (data->interrupt_index == IDT_SYSCALL)
    {
        syscall_handler(data);
    }
}

idt_t idt;
void idt_module_init()
{
    idt_create(&idt);
}