#include <elf.h>
#include <link.h>

int basic_strcmp(const char *a, const char *b)
{
    for (; *a != 0 && *b != 0; a++, b++)
        if (*a != *b)
            return 1;
    return *a != *b;
}

ElfW(Addr) sanitize(ElfW(Addr) ptr, ElfW(Addr) base_addr)
{
    if (ptr < base_addr)
        return ptr + base_addr;
    return ptr;
}
