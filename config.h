#pragma once
/* This header is used to configure the behavior of this program */

// Do we want to cheat, and get the dynamic table using link.h ?
//#define USE_EXPORTED_DYNAMIC_SYMBOLS

// Do we want to use the GOT to get the link-map, or r_debug
#define USE_GOT_FOR_LINK_MAP

// Do we use the dump method ?
//#define CRAWL_THROUGH_SYMTAB

