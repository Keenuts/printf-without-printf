#pragma once

#include <elf.h>
#include <link.h>

#define NULL (void*)0

typedef struct
{
    ElfW(Word) *hashtable;
    ElfW(Sym) *symtab;
    char *strtab;

} elf_info_t;

elf_info_t get_info_from_dyntable(ElfW(Dyn) *dyntab, ElfW(Addr) base_addr);
elf_info_t get_elf_info_from_phdr(ElfW(Phdr) *phdr_ptr);
elf_info_t get_elf_info_from_auxv(ElfW(auxv_t) *auxv);
void *find_symbol_address(const char* symbol, ElfW(Dyn) *dyntab, ElfW(Addr) offset);
