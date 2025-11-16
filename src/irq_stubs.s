global irq_stubs
extern irq_stub_handler

%macro IRQ_STUB 1
global irq%1
irq%1:
    push dword 0
    push dword %1 + 32
    jmp irq_stub_handler
%endmacro

%assign i 0
%rep 16
    IRQ_STUB i
%assign i i+1
%endrep

; setup the irq_stubs variable to use in C
irq_stubs:
%assign i 0
%rep 16
    dd irq%+i

%assign i i+1
%endrep