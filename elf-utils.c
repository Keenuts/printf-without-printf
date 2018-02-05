#include <assert.h>

#include "config.h"
#include "elf-utils.h"
#include "utils.h"

#define FindTableEntryFunc(FuncSuffix, PtrType, KeyType, FieldName, NullValue)     \
    static                                                                         \
    PtrType* find_in_ ## FuncSuffix(PtrType *start, KeyType type)                  \
    {                                                                              \
        for (; start->FieldName != NullValue; start++) {                           \
            if (start->FieldName == type)                                          \
                return start;                                                      \
        }                                                                          \
        return NULL;                                                               \
    }

FindTableEntryFunc(phdr, ElfW(Phdr), ElfW(Word), p_type, PT_NULL)
FindTableEntryFunc(auxv, ElfW(auxv_t), ElfW(Addr), a_type, AT_NULL)
FindTableEntryFunc(dyn, ElfW(Dyn), ElfW(Addr), d_tag, DT_NULL)

/* Shameless copy of the hash function from
 * http://www.muppetlabs.com/~breadbox/software/ELF.txt
 */
static
unsigned long elf_hash(const unsigned char *name)
{
  unsigned long       h = 0, g;

  while (*name) {
      h = (h << 4) + *name++;
      if ((g = h & 0xf0000000))
          h ^= g >> 24;
      h &= ~g;
  }
  return h;
}

elf_info_t get_info_from_dyntable(ElfW(Dyn) *dyntab, ElfW(Addr) base_addr)
{
    elf_info_t info;

    ElfW(Dyn) *dt_symtab_ent = find_in_dyn(dyntab, DT_SYMTAB);
    ElfW(Dyn) *dt_strtab_ent = find_in_dyn(dyntab, DT_STRTAB);
    ElfW(Dyn) *dt_hash_ent = find_in_dyn(dyntab, DT_HASH);
    assert(dt_hash_ent && dt_strtab_ent && dt_symtab_ent && "missing entries.");

    info.hashtable = (ElfW(Word)*)sanitize(dt_hash_ent->d_un.d_ptr, base_addr);
    info.symtab = (ElfW(Sym)*)sanitize(dt_symtab_ent->d_un.d_ptr, base_addr);
    info.strtab = (char*)sanitize(dt_strtab_ent->d_un.d_ptr, base_addr);

    return info;
}


elf_info_t get_elf_info_from_phdr(ElfW(Phdr) *phdr_ptr)
{
    ElfW(Phdr) *pt_dynamic = find_in_phdr(phdr_ptr, PT_DYNAMIC);
    ElfW(Phdr) *pt_phdr = find_in_phdr(phdr_ptr, PT_PHDR);
    assert(pt_dynamic && pt_phdr && "No DYNAMIC or PHDR entry on the PHDR");

    ElfW(Addr) base_address = (ElfW(Addr))phdr_ptr - pt_phdr->p_vaddr;

#if defined(USE_EXPORTED_DYNAMIC_SYMBOL)
    // Symbol exported in link.h
    ElfW(Dyn) *dyn_table = _DYNAMIC;
#else
    ElfW(Dyn) *dyn_table = (ElfW(Dyn)*)(base_address + pt_dynamic->p_vaddr);
#endif

    return get_info_from_dyntable(dyn_table, base_address);
}

elf_info_t get_elf_info_from_auxv(ElfW(auxv_t) *auxv)
{
    ElfW(auxv_t) *auxv_phdr_entry = find_in_auxv(auxv, AT_PHDR);
    assert(auxv_phdr_entry && "No PHDR entry found in auxv.");
    ElfW(Phdr) *phdr_ptr = (ElfW(Phdr)*)auxv_phdr_entry->a_un.a_val;

    return get_elf_info_from_phdr(phdr_ptr);
}

void *find_symbol_by_crawling(const char* sym, elf_info_t info, ElfW(Addr) base_addr)
{
    ElfW(Sym) *symtab = info.symtab;
    while ((char*)symtab < info.strtab) {

        if (!basic_strcmp(sym, info.strtab + symtab->st_name))
            return (void*)sanitize(symtab->st_value, base_addr);
        symtab++;
    }

    return NULL;
}

void *find_symbol_by_hash(const char* sym, elf_info_t info, ElfW(Addr) base_addr)
{
    ElfW(Word) nbuckets = info.hashtable[0];
    ElfW(Word) *buckets = info.hashtable + 2;
    ElfW(Word) *chains = info.hashtable + 2 + nbuckets;

    ElfW(Word) sym_hash = (ElfW(Word))elf_hash((const unsigned char*)sym);
    ElfW(Word) x = sym_hash % nbuckets;
    ElfW(Word) y = buckets[x % nbuckets];
    ElfW(Addr) addr = 0;

    while (y) {
        const char* l_sym = info.strtab + info.symtab[y].st_name;

        if (!basic_strcmp(l_sym, (const char*)sym)) {
            addr = info.symtab[y].st_value;
            break;
        }

        y = chains[y];
    }

    return y ? (void*)sanitize(addr, base_addr) : NULL;
}

void *find_symbol_address(const char* symbol, ElfW(Dyn) *dyntab, ElfW(Addr) base_addr)
{
    elf_info_t info = get_info_from_dyntable(dyntab, base_addr);

    #if defined(CRAWL_THROUGH_SYMTAB)
        return find_symbol_by_crawling(symbol, info, base_addr);
    #else
        return find_symbol_by_hash(symbol, info, base_addr);
    #endif
}
