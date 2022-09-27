// SPDX-License-Identifier: ISC
#include "bfc_common.h"
#include <stddef.h>

//#define ELF_BITS 64


#if ELF_BITS == 64
#  define ELFN_T uint64_t
#  define LEI_N le64i
#  define LEB_N LE64
#elif ELF_BITS == 32
#  define ELFN_T uint32_t
#  define LEI_N le32i
#  define LEB_N LE32
#else
#  error "please set ELF_BITS"
#endif

struct elf_ehdr {
	uint8_t	e_ident[16];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	ELFN_T e_entry;
	ELFN_T e_phoff;
	ELFN_T e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};
struct elf_phdr {
	uint32_t p_type;
#if ELF_BITS == 64
	uint32_t p_flags;
#endif
	ELFN_T p_offset;
	ELFN_T p_vaddr;
	ELFN_T p_paddr;
	ELFN_T p_filesz;
	ELFN_T p_memsz;
#if ELF_BITS == 32
	uint32_t p_flags;
#endif
	ELFN_T p_align;
};
struct elf_shdr {
	uint32_t sh_name;
	uint32_t sh_type;
	ELFN_T sh_flags;
	ELFN_T sh_addr;
	ELFN_T sh_offset;
	ELFN_T sh_size;
	uint32_t sh_link;
	uint32_t shinfo;
	ELFN_T sh_addralign;
	ELFN_T sh_entsize;
};

struct elf_size_write_locs {
	ELFN_T first;
	ELFN_T second;
	ELFN_T third;
	ELFN_T text_start;
};

// this function is so ugly but i couldn't figure out a better way to do this
// in plain C
// return value is a pointer to pass into finalize_elf_header
static void* write_elf_hdr(ELFN_T code_loc, ELFN_T tape_loc,
		ELFN_T tape_size, bool write_shdr, ELFN_T entry_off, uint16_t emachine) {
	struct elf_phdr phdrs[2] = { {
			.p_type = le32i(1), // PT_LOAD
			.p_flags = le32i(5), // read+exec
			.p_vaddr = LEI_N(code_loc),
			.p_paddr = LEI_N(code_loc),
			.p_align = LEI_N(1)
			// filesz and memsz fixed later
		}, {
			.p_type = le32i(1), // PT_LOAD
			.p_flags = le32i(6), // read+write
			.p_vaddr = LEI_N(tape_loc),
			.p_paddr = LEI_N(tape_loc),
			.p_memsz = LEI_N(tape_size),
			.p_align = LEI_N(1)
	} };


	// TODO: hardcode less
	uint8_t shstrtab[] = "\0.shstrtab\0.text\0.bss";
	int strtab_shstrtab = 1;
	int strtab_text = sizeof("\0.shstrtab");
	int strtab_bss = sizeof("\0.shstrtab\0.text");

	struct elf_shdr shdrs[4] = {
		{ 0 }, // dummy first entry, required by spec
		{
			.sh_name = strtab_shstrtab,
			.sh_type = le32i(3), // SHT_STRTAB
			.sh_size = LEI_N(sizeof(shstrtab)),
			.sh_offset = sizeof(struct elf_ehdr) + sizeof(phdrs) + sizeof(shdrs),
		}, {
			.sh_name = strtab_text,
			.sh_type = le32i(1), // SHT_PROGBITS
			.sh_flags = LEI_N(6), // ALLOC + EXECINSTR
			.sh_offset = LEI_N(sizeof(struct elf_ehdr) + sizeof(phdrs) + sizeof(shdrs) + sizeof(shstrtab)),
			.sh_addr = LEI_N(sizeof(struct elf_ehdr) + sizeof(phdrs) + sizeof(shdrs) + sizeof(shstrtab) + code_loc),
			// sh_size filled in later
		}, {
			.sh_name = strtab_bss,
			.sh_type = le32i(8), // SHT_NOBITS
			.sh_flags = LEI_N(3), // ALLOC + WRITE
			.sh_addr = LEI_N(tape_loc),
			.sh_size = LEI_N(tape_size),
		}
	};
	struct elf_ehdr ehdr = {
		.e_ident = { 0x7f, 'E', 'L', 'F', (ELF_BITS == 64 ? 2 : 1), 1, 1, 1 },
		.e_type = le16i(2), // ET_EXEC
		.e_machine = le16i(emachine),
		.e_version = le32i(1),
		// e_entry filled later
		.e_phoff = LEI_N(sizeof(ehdr)),
		.e_ehsize = LEI_N(sizeof(ehdr)),
		.e_phentsize = le16i(sizeof(struct elf_phdr)),
		.e_phnum = le16i(sizeof(phdrs) / sizeof(struct elf_phdr)),
		.e_shstrndx = le16i(1),
	};

	struct elf_size_write_locs* res_ptr = malloc(sizeof(struct elf_size_write_locs));
	res_ptr->first = sizeof(ehdr) + offsetof(struct elf_phdr, p_filesz);
	res_ptr->second = sizeof(ehdr) + offsetof(struct elf_phdr, p_memsz);
	res_ptr->third = 0;

	if(write_shdr) {
		ehdr.e_shoff = LEI_N(sizeof(ehdr)+sizeof(phdrs));
		ehdr.e_shentsize = le16i(sizeof(struct elf_shdr));
		ehdr.e_shnum = le16i(sizeof(shdrs) / sizeof(struct elf_shdr));
		ehdr.e_entry = LEI_N(code_loc + sizeof(ehdr) + sizeof(phdrs)
				+ sizeof(shdrs) + sizeof(shstrtab) + entry_off);
		res_ptr->third = sizeof(ehdr) + sizeof(phdrs) + sizeof(struct elf_shdr)*2 + offsetof(struct elf_shdr, sh_size);
	}
	else ehdr.e_entry = LEI_N(code_loc + sizeof(ehdr) + sizeof(phdrs) + entry_off);
	writebuf2((uint8_t*)&ehdr, sizeof(ehdr));
	writebuf2((uint8_t*)phdrs, sizeof(phdrs));
	if(write_shdr) {
		writebuf2((uint8_t*)shdrs, sizeof(shdrs));
		writebuf(shstrtab);
	}
	res_ptr->text_start = out_off;
	return res_ptr;
}

static void finalize_elf_header(void* write_offs_in) {
	struct elf_size_write_locs* offs = write_offs_in;
	size_t old_out_pos = out_off;
	out_off = offs->first;
	fseek(output, out_off, SEEK_SET);
	writebuf(B(LEB_N(old_out_pos)));
	out_off = offs->second;
	fseek(output, out_off, SEEK_SET);
	writebuf(B(LEB_N(old_out_pos)));
	if(offs->third) {
		out_off = offs->third;
		fseek(output, out_off, SEEK_SET);
		writebuf(B(LEB_N(old_out_pos - offs->text_start)));
	}
	out_off = old_out_pos;
	fseek(output, out_off, SEEK_SET);
}
