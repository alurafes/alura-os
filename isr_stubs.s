global isr_stubs
extern isr_stub_handler

%macro ISR_STUB 1
global isr%1
isr%1:
    push dword 0  ; push 0 as "error code" so we can use the same structure as for IST_STUB_EXCEPTION
    push dword %1
    jmp isr_stub_handler
%endmacro

%macro IST_STUB_EXCEPTION 1
global isr%1
isr%1:
    push dword %1
    jmp isr_stub_handler
%endmacro

%assign i 0
%rep 32
%if i = 8 | i = 10 | i = 11 | i = 12 | i = 13 | i = 14 | i = 17
    IST_STUB_EXCEPTION i
%else
    ISR_STUB i
%endif
%assign i i+1
%endrep

%assign i 32
%rep 224

    ISR_STUB i

%assign i i+1
%endrep

; setup the isr_stubs variable to use in C
isr_stubs:
%assign i 0
%rep 256
    dd isr%+i

%assign i i+1
%endrep