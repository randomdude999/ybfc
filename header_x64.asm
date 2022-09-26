; vim: syntax=nasm
tape_addr equ 0x200000000000
export_tape_addr equ tape_addr
dummy equ 0x1337deadbeef1337

; assumes size is a power of 2, and that the size'th bit in addr is 0
; tape_andmask equ tape_addr+tape_size-1

[bits 64]
[map symbols header_sym.map]
export_code_start equ 0x01000000
org export_code_start

input:
        xor eax, eax
io_common:
        mov edi, eax
        push rcx
        syscall
        pop rcx
        ret
output:
        xor eax, eax
        inc eax
        jmp io_common

; the 6 bytes after the call to this are interpreted as a signed offset to jump to
trampoline:
        cmp dh, [rsi]
        jne ret_to_start
trampoline_always:
        pop rax
        mov rdi, [rax-2]
        sar rdi, 16
        add rdi, rax
        jmp rdi
ret_to_start:
        pop rax
        add rax, 6
        jmp rax

_start:
        xor edx,edx
        inc edx
        mov rsi, tape_addr
        mov r10, rsi
export_tape_andmask equ $ - $$ + 2
        mov r9, dummy
        lea rbx, [rel input]
        lea rbp, [rbx + (output-input)]
        lea rcx, [rbx + (trampoline-input)]
        lea r8, [rbx + (trampoline_always-input)]
