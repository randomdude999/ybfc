; SPDX-License-Identifier: ISC
; vim: syntax=nasm
tape_addr     equ 0x80000000
export_tape_addr equ tape_addr
;tape_size     equ 0x00100000

[bits 32]
[map symbols header_sym.map]
export_code_start equ 0x01000000
org export_code_start

export_sub_input equ $ - $$
    xor eax,eax
    mov al, 3
    mov bl, 0
    int 0x80
    ret
export_sub_output equ $ - $$
    xor eax,eax
    mov al, 4
    mov bl, 1
    int 0x80
    ret

_start:
    xor ebx,ebx
    xor edx,edx
    inc edx
    mov ecx, tape_addr
export_tape_andmask equ $ - $$ + 1
    mov edi, 0x69696969
    mov esi, ecx
