#pragma once

int basic_strcmp(const char *a, const char *b);
ElfW(Addr) sanitize(ElfW(Addr) ptr, ElfW(Addr) base_addr);
