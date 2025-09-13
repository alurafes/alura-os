; ; Multiboot 2 stuff

; MB2_MAGIC equ 0xe85250d6  ; multiboot 2 magic
; MB2_TYPE equ 0 ; 32-bit (protected) mode of i386.
; MB2_HEADER_LENGTH equ (mb2_header_end - mb2_header)
; MB2_CHECKSUM equ -(MB2_MAGIC + MB2_TYPE + MB2_HEADER_LENGTH)
; MB2_TAG_END equ 0
; MB2_TAG_FRAMEBUFFER equ 5

; ; Framebuffer blah blah

; FRAMEBUFFER_WIDTH equ 1024
; FRAMEBUFFER_HEIGHT equ 768
; FRAMEBUFFER_BPP equ 32

; ; ----------------

; %macro MB2_TAG 3
;     align 8
;     dw %1        ; type
;     dw %2        ; flags
;     dd %3        ; size
; %endmacro

; section .multiboot2
; align 8
;     dd MB2_MAGIC
;     dd MB2_TYPE
;     dd MB2_HEADER_LENGTH
;     dd MB2_CHECKSUM
; mb2_header:
;     MB2_TAG MB2_TAG_FRAMEBUFFER, 0, 20
;     dd FRAMEBUFFER_WIDTH
;     dd FRAMEBUFFER_HEIGHT
;     dd FRAMEBUFFER_BPP

;     MB2_TAG MB2_TAG_END, 0, 8
; mb2_header_end:

; ----------------

; I gave up on Multiboot 2 for now as it load into 64 bit protected mode which is a bit too early for me :D.
; Gotta learn the basics first. MB1 for now

; ----------------

; Multiboot 1

; ----------------

MB_MAGIC equ 0x1BADB002
MB_FLAGS equ 0
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)

section .multiboot
    align 4
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM

section .text
global _start
extern kernel_main
_start:
    mov esp, stack_top
    push eax
    push ebx
    call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: