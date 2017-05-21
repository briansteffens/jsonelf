jsonelf
=======

Describe an ELF file in JSON.

This is basically like readelf but with structured output appropriate for
scripting or integration tests.

# Compiling and installing

Compiling jsonelf requires the following dependencies:

- git
- gcc
- linux-headers
- python3
- [json-c](https://github.com/json-c/json-c)

Once you have the dependencies installed:

```bash
git clone https://github.com/briansteffens/jsonelf
cd jsonelf
make
sudo make install
```

Uninstall like this:

```bash
sudo make uninstall
```

# Usage example

To run jsonelf on an ELF file called `test.o`, execute the following:

```bash
jsonelf test.o
```

In this case `test.o` is a minimal hello world object file produced by nasm.
The output should look something like this:

```json
{
   "e_ident": {
     "ei_class": "ELFCLASS64",
     "ei_data": "ELFDATA2LSB",
     "ei_version": "EV_CURRENT",
     "ei_osabi": "ELFOSABI_NONE | ELFOSABI_SYSV",
     "ei_abiversion": 0
   },
   "e_type": "ET_REL",
   "e_machine": "EM_X86_64",
   "e_version": "EV_CURRENT",
   "e_entry": 0,
   "e_phoff": 0,
   "e_shoff": 64,
   "e_flags": 0,
   "e_ehsize": 64,
   "e_phentsize": 0,
   "e_phnum": 0,
   "e_shentsize": 64,
   "e_shnum": 7,
   "e_shstrndx": 3,
   "program_headers": [
   ],
   "section_headers": [
     {
       "sh_name": "",
       "sh_type": "SHT_NULL",
       "sh_flags": "",
       "sh_addr": 0,
       "sh_offset": 0,
       "sh_size": 0,
       "sh_link": 0,
       "sh_info": 0,
       "sh_addralign": 0,
       "sh_entsize": 0
     },
     {
       "sh_name": ".data",
       "sh_type": "SHT_PROGBITS",
       "sh_flags": "SHF_WRITE | SHF_ALLOC",
       "sh_addr": 0,
       "sh_offset": 512,
       "sh_size": 11,
       "sh_link": 0,
       "sh_info": 0,
       "sh_addralign": 4,
       "sh_entsize": 0
     },
     {
       "sh_name": ".text",
       "sh_type": "SHT_PROGBITS",
       "sh_flags": "SHF_ALLOC | SHF_EXECINSTR",
       "sh_addr": 0,
       "sh_offset": 528,
       "sh_size": 39,
       "sh_link": 0,
       "sh_info": 0,
       "sh_addralign": 16,
       "sh_entsize": 0
     },
     {
       "sh_name": ".shstrtab",
       "sh_type": "SHT_STRTAB",
       "sh_flags": "SHF_ALLOC | SHF_EXECINSTR",
       "sh_addr": 0,
       "sh_offset": 576,
       "sh_size": 50,
       "sh_link": 0,
       "sh_info": 0,
       "sh_addralign": 1,
       "sh_entsize": 0,
       "strings": [
         "",
         ".data",
         ".text",
         ".shstrtab",
         ".symtab",
         ".strtab",
         ".rela.text",
         ""
       ]
     },
     {
       "sh_name": ".symtab",
       "sh_type": "SHT_SYMTAB",
       "sh_flags": "SHF_ALLOC | SHF_EXECINSTR",
       "sh_addr": 0,
       "sh_offset": 640,
       "sh_size": 168,
       "sh_link": 5,
       "sh_info": 6,
       "sh_addralign": 8,
       "sh_entsize": 24,
       "symbols": [
         {
           "st_name": "",
           "st_info": "STT_NOTYPE | STB_LOCAL",
           "st_other": "STV_DEFAULT",
           "st_shndx": "UND",
           "st_value": 0,
           "st_size": 0
         },
         {
           "st_name": "test.asm",
           "st_info": "STT_FILE | STB_LOCAL",
           "st_other": "STV_DEFAULT",
           "st_shndx": "ABS",
           "st_value": 0,
           "st_size": 0
         },
         {
           "st_name": "",
           "st_info": "STT_SECTION | STB_LOCAL",
           "st_other": "STV_DEFAULT",
           "st_shndx": ".data",
           "st_value": 0,
           "st_size": 0
         },
         {
           "st_name": "",
           "st_info": "STT_SECTION | STB_LOCAL",
           "st_other": "STV_DEFAULT",
           "st_shndx": "data",
           "st_value": 0,
           "st_size": 0
         },
         {
           "st_name": "msg",
           "st_info": "STT_NOTYPE | STB_LOCAL",
           "st_other": "STV_DEFAULT",
           "st_shndx": ".data",
           "st_value": 0,
           "st_size": 0
         },
         {
           "st_name": "msg_len",
           "st_info": "STT_NOTYPE | STB_LOCAL",
           "st_other": "STV_DEFAULT",
           "st_shndx": "ABS",
           "st_value": 11,
           "st_size": 0
         },
         {
           "st_name": "_start",
           "st_info": "STT_NOTYPE | STB_GLOBAL",
           "st_other": "STV_DEFAULT",
           "st_shndx": "data",
           "st_value": 0,
           "st_size": 0
         }
       ]
     },
     {
       "sh_name": ".strtab",
       "sh_type": "SHT_STRTAB",
       "sh_flags": "SHF_ALLOC | SHF_EXECINSTR",
       "sh_addr": 0,
       "sh_offset": 816,
       "sh_size": 29,
       "sh_link": 0,
       "sh_info": 0,
       "sh_addralign": 1,
       "sh_entsize": 0,
       "strings": [
         "",
         "test.asm",
         "msg",
         "msg_len",
         "_start",
         ""
       ]
     },
     {
       "sh_name": ".rela.text",
       "sh_type": "SHT_RELA",
       "sh_flags": "SHF_ALLOC | SHF_EXECINSTR",
       "sh_addr": 0,
       "sh_offset": 848,
       "sh_size": 24,
       "sh_link": 4,
       "sh_info": 2,
       "sh_addralign": 8,
       "sh_entsize": 24,
       "relocations": [
         {
           "r_offset": 12,
           "r_info": 1,
           "r_addend": 0,
           "__sym_table": 2,
           "__relo_type": "R_X86_64_64"
         }
       ]
     }
   ]
}
```
