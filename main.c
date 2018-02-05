#include <assert.h>

#include "config.h"
#include "elf-utils.h"

extern void* _GLOBAL_OFFSET_TABLE_[];

// Goes to the end of the Argv array to find the ENV array
static char** get_env(char **argv)
{
    while (*argv)
        argv++;
    return argv + 1;
}

// Goes to the end of the Env array to find the first AuxV entry
static ElfW(auxv_t)* get_auxv(char **argv)
{
    char **env = get_env(argv);
    while (*env)
        env++;
    return (ElfW(auxv_t)*)(env + 1);
}

typedef int (*target_function)(const char*, ...);

int main(int argc, char **argv)
{
    char **env = get_env(argv);
    ElfW(auxv_t) *auxv = get_auxv(argv);

    elf_info_t info = get_elf_info_from_auxv(auxv);

#if defined(USE_GOT_FOR_LINK_MAP)
    void *got = _GLOBAL_OFFSET_TABLE_;
    struct link_map *lm = (struct link_map*)*((ElfW(Addr)*)got + 1);
#else
    struct link_map *lm = _r_debug.r_map;
#endif
    lm = lm->l_next; //Skipping our own file

    for ( ; lm; lm = lm->l_next) {

        void *addr = find_symbol_address("printf", lm->l_ld, lm->l_addr);

        if (addr) {
            target_function tf = (target_function)(addr);
            tf("printf@%p\n", (void*)tf);
            return 0;
        }
    }

    
    assert(0 && "Symbol not found.");
    return 0;
}
