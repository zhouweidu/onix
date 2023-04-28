[bits 32]

extern kernel_init

global _start
_start:
    ; mov byte [0xb8000], 'K' 
    ; bochs的魔术断点可以去掉了，因为用的是qemu
    ; xchg bx, bx
    call kernel_init
    int 0x80
    ; mov bx, 0
    ; div bx
    jmp $