[bits 32]

extern device_init
extern console_init
extern gdt_init
extern memory_init
extern kernel_init
extern gdt_ptr

code_selector equ (1 << 3)
data_selector equ (2 << 3)

section .text
global _start
_start:
    push ebx; ards_count
    push eax; magic

    call device_init
    call console_init   ; 控制台初始化

    call gdt_init       ; 全局描述符初始化

    lgdt [gdt_ptr]

    jmp dword code_selector:_next
_next:

    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax; 初始化段寄存器

    call memory_init    ; 内存初始化
    ; xchg bx, bx

    mov esp, 0x10000; 修改栈顶
    ; xchg bx, bx
    call kernel_init    ; 内核初始化

    jmp $; 阻塞
