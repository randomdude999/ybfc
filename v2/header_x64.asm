tape_addr equ 0x200000000000
export_tape_addr equ tape_addr
dummy equ 0x1337deadbeef1337

; assumes size is a power of 2, and that the size'th bit in addr is 0
; tape_andmask equ tape_addr+tape_size-1

[bits 64]
[map symbols header_sym.map]
org 0x01000000

  ehdr:                                                 ; Elf32_Ehdr
                db      0x7F, "ELF", 2, 1, 1, 1         ;   e_ident
                dq      0
                dw      2                               ;   e_type
                dw      62                              ;   e_machine
                dd      1                               ;   e_version
                dq      _start                          ;   e_entry
                dq      phdr - $$                       ;   e_phoff
                dq      0                               ;   e_shoff
                dd      0                               ;   e_flags
                dw      ehdrsize                        ;   e_ehsize
                dw      phdrsize                        ;   e_phentsize
                dw      2                               ;   e_phnum
                dw      0                               ;   e_shentsize
                dw      0                               ;   e_shnum
                dw      0                               ;   e_shstrndx
  
  ehdrsize      equ     $ - ehdr
  
  phdr:                                                 ; Elf64_Phdr
                dd      1                               ;   p_type
                dd      5                               ;   p_flags
                dq      0                               ;   p_offset
                dq      $$                              ;   p_vaddr
                dq      $$                              ;   p_paddr
export_file_size_loc   equ $ - $$
                dq      dummy                           ;   p_filesz
                dq      dummy                           ;   p_memsz
                dq      1                               ;   p_align
  phdrsize      equ     $ - phdr
                                                        ; 2nd header entry
                dd      1                               ;   p_type
                dd      6                               ;   p_flags
                dq      0                               ;   p_offset
                dq      tape_addr                       ;   p_vaddr
                dq      tape_addr                       ;   p_paddr
                dq      0                               ;   p_filesz
export_tape_size equ $ - $$
                dq      dummy                           ;   p_memsz
                dq      1                               ;   p_align


export_sub_input equ $ - $$
input:
        xor eax, eax
io_common:
        mov edi, eax
        syscall
        ret
export_sub_output equ $ - $$
output:
        xor eax, eax
        inc eax
        jmp io_common

_start:
        xor edx,edx
        inc edx
        mov rsi, tape_addr
        mov rcx, rsi
export_tape_andmask equ $ - $$ + 2
        mov r9, dummy
        mov r10, rcx
        lea rbx, [rel input]
        lea rbp, [rel output]
