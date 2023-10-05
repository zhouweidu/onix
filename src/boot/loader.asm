[org 0x1000]

dd 0x55aa

kernel_size: dd KERNEL_SIZE

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
    inc dword [ards_count]
    cmp ebx, 0
    jnz .next

    mov si, detecting
    call print
    ;实模式下不能直接访问16bit以外的内存
    ; xchg bx, bx
    ; mov byte [0xb8000], 'P'
    jmp prepare_protected_mode
    

prepare_protected_mode:
    ; xchg bx, bx
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
    ; xchg bx, bx
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x10000

    mov esp, 0x10000; 修改栈顶

    sub esp, 4 * 3; 三个变量
    mov dword [esp], 0; 读出的数量
    mov dword [esp + 4], 10     ; ecx 初始扇区位置
    mov dword [esp + 8], 0x10000; edi 目标内存位置
    BLOCK_SIZE equ 200          ; 一次读取的扇区数量

.read_block:

    mov edi, [esp + 8]  ; 读取的目标内存
    mov ecx, [esp + 4]  ; 起始扇区
    mov bl, BLOCK_SIZE  ; 扇区数量

    call read_disk ; 读取内核

    add dword [esp + 8], BLOCK_SIZE * 512  ; edi 目标内存位置
    add dword [esp + 4], BLOCK_SIZE        ; ecx 初始扇区位置

    mov eax, [kernel_size]
    add dword [esp], BLOCK_SIZE * 512
    cmp dword [esp], eax; 判断已读数量与 kernel_size 的大小

    jl .read_block

    mov eax, 0x20220205; 内核魔数
    mov ebx, ards_count; ards 数量指针

    jmp dword code_selector:0x10000

    ud2; 表示出错


read_disk:

    ; 设置读写扇区的数量
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    inc dx; 0x1f3
    mov al, cl; 起始扇区的前八位
    out dx, al

    inc dx; 0x1f4
    shr ecx, 8
    mov al, cl; 起始扇区的中八位
    out dx, al

    inc dx; 0x1f5
    shr ecx, 8
    mov al, cl; 起始扇区的高八位
    out dx, al

    inc dx; 0x1f6
    shr ecx, 8
    and cl, 0b1111; 将高四位置为 0

    mov al, 0b1110_0000;
    or al, cl
    out dx, al; 主盘 - LBA 模式

    inc dx; 0x1f7
    mov al, 0x20; 读硬盘
    out dx, al

    xor ecx, ecx; 将 ecx 清空
    mov cl, bl; 得到读写扇区的数量

    .read:
        push cx; 保存 cx
        call .waits; 等待数据准备完毕
        call .reads; 读取一个扇区
        pop cx; 恢复 cx
        loop .read

    ret

    .waits:
        mov dx, 0x1f7
        .check:
            in al, dx
            jmp $+2; nop 直接跳转到下一行
            jmp $+2; 一点点延迟
            jmp $+2
            and al, 0b1000_1000
            cmp al, 0b0000_1000
            jnz .check
        ret

    .reads:
        mov dx, 0x1f0
        mov cx, 256; 一个扇区 256 字
        .readw:
            in ax, dx
            jmp $+2; 一点点延迟
            jmp $+2
            jmp $+2
            mov [edi], ax
            add edi, 2
            loop .readw
        ret
    

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
    dd 0
ards_buffer:
