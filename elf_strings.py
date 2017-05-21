#!/usr/bin/env python3

import sys
import re


FUNCTIONS = [
    ("ELFCLASS", "ei_class"),
    ("ELFDATA", "ei_data"),
    ("EV_", "ei_version"),
    ("ELFOSABI", "ei_osabi"),
    ("ET_", "e_type"),
    ("EM_", "e_machine"),
    ("EV_", "e_version"),
    ("PT_", "p_type"),
    ("SHT_", "sh_type"),
    ("STT_", "st_type"),
    ("STB_", "st_binding"),
    ("STV_", "st_other"),
    ("R_X86_64_", "relo_type_x86_64"),
]


class Define(object):

    def __init__(self, key, value, comment):
        self.key = key
        self.value = value
        self.comment = comment

    def __repr__(self):
        return "{} = {}".format(self.key, self.value)


defines = []


def get_defines(prefix):
    ret = []

    for define in defines:
        if define.key.startswith(prefix):
            ret.append(define)

    return ret


def sort_by_value(defines):
    ret = {}

    for define in defines:
        if define.value not in ret:
            ret[define.value] = []

        ret[define.value].append(define)

    return ret


def generate_function(prefix, name):
    ret = [
        "const char* str_{}(long long v)".format(name),
        "{",
        "    switch (v)",
        "    {",
    ]

    for key, defines in sort_by_value(get_defines(prefix)).items():
        val = " | ".join([d.key for d in defines])

        ret.extend([
            "        case {}:".format(key),
            "            return \"{}\";".format(val),
        ])

    ret.extend([
        "        default:",
        "            printf(\"Unrecognized {}: %lld\\n\", v);".format(name),
        "            exit(1);",
        "    }",
        "}",
    ])

    return ret


try:
    with open("/usr/include/elf.h", "r") as f:
        lines = [l.strip() for l in f.read().split("\n")]
except:
    print("Failed to read /usr/include/elf.h")
    sys.exit(1)


for line in lines:
    if not line.startswith("#define "):
        continue

    line = line[len("#define "):]

    parts = line.split()
    key = parts[0]
    value = parts[1]

    if "(" in key or ")" in key or "(" in value or ")" in value:
        continue

    comment = ""
    parts = line.split("/*")
    if len(parts) > 1:
        if parts[1].endswith("*/"):
            comment = parts[1][:-2].strip()

    defines.append(Define(key, value, comment))


to_remove = []


for define in defines:
    if define.value.startswith("\""):
        to_remove.append(define)
        continue

    if define.value.isdigit() or define.value.lower().startswith("0x"):
        define.value = int(define.value, 0)
        continue

    if define.value[0] == "'" and define.value[-1] == "'":
        define.value = ord(define.value[1:-1])
        continue

    target = None
    for d in defines:
        if d.key == define.value:
            target = d
            break

    if target is None:
        print("Can't resolve: {}".format(define))
        sys.exit(2)

    define.value = target.value


for r in to_remove:
    defines.remove(r)


funcs = ""
for func in FUNCTIONS:
    if funcs != "":
        funcs += "\n"

    funcs += "\n".join(generate_function(*func)) + "\n"

with open("elf_strings.h", "w") as f:
    f.write(funcs)
