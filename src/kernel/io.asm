[bits 32]

section .text

global inb ;将inb导出
inb:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp+8]
    in al, dx
    jmp $+2;跳转到下一行
    jmp $+2
    jmp $+2

    leave
    ret

global outb ;将outb导出
outb:
    push ebp
    mov ebp, esp;保存栈帧
    mov edx, [ebp+8];port
    mov eax, [ebp+12];value
    out dx, al
    jmp $+2;跳转到下一行
    jmp $+2
    jmp $+2

    leave
    ret

global inw 
inw:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp+8]
    in ax, dx
    jmp $+2;跳转到下一行
    jmp $+2
    jmp $+2

    leave
    ret

global outw
outw:
    push ebp
    mov ebp, esp;保存栈帧
    mov edx, [ebp+8];port
    mov eax, [ebp+12];value
    out dx, ax
    jmp $+2;跳转到下一行
    jmp $+2
    jmp $+2

    leave
    ret
