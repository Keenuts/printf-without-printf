#include <elf.h>
#include <link.h>

#define NULL (void*)0;

/*
 * An ELF has argv is an array of char*
 * The last entry is NULL
 *
 * Beyond this last entry, there is the ENV array
 * It's a char* array.
 * Last entry is also NULL
 *
 * Behind this env array, there is the AUXV
 * It's an array of struct containing a pointer and a type field
 * This array is used to retreive informations like symbol table, linkmap, etc
 */

// Goes to the end of the Argv array to find the ENV array
static
char** get_env(char **argv)
{
    while (*argv)
        argv++;

    return argv + 1;
}

/* Goes to the end of the Env array to find the first AuxV entry
 * Interresting AuxV entries:
 *   (All are defined into the elf.h file)
 *   AT_PHDR -> program header
 *   AT_ENTRY -> entry-point location 
 *   ...
 */
static
ElfW(auxv_t)* get_auxv(char **argv)
{
    char **env = get_env(argv);

    while (*env)
        env++;
    return (ElfW(auxv_t)*)(env + 1);
}

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

// We use Addr since these fileds are either 32 or 64 bit long
FindTableEntryFunc(auxv, ElfW(auxv_t), ElfW(Addr), a_type, AT_NULL)
FindTableEntryFunc(phdr, ElfW(Phdr), ElfW(Word), p_type, PT_NULL)
// We use Addr again, same problem as the PT_HEADER
FindTableEntryFunc(dyn, ElfW(Dyn), ElfW(Addr), d_tag, DT_NULL)


int main(int argc, char **argv)
{
    char **env = get_env(argv);
    ElfW(auxv_t) *auxv = get_auxv(argv);
    ElfW(auxv_t) *auxv_phdr_entry = find_in_auxv(auxv, AT_PHDR);
    ElfW(Phdr) *phdr_ptr = (ElfW(Phdr)*)auxv_phdr_entry->a_un.a_val;

    ElfW(Phdr) *pt_dynamic = find_in_phdr(phdr_ptr, PT_DYNAMIC);
    ElfW(Phdr) *pt_phdr = find_in_phdr(phdr_ptr, PT_PHDR);

    ElfW(Addr) base_address = (ElfW(Addr))phdr_ptr - pt_phdr->p_vaddr;

#if defined(USE_EXPORTED_SYMBOL)
    ElfW(Dyn) *dyn_table = _DYNAMIC;
#else
    ElfW(Dyn) *dyn_table = (ElfW(Dyn)*)(base_address + pt_dynamic->p_vaddr);
#endif

    return 0;
}
