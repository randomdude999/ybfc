tape_addr     equ 0x40000000
header_tape_addr equ tape_addr
;tape_size     equ 0x00100000

; assumes size is a power of 2, and that the size'th bit in addr is 0
; tape_andmask equ tape_addr+tape_size-1

[bits 32]
[map symbols header_sym.map]
org 0x01000000

  ehdr:                                                 ; Elf32_Ehdr
                db      0x7F, "ELF", 1, 1, 1, 0         ;   e_ident
        times 8 db      0
                dw      2                               ;   e_type
                dw      3                               ;   e_machine
                dd      1                               ;   e_version
                dd      _start                          ;   e_entry
                dd      phdr - $$                       ;   e_phoff
                dd      0                               ;   e_shoff
                dd      0                               ;   e_flags
                dw      ehdrsize                        ;   e_ehsize
                dw      phdrsize                        ;   e_phentsize
                dw      2                               ;   e_phnum
                dw      0                               ;   e_shentsize
                dw      0                               ;   e_shnum
                dw      0                               ;   e_shstrndx
  
  ehdrsize      equ     $ - ehdr
  
  phdr:                                                 ; Elf32_Phdr
                dd      1                               ;   p_type
                dd      0                               ;   p_offset
                dd      $$                              ;   p_vaddr
                dd      $$                              ;   p_paddr
header_file_size_loc   equ $ - $$
                dd      0x69696969                      ;   p_filesz
                dd      0x69696969                      ;   p_memsz
                dd      5                               ;   p_flags
                dd      1                               ;   p_align
  phdrsize      equ     $ - phdr
                                                        ; 2nd header entry
                dd      1                               ;   p_type
                dd      0                               ;   p_offset
                dd      tape_addr                       ;   p_vaddr
                dd      0                               ;   p_paddr
                dd      0                               ;   p_filesz
header_tape_size equ $ - $$
                dd      0x69696969                       ;   p_memsz
                dd      6                               ;   p_flags
                dd      1                               ;   p_align
  

header_sub_input equ $ - $$
    xor eax,eax
    mov al, 3
    mov bl, 0
    int 0x80
    ret
header_sub_output equ $ - $$
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
header_tape_andmask equ $ - $$ + 1
    mov edi, 0x69696969
    mov esi, ecx
