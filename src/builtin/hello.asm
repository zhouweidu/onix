[bits 32]

section .text
global _start

_start:
    mov eax,4
    mov ebx,1
    mov ecx,message
    mov edx,message.end-message-1
    int 0x80
    mov eax,1
    mov ebx,0
    int 0x80

section .data

message:
    db "hello,onix!!!",10,0
    .end:

section .bss

buffer: resb 1024