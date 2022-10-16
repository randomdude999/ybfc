; vim: filetype=nasm
[map symbols header_sym.map]
[bits 32]
org 0x00400000
file_start:
; MZ header:
db "MZ" ; signature
dw 0x80 ; last page size (1 page = 512B)
dw 1 ; number of pages
dw 0 ; relocation items
dw 4 ; header size, multiplied by 16
dw 0x10 ; minumum allocation
dw 0xffff ; requested allocation
dw 0 ; initial SS
dw 0x0140 ; initial SP
dw 0 ; checksum
dw 0 ; initial IP (???)
dw 0 ; initial CS
dw 0x40 ; offset to relocation table
dw 0 ; overlay - unused
dq 0 ; reserved by PE
dd 0 ; OEM info - unused
times 20 db 0 ; more reserved space
dd pe_header - $$ ; PE header start

dos_stub_start:
[bits 16]
push cs
pop ds
mov dx, dos_msg - dos_stub_start
mov ah, 9
int 0x21
mov ax, 0x4c01
int 0x21
dos_msg:
db "this pwogwam does not suppowt DOS >_<",13,10,"$"
times file_start+0x80-$ db 0

[bits 32]

pe_header:
db "PE",0,0 ; mMagic
; PE file header:
dw 0x14c ; mMachine - i386
dw 2 ; mNumberOfSections
dd 0 ; timestamp
dd 0 ; pointer to symbol table
dd 0 ; number of symbols
dw pe_hdr_end-pe_optional_header ; size of optional header
dw 0x0103 ; characteristics: 32-bit, everything stripped
pe_optional_header:
dw 0x010b ; magic: PE32
db 0x01, 0x49 ; linker version
export_code_size_phys_1 equ $ - $$
dd section_code_end-section_code ; Size of code
dd 0 ; Size of initialized data
dd 0 ; size of bss
dd 0x2000 + entry_point - section_code ; address of entry point
dd 0x2000 ; base of code
dd 0x1000 ; base of data
dd $$ ; ImageBase
dd 0x1000 ; section alignment
dd 0x200 ; file alignment (TODO: try to reduce)
dw 1, 0 ; operating system version
dw 0, 0 ; image version
dw 3, 10 ; subsystem version
dd 0 ; win32 version
export_size_of_image equ $ - $$
dd 0x4000 ; size of image
dd 0x200 ; size of headers
dd 0 ; checksum
dw 3 ; subsystem (win32 console)
dw 0 ; dll characteristics
dd 0x1000 ; size of stack reserve
dd 0x1000 ; size of stack commit
dd 0x10000 ; size of heap reserve
dd 0 ; size of heap commit
dd 0 ; loader flags
dd 16 ; number of RVAs and sizes
; data directories:
dd 0,0 ; export table
dd 0x1000, section_import.imp_dir_end-section_import ; import table
times 14 dd 0,0 ; rest of the entries (unused)
pe_hdr_end:
; section headers:
db ".idata",0,0
dd section_import_end-section_import ; size
dd 0x1000 ; vaddr
dd 0x200 ; filesize
dd section_import-$$
dd 0,0
dw 0,0
dd 0x40000040 ; flags - read, initialized data
db ".code",0,0,0 ; name
export_code_size_virt equ $ - $$
dd section_code_end-section_code ; virtual size
dd 0x2000 ; virtual address
export_code_size_phys_2 equ $ - $$
dd 0x200 ; raw data size
dd section_code-$$ ; raw data address
dd 0,0 ; relocs/linenos
dw 0,0 ; number of ^
dd 0x60000020 ; flags - read+exec, contains code

times $$+0x200-$ db 0
section_import:
import_base equ section_import - 0x1000
; import section
; import directory table
dd .import_lookup_table - import_base ; import lookup table RVA
dd 0 ; timestamp
dd 0 ; forwarder chain (?)
dd .dllname - import_base ; dll name RVA
dd .thunk_table - import_base ; thunk table RVA
dd 0,0,0,0,0 ; null entry - end of directory table
.imp_dir_end:
.dllname:
db "kernel32.dll"
dd 0 ; padding?
; lookup table for kernel32:
.import_lookup_table:
dd .name_writefile - import_base ; import by name, name at 0x3058
dd .name_readfile - import_base
dd .name_getstdhandle - import_base
dd .name_suef - import_base
dd .name_valloc - import_base
dd 0 ; null - end of table
; thunk table for kernel32:
.thunk_table:
WriteFile equ $ - import_base + $$
dd .name_writefile - import_base
ReadFile equ $ - import_base + $$
dd .name_readfile - import_base
GetStdHandle equ $ - import_base + $$
dd .name_getstdhandle - import_base
SUEF equ $ - import_base + $$
dd .name_suef - import_base
VirtualAlloc equ $ - import_base + $$
dd .name_valloc - import_base
dd 0
; hint/name table:
.name_writefile:
dw 0
db "WriteFile",0
.name_readfile:
dw 0
db "ReadFile",0,0
.name_getstdhandle:
dw 0
db "GetStdHandle",0,0
.name_suef:
dw 0
db "SetUnhandledExceptionFilter",0
.name_valloc:
dw 0
db "VirtualAlloc",0
section_import_end:

times $$+0x400-$ db 0
section_code:
export_code_section_start equ $ - $$

input:
export_input_loc equ $ - $$

push ecx ; save regs
push edx ; save regs
mov ecx, esi
mov edx, ReadFile
call io_common
pop edx
pop ecx
cmp [eax], byte `\r`
je input
ret

output:
export_output_loc equ $ - $$
push ecx
push edx
mov ecx, edi
mov edx, WriteFile
call io_common
pop edx
pop ecx
ret

io_common:
push eax ; save regs
push 0 ; dummy value for the read/written pointer
push 0 ; pOverlapped pointer
push esp ; bytes read/written pointer
add dword [esp], 4 ; correct the read/written pointer
push 1 ; number of bytes to read/write
push eax ; buffer address
push ecx ; handle
mov cl, [eax] ; dummy load to make sure the page is initialized
call dword [edx] ; call to ReadFile/WriteFile
pop eax ; restore dummy value
pop eax ; restore regs
ret

except_handler:
; load exception code
mov eax, [esp+4]
mov eax, [eax]
mov edx, [eax]
cmp edx, 0xc0000005
; if not page fault, skip
jne .ret0
mov edx, [eax+24]
and edx, 0xfffff000
; check if address is in the tape buffer
;mov ecx, edx
;sub ecx, [tapeptr]
;cmp ecx, 0x20000000
; if not, skip
;ja .ret0
; allocate that address
push 0x04 ; PAGE_READWRITE
push 0x00001000 ; MEM_COMMIT
push 0x1000
push edx
call dword [VirtualAlloc]
cmp eax, 0
je .ret0
mov eax, -1
ret 4
.ret0:
mov eax, 0
ret 4

entry_point:
push except_handler - section_code + 0x402000
call dword [SUEF]
push -0xa ; stdin handle
call dword [GetStdHandle]
mov esi, eax
push -0xb ; stdout handle
call dword [GetStdHandle]
mov edi, eax
push 0x04 ; PAGE_READWRITE
push 0x00002000 ; MEM_RESERVE
export_tape_size equ $ - $$ + 1
mov ebp, 0x20000000
push ebp
push 0
call dword [VirtualAlloc]
; eax - start of tape
mov ebx, eax
lea ecx, [eax+ebp]
xor edx,edx

;call input
;add byte [eax], 1
;call output
;mov byte [eax], 'A'
;call output

;xor eax,eax
;ret

section_code_end:
