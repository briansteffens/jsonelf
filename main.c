#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include <json-c/json.h>

#include "elf_strings.h"


#define ERR_USAGE         1
#define ERR_OPEN_FILE     2
#define ERR_STAT_FILE     3
#define ERR_READ_FILE     4
#define ERR_32_BIT        5
#define ERR_OUTPUT_BUFFER 100
#define ERR_UNINDENT      101
#define ERR_BAD_ELF_VALUE 102


char* file_data;
Elf64_Ehdr* header;

char* strtab = NULL;
char* shstrtab = NULL;


Elf64_Shdr* section_header(int index)
{
    return (Elf64_Shdr*)(file_data + header->e_shoff +
            index * header->e_shentsize);
}


const char* str_p_flags(const long long f)
{
    switch (f)
    {
        case 1:
            return "PF_X";
        case 2:
            return "PF_W";
        case 3:
            return "PF_X | PF_W";
        case 4:
            return "PF_R";
        case 5:
            return "PF_X | PF_R";
        case 6:
            return "PF_W | PF_R";
        case 7:
            return "PF_X | PF_W | PF_R";
        case 0x0ff00000:
            return "PF_MASKOS";
        case 0xf0000000:
            return "PF_MASKPROC";
        default:
            printf("Unrecognized p_flags: %lld\n", f);
            exit(ERR_BAD_ELF_VALUE);
    }
}


#define next_flag(f, str) \
{ \
    if (f & str) \
    { \
        if (ret_next != ret) \
        { \
            ret_next += sprintf(ret_next, " | "); \
        } \
        ret_next += sprintf(ret_next, #str); \
    } \
}


const char* str_sh_flags(const long long f)
{
    static char ret[256];
    char* ret_next = ret;

    next_flag(f, SHF_WRITE)
    next_flag(f, SHF_ALLOC)
    next_flag(f, SHF_EXECINSTR)
    next_flag(f, SHF_MERGE)
    next_flag(f, SHF_STRINGS)
    next_flag(f, SHF_INFO_LINK)
    next_flag(f, SHF_LINK_ORDER)
    next_flag(f, SHF_OS_NONCONFORMING)
    next_flag(f, SHF_GROUP)
    next_flag(f, SHF_TLS)
    next_flag(f, SHF_COMPRESSED)
    next_flag(f, SHF_MASKOS)
    next_flag(f, SHF_MASKPROC)
    next_flag(f, SHF_ORDERED)
    next_flag(f, SHF_EXCLUDE)

    ret_next = NULL;
    return ret;
}


const char* str_st_info(const long long v)
{
    static char ret[256];
    char* ret_next = ret;

    const char* type_str = str_st_type(ELF64_ST_TYPE(v));
    strcpy(ret_next, type_str);
    ret_next += strlen(type_str);

    *ret_next++ = ' ';
    *ret_next++ = '|';
    *ret_next++ = ' ';

    const char* binding_str = str_st_binding(ELF64_ST_BIND(v));
    strcpy(ret_next, binding_str);
    ret_next += strlen(binding_str);

    *ret_next = 0;
    return ret;
}


const char* lookup_strtab(const long long v)
{
    if (strtab == NULL)
    {
        return "Not found";
    }

    return strtab + v;
}


const char* lookup_shstrtab(const long long v)
{
    if (shstrtab == NULL)
    {
        return "Not found";
    }

    return shstrtab + v;
}


const char* str_st_shndx(const long long v)
{
    switch (v)
    {
        case 0:
            return "UND";
        case 65521:
            return "ABS";
        default:
            return lookup_shstrtab(v);
    }
}


void add_key(json_object* out, const char* key, long long value,
             const char* (*converter)(long long l))
{
    if (converter)
    {
        json_object_object_add(out, key,
                json_object_new_string(converter(value)));
        return;
    }

    json_object_object_add(out, key, json_object_new_int(value));
}


int main(int argc, char* argv[])
{
    // Parse command line arguments -------------------------------------------

    char* filename = NULL;
    bool arg_fail = false;

    while (--argc)
    {
        if (filename != NULL)
        {
            arg_fail = true;
            break;
        }

        filename = argv[argc];
    }

    if (filename == NULL)
    {
        arg_fail = true;
    }

    if (arg_fail)
    {
        printf("Usage: jsonelf somefile.o\n");
        return ERR_USAGE;
    }

    // Load file into memory --------------------------------------------------

    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Failed to open file\n");
        return ERR_OPEN_FILE;
    }

    struct stat file_stat;

    if (fstat(fileno(file), &file_stat) < 0)
    {
        printf("Failed to stat file\n");
        return ERR_STAT_FILE;
    }

    file_data = malloc(file_stat.st_size * sizeof(char));

    if (!fread(file_data, file_stat.st_size, 1, file))
    {
        printf("Failed to read file contents\n");
        return ERR_READ_FILE;
    }

    fclose(file);

    // Validate file ----------------------------------------------------------

    header = (Elf64_Ehdr*)file_data;

    if (header->e_ident[EI_CLASS] != ELFCLASS64)
    {
        printf("This program only supports 64-bit files\n");
        return ERR_32_BIT;
    }

    // Lookup shstrtab
    shstrtab = file_data + section_header(header->e_shstrndx)->sh_offset;

    // Find strtab
    for (int i = 0; i < header->e_shnum; i++)
    {
        Elf64_Shdr* sh = section_header(i);

        if (sh->sh_type == SHT_STRTAB &&
            strcmp(lookup_shstrtab(sh->sh_name), ".strtab") == 0)
        {
            strtab = file_data + sh->sh_offset;
            break;
        }
    }

    // Generate output --------------------------------------------------------

    json_object* obj = json_object_new_object();

    // e_ident ----------------------------------------------------------------

    json_object* ident = json_object_new_object();

    add_key(ident, "ei_class", header->e_ident[EI_CLASS], str_ei_class);
    add_key(ident, "ei_data", header->e_ident[EI_DATA], str_ei_data);
    add_key(ident, "ei_version", header->e_ident[EI_VERSION], str_ei_version);
    add_key(ident, "ei_osabi", header->e_ident[EI_OSABI], str_ei_osabi);
    add_key(ident, "ei_abiversion", header->e_ident[EI_ABIVERSION], NULL);

    json_object_object_add(obj, "e_ident", ident);

    // header -----------------------------------------------------------------

    add_key(obj, "e_type", header->e_type, str_e_type);
    add_key(obj, "e_machine", header->e_machine, str_e_machine);
    add_key(obj, "e_version", header->e_version, str_e_version);
    add_key(obj, "e_entry", header->e_entry, NULL);
    add_key(obj, "e_phoff", header->e_phoff, NULL);
    add_key(obj, "e_shoff", header->e_shoff, NULL);
    add_key(obj, "e_flags", header->e_flags, NULL);
    add_key(obj, "e_ehsize", header->e_ehsize, NULL);
    add_key(obj, "e_phentsize", header->e_phentsize, NULL);
    add_key(obj, "e_phnum", header->e_phnum, NULL);
    add_key(obj, "e_shentsize", header->e_shentsize, NULL);
    add_key(obj, "e_shnum", header->e_shnum, NULL);
    add_key(obj, "e_shstrndx", header->e_shstrndx, NULL);

    // program headers --------------------------------------------------------

    json_object* program_headers = json_object_new_array();

    for (int i = 0; i < header->e_phnum; i++)
    {
        Elf64_Phdr* ph = (Elf64_Phdr*)(file_data + header->e_phoff +
                i * header->e_phentsize);

        json_object* out = json_object_new_object();

        add_key(out, "p_type", ph->p_type, str_p_type);
        add_key(out, "p_flags", ph->p_flags, str_p_flags);
        add_key(out, "p_offset", ph->p_offset, NULL);
        add_key(out, "p_vaddr", ph->p_vaddr, NULL);
        add_key(out, "p_paddr", ph->p_paddr, NULL);
        add_key(out, "p_filesz", ph->p_filesz, NULL);
        add_key(out, "p_memsz", ph->p_memsz, NULL);
        add_key(out, "p_align", ph->p_align, NULL);

        json_object_array_add(program_headers, out);
    }

    json_object_object_add(obj, "program_headers", program_headers);

    // section headers --------------------------------------------------------

    json_object* section_headers = json_object_new_array();

    for (int i = 0; i < header->e_shnum; i++)
    {
        Elf64_Shdr* sh = section_header(i);

        json_object* out = json_object_new_object();

        add_key(out, "sh_name", sh->sh_name, lookup_shstrtab);
        add_key(out, "sh_type", sh->sh_type, str_sh_type);
        add_key(out, "sh_flags", sh->sh_flags, str_sh_flags);
        add_key(out, "sh_addr", sh->sh_addr, NULL);
        add_key(out, "sh_offset", sh->sh_offset, NULL);
        add_key(out, "sh_size", sh->sh_size, NULL);
        add_key(out, "sh_link", sh->sh_link, NULL);
        add_key(out, "sh_info", sh->sh_info, NULL); // TODO
        add_key(out, "sh_addralign", sh->sh_addralign, NULL);
        add_key(out, "sh_entsize", sh->sh_entsize, NULL);

        // strings ------------------------------------------------------------

        if (sh->sh_type == SHT_STRTAB)
        {
            json_object* strings = json_object_new_array();

            char* string = file_data + sh->sh_offset;
            char* end = file_data + sh->sh_offset + sh->sh_size;

            while (string <= end)
            {
                json_object_array_add(strings, json_object_new_string(string));

                char* runner = string;

                while (*runner)
                {
                    runner++;
                }

                string = runner + 1;
            }

            json_object_object_add(out, "strings", strings);
        }

        // symbols ------------------------------------------------------------

        else if (sh->sh_type == SHT_SYMTAB)
        {
            json_object* symbols = json_object_new_array();

            void* ptr = file_data + sh->sh_offset;
            void* end = ptr + sh->sh_size;

            while (ptr < end)
            {
                Elf64_Sym* sym = ptr;

                json_object* symobj = json_object_new_object();

                add_key(symobj, "st_name", sym->st_name, lookup_strtab);
                add_key(symobj, "st_info", sym->st_info, str_st_info);
                add_key(symobj, "st_other", sym->st_other, str_st_other);
                add_key(symobj, "st_shndx", sym->st_shndx, str_st_shndx);
                add_key(symobj, "st_value", sym->st_value, NULL);
                add_key(symobj, "st_size", sym->st_size, NULL);

                json_object_array_add(symbols, symobj);

                ptr += sh->sh_entsize;
            }

            json_object_object_add(out, "symbols", symbols);
        }

        // relocations --------------------------------------------------------

        else if (sh->sh_type == SHT_RELA || sh->sh_type == SHT_REL)
        {
            json_object* relocations = json_object_new_array();

            void* ptr = file_data + sh->sh_offset;
            void* end = ptr + sh->sh_size;

            while (ptr < end)
            {
                Elf64_Rela* rela = ptr;

                json_object* out = json_object_new_object();

                add_key(out, "r_offset", rela->r_offset, NULL);
                add_key(out, "r_info", rela->r_info, NULL);

                if (sh->sh_type == SHT_RELA)
                {
                    add_key(out, "r_addend", rela->r_addend, NULL);
                }

                add_key(out, "__sym_table", rela->r_info >> 32, NULL);

                int relo_type = rela->r_info & 0xffffffff;

                switch (header->e_machine)
                {
                    case EM_X86_64:
                        add_key(out, "__relo_type", relo_type,
                                str_relo_type_x86_64);
                        break;
                    default:
                        printf("Unrecognized e_machine: %d",
                                header->e_machine);
                        exit(ERR_BAD_ELF_VALUE);
                }

                json_object_array_add(relocations, out);

                ptr += sh->sh_entsize;
            }

            json_object_object_add(out, "relocations", relocations);
        }

        json_object_array_add(section_headers, out);
    }

    json_object_object_add(obj, "section_headers", section_headers);

    printf("%s\n", json_object_to_json_string_ext(obj,
            JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED));

    return 0;
}
