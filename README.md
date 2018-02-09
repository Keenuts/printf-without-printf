# Printf-without-printf

This C code will call printf without directly calling it.

 - get link-map
 - for each lib:
>    - get dyn-tab
>    - get hashtable, symtable, strtab
>    - search for the symbol
>    - return correct address
 - call this address
