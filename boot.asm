section .multiboot
align 4
    dd 0x1BADB002          ; magic
    dd 0x03                ; flags (page align + memory info)
    dd -(0x1BADB002 + 0x03) ; checksum

section .text
global start
extern kernel_main

start:
    mov esp, stack_top
    push eax               ; push magic
    push ebx               ; push multiboot info
    call kernel_main
    cli
    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
