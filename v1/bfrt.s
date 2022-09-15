	.intel_syntax noprefix
	.globl input
	.globl output
	.globl _start

	.bss
bf_buffer:
	.zero 30000
	.text

_start:
	lea rdx, [rip+bf_buffer]
	xor eax,eax
	mov rcx, 29999
	xor ebx,ebx
	call main
	mov eax, 60
	xor edi, edi
	syscall

output:
	push rax
	push rdx
	# this must be first because rax and rdx are overwritten later
	lea rsi, [rax+rdx]  # address of data
	mov eax, 1  # write()
	mov edi, 1  # fileno = stdout
	mov edx, 1  # number of bytes to write
	syscall
	# ignore errors :)
	pop rdx
	pop rax
	ret

input:
	push rax
	push rdx
	lea rsi, [rax+rdx]  # address to write to
	mov eax, 0  # read()
	mov edi, 0  # fileno (stdin)
	mov edx, 1  # number of bytes
	syscall
	# should probably check if anything was read... meh
	pop rdx
	pop rax
	ret
