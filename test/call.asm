[bits 32]

extern exit

test:
    push $
    ret
global main
main:
    ; push 5
    ; push eax
    ; pop ebx
    ; pop ecx
    ; pusha
    ; popa
    call test
    call exit