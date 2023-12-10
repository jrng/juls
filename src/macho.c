static void
generate_macho(StringBuilder *builder, StringBuilder code, JulsArchitecture target_architecture)
{
    u64 page_size = 2 << 12; // 4096
    u64 vm_base = 0x100000000; // 4 GiB

    string_builder_append_u32le(builder, 0xFEEDFACF); // magic

    if (target_architecture == JulsArchitectureArm64)
    {
        string_builder_append_u32le(builder, 0x100000C); // cpu type
        string_builder_append_u32le(builder, 0); // cpu subtype

        page_size = 2 << 14; // 16384
    }
    else if (target_architecture == JulsArchitectureX86_64)
    {
        string_builder_append_u32le(builder, 0x1000007); // cpu type
        string_builder_append_u32le(builder, 0x3); // cpu subtype
    }
    else
    {
        string_builder_append_u32le(builder, 0); // cpu type
        string_builder_append_u32le(builder, 0); // cpu subtype
    }

    string_builder_append_u32le(builder, 2); // file type
    u32 *load_commands_count = string_builder_append_size(builder, 4);
    u32 *load_commands_size = string_builder_append_size(builder, 4);
    string_builder_append_u32le(builder, 0x00200085); // flags
    string_builder_append_u32le(builder, 0); // reserved

    u32 load_command_count = 0;

    // load commands

    u64 load_commands_start = string_builder_get_size(builder);

    u32 *command_size;
    u64 command_start, command_end;

    // __PAGEZERO
    command_start = string_builder_get_size(builder);

    string_builder_append_u32le(builder, 0x19); // command type
    command_size = string_builder_append_size(builder, 4);
    string_builder_append_string(builder, S("__PAGEZERO\0\0\0\0\0\0"));
    string_builder_append_u64le(builder, 0); // vmaddr
    string_builder_append_u64le(builder, vm_base); // vmsize
    string_builder_append_u64le(builder, 0); // file offset
    string_builder_append_u64le(builder, 0); // file size
    string_builder_append_u32le(builder, 0); // maximum protection
    string_builder_append_u32le(builder, 0); // initial protection
    string_builder_append_u32le(builder, 0); // number of section
    string_builder_append_u32le(builder, 0); // flags

    command_end = string_builder_get_size(builder);
    *command_size = (u32) (command_end - command_start);
    load_command_count += 1;

    // __TEXT
    command_start = string_builder_get_size(builder);

    string_builder_append_u32le(builder, 0x19); // command type
    command_size = string_builder_append_size(builder, 4);
    string_builder_append_string(builder, S("__TEXT\0\0\0\0\0\0\0\0\0\0"));
    string_builder_append_u64le(builder, vm_base); // vmaddr
    u64 *code_vmsize = string_builder_append_size(builder, 8);
    string_builder_append_u64le(builder, 0); // file offset
    u64 *code_size = string_builder_append_size(builder, 8);
    string_builder_append_u32le(builder, 5); // maximum protection
    string_builder_append_u32le(builder, 5); // initial protection
    string_builder_append_u32le(builder, 1); // number of section
    string_builder_append_u32le(builder, 0); // flags

    // section __TEXT,__text
    string_builder_append_string(builder, S("__text\0\0\0\0\0\0\0\0\0\0")); // section name
    string_builder_append_string(builder, S("__TEXT\0\0\0\0\0\0\0\0\0\0")); // segment name
    u64 *section_vmaddr = string_builder_append_size(builder, 8); // section address
    u64 *section_text_size = string_builder_append_size(builder, 8); // section size
    u32 *section_text_offset = string_builder_append_size(builder, 4); // section file offset
    string_builder_append_u32le(builder, 2); // alignment
    string_builder_append_u32le(builder, 0); // relocations file offset
    string_builder_append_u32le(builder, 0); // number of relocations
    string_builder_append_u32le(builder, 0x80000400); // flag/type
    string_builder_append_u32le(builder, 0); // reserved1
    string_builder_append_u32le(builder, 0); // reserved2
    string_builder_append_u32le(builder, 0); // reserved3

    command_end = string_builder_get_size(builder);
    *command_size = (u32) (command_end - command_start);
    load_command_count += 1;

    // __LINKEDIT
    command_start = string_builder_get_size(builder);

    string_builder_append_u32le(builder, 0x19); // command type
    command_size = string_builder_append_size(builder, 4);
    string_builder_append_string(builder, S("__LINKEDIT\0\0\0\0\0\0"));
    u64 *linkedit_vmaddr = string_builder_append_size(builder, 8);
    string_builder_append_u64le(builder, page_size); // vmsize
    u64 *linkedit_offset = string_builder_append_size(builder, 8);
    string_builder_append_u64le(builder, 0); // file size
    // TODO: there needs to be atleast one writable segment,
    // once we have a __DATA segment we can make this read only
    string_builder_append_u32le(builder, 3); // maximum protection
    string_builder_append_u32le(builder, 3); // initial protection
    string_builder_append_u32le(builder, 0); // number of section
    string_builder_append_u32le(builder, 0); // flags

    command_end = string_builder_get_size(builder);
    *command_size = (u32) (command_end - command_start);
    load_command_count += 1;

    // LOAD_DYLINKER
    command_start = string_builder_get_size(builder);

    String linker = S("/usr/lib/dyld");

    string_builder_append_u32le(builder, 0xE); // command type
    command_size = string_builder_append_size(builder, 4);
    string_builder_append_u32le(builder, 12); // linker string offset
    string_builder_append_string(builder, linker);

    {
        u64 pad_start = string_builder_get_size(builder);
        u64 pad_end = Align(pad_start + 1, 8);

        for (u64 i = pad_start; i < pad_end; i += 1)
        {
            string_builder_append_u8(builder, 0);
        }
    }

    command_end = string_builder_get_size(builder);
    *command_size = (u32) (command_end - command_start);
    load_command_count += 1;

    // LOAD_DYLIB
    command_start = string_builder_get_size(builder);

    String lib_str = S("/usr/lib/libSystem.B.dylib");

    string_builder_append_u32le(builder, 0xC); // command type
    command_size = string_builder_append_size(builder, 4);
    string_builder_append_u32le(builder, 24); // linker string offset
    string_builder_append_u32le(builder, 0); // timestamp
    // 1319.0.0
    string_builder_append_u32le(builder, 0x00010000); // current version
    string_builder_append_u32le(builder, 0x00010000); // compatible version
    string_builder_append_string(builder, lib_str);

    {
        u64 pad_start = string_builder_get_size(builder);
        u64 pad_end = Align(pad_start + 1, 8);

        for (u64 i = pad_start; i < pad_end; i += 1)
        {
            string_builder_append_u8(builder, 0);
        }
    }

    command_end = string_builder_get_size(builder);
    *command_size = (u32) (command_end - command_start);
    load_command_count += 1;

    // LC_MAIN
    command_start = string_builder_get_size(builder);

    string_builder_append_u32le(builder, 0x80000028); // command type
    command_size = string_builder_append_size(builder, 4);
    u64 *entry_point = string_builder_append_size(builder, 8);
    string_builder_append_u64le(builder, 0); // stack size

    command_end = string_builder_get_size(builder);
    *command_size = (u32) (command_end - command_start);
    load_command_count += 1;

    // LC_SYMTAB
    command_start = string_builder_get_size(builder);

    string_builder_append_u32le(builder, 0x2); // command type
    command_size = string_builder_append_size(builder, 4);

    string_builder_append_u32le(builder, 0); // symbol table offset
    string_builder_append_u32le(builder, 0); // number of symbol table entries
    string_builder_append_u32le(builder, 0); // string table offset
    string_builder_append_u32le(builder, 0); // string table size in bytes

    command_end = string_builder_get_size(builder);
    *command_size = (u32) (command_end - command_start);
    load_command_count += 1;

    // LC_DYSYMTAB
    command_start = string_builder_get_size(builder);

    string_builder_append_u32le(builder, 0xB); // command type
    command_size = string_builder_append_size(builder, 4);

    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);
    string_builder_append_u32le(builder, 0);

    command_end = string_builder_get_size(builder);
    *command_size = (u32) (command_end - command_start);
    load_command_count += 1;

    u64 load_commands_end = string_builder_get_size(builder);
    *load_commands_size = (u32) (load_commands_end - load_commands_start);
    *load_commands_count = load_command_count;

    for (u64 i = 0; i < 256; i += 1)
    {
        string_builder_append_u8(builder, 0);
    }

    u64 code_start = string_builder_get_size(builder);

    *entry_point = code_start;

    string_builder_append_builder(builder, code);

    u64 code_end = string_builder_get_size(builder);

    *code_vmsize = code_end;
    *code_size = code_end;

    *section_vmaddr = vm_base + code_start;
    *section_text_size = code_end - code_start;
    *section_text_offset = (u32) code_start;

    u64 padding_size = Align(code_end, page_size) - code_end;

    for (u64 i = 0; i < padding_size; i += 1)
    {
        string_builder_append_u8(builder, 0);
    }

    *linkedit_vmaddr = vm_base + string_builder_get_size(builder);
    *linkedit_offset = string_builder_get_size(builder);
}
