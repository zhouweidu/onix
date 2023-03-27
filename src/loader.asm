[org 0x1000]

dw 0x55aa


mov si, loading
call print

detect_memory:
    xor ebx, ebx
    mov ax, 0
    mov es, ax
    mov edi, ards_buffer

    mov edx, 0x534d4150

.next:
    mov eax, 0xe820
    mov ecx, 20
    int 0x15

    jc error
    add di, cx
    inc word [ards_count]
    cmp ebx, 0
    jnz .next

    mov si, detecting
    call print
    xchg bx, bx
    mov byte [0xb8000], 'P'
    jmp prepare_protected_mode
    

prepare_protected_mode:
    xchg bx, bx
    cli
    ;打开A20线
    in al, 0x92
    or al, 0b10
    out 0x92, al
    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp dword code_selector:protect_mode


print:
    mov ah, 0x0e
.next:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10
    inc si
    jmp .next
.done:
    ret

loading:
    db "Loading Onix...", 10, 13, 0;\n \r
detecting:
    db "Detecting Memory Success...", 10, 13, 0;\n \r

error:
    mov si, .msg
    call print
    hlt ;让cpu停止
    jmp $
    .msg db "Loading Error!!!", 10, 13, 0

[bits 32]
protect_mode:
    xchg bx, bx
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x10000

    mov byte [0xb8000], 'P'

    mov byte [0x200000], 'P'

    jmp $


    

code_selector equ (1<<3)
data_selector equ (2<<3)

memory_base equ 0
memory_limit equ (((1024*1024*1024*4)/(1024*4))-1)

gdt_ptr:
    dw (gdt_end-gdt_base)-1
    dd gdt_base
gdt_base:
    dd 0, 0
gdt_code:
    dw memory_limit & 0xffff
    dw memory_base & 0xffff
    db (memory_base>>16) & 0xff
    db 0b1_00_1_1_0_1_0
    db 0b1_1_0_0_0000 | ((memory_limit>>16) & 0xf)
    db (memory_base>>24) & 0xff
gdt_data:
    dw memory_limit & 0xffff
    dw memory_base & 0xffff
    db (memory_base>>16) & 0xff
    db 0b1_00_1_0_0_1_0
    db 0b1_1_0_0_0000 | ((memory_limit>>16) & 0xf)
    db (memory_base>>24) & 0xff
gdt_end:

ards_count:
    dw 0
ards_buffer:
