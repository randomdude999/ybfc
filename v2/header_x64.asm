tape_addr equ 0x200000000000
export_tape_addr equ tape_addr
dummy equ 0x1337deadbeef1337

; assumes size is a power of 2, and that the size'th bit in addr is 0
; tape_andmask equ tape_addr+tape_size-1

; disabled by default because it's like 300 bytes of cruft,
; but might be useful for debugging
;%define SHDR 1

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
%ifdef SHDR
                dq      shdr - $$                       ;   e_shoff
%else
                dq      0                               ;   e_shoff
%endif
                dd      0                               ;   e_flags
                dw      ehdrsize                        ;   e_ehsize
                dw      phdrsize                        ;   e_phentsize
                dw      2                               ;   e_phnum
%ifdef SHDR
                dw      shdrsize                        ;   e_shentsize
                dw      4                               ;   e_shnum
                dw      1                               ;   e_shstrndx
%else
                dw      0                               ;   e_shentsize
                dw      0                               ;   e_shnum
                dw      0                               ;   e_shstrndx
%endif

  
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
                dq      tape_addr                       ;   p_align

%ifdef SHDR
shdr:                                                   ; Elf64_Shdr
                dd      0                               ;   sh_name
                dd      0 ; SHT_NULL                    ;   sh_type
                dq      0                               ;   sh_flags
                dq      0                               ;   sh_addr
                dq      0                               ;   sh_offset
                dq      0                               ;   sh_size
                dd      0                               ;   sh_link
                dd      0                               ;   sh_info
                dq      0                               ;   sh_addralign
                dq      0                               ;   sh_entsize
shdrsize        equ     $ - shdr

                dd      str_shstrtab                    ;   sh_name
                dd      3 ; SHT_STRTAB                  ;   sh_type
                dq      0                               ;   sh_flags
                dq      0                               ;   sh_addr
                dq      strtab - $$                     ;   sh_offset
                dq      strtab_size                     ;   sh_size
                dd      0                               ;   sh_link
                dd      0                               ;   sh_info
                dq      0                               ;   sh_addralign
                dq      0                               ;   sh_entsize

                dd      str_bss                         ;   sh_name
                dd      8 ; SHT_NOBITS                  ;   sh_type
                dq      3 ; ALLOC + WRITE               ;   sh_flags
                dq      tape_addr                       ;   sh_addr
                dq      0                               ;   sh_offset
export_tape_size_2 equ $ - $$
                dq      dummy                           ;   sh_size
                dd      0                               ;   sh_link
                dd      0                               ;   sh_info
                dq      tape_addr                       ;   sh_addralign
                dq      0                               ;   sh_entsize

                dd      str_text                        ;   sh_name
                dd      1 ; SHT_PROGBITS                ;   sh_type
                dq      6 ; ALLOC + EXECINSTR           ;   sh_flags
                dq      export_sub_input + $$           ;   sh_addr
                dq      export_sub_input                ;   sh_offset
export_file_size_2 equ $ - $$
                dq      dummy                           ;   sh_size
                dd      0                               ;   sh_link
                dd      0                               ;   sh_info
                dq      0                               ;   sh_addralign
                dq      0                               ;   sh_entsize

strtab:
                db      0
str_text        equ     $ - strtab
                db      ".text", 0
str_shstrtab    equ     $ - strtab
                db      ".shstrtab", 0
str_bss         equ     $ - strtab
                db      ".bss", 0
strtab_size     equ     $ - strtab

%else
export_file_size_2 equ 0
export_tape_size_2 equ 0
%endif

export_sub_input equ $ - $$
input:
        xor eax, eax
io_common:
        mov edi, eax
        push rcx
        syscall
        pop rcx
        ret
export_sub_output equ $ - $$
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
