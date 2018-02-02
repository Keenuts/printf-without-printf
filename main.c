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

// Goes to the end of the Env array to find the first AuxV entry
static
Elf64_auxv* get_auxv(char **argv)
{
   char **env = get_env(argv);

   while (*env)
      env++;
   return (void*)(env + 1);
}


/* AuxV entries:
 *   (All are defined into the elf.h file)
 *   AT_PHDR -> program header
 *   AT_ENTRY -> entry-point location 
 *   ...
 */
int main(int argc, char argv**)
{
   char **env = get_env(argv);
   char **auxv = get_auxv(argv);

   void *phdr_ptr = read_auxv(auxv, AT_PHDR);
   return 0;
}
