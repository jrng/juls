static void
generate_macho(StringBuilder *builder, Codegen codegen, SymbolTable symbol_table, JulsArchitecture target_architecture)
{
    u64 page_size = 1 << 12; // 4096
    u64 vm_base = 0x100000000; // 4 GiB

    string_builder_append_u32le(builder, 0xFEEDFACF); // magic

    if (target_architecture == JulsArchitectureArm64)
    {
        string_builder_append_u32le(builder, 0x100000C); // cpu type
        string_builder_append_u32le(builder, 0); // cpu subtype

        page_size = 1 << 14; // 16384
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

    //
    // begin load commands
    //

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
    u64 *segment_text_vmsize = string_builder_append_size(builder, 8);
    string_builder_append_u64le(builder, 0); // file offset
    u64 *segment_text_size = string_builder_append_size(builder, 8);
    string_builder_append_u32le(builder, 5); // maximum protection
    string_builder_append_u32le(builder, 5); // initial protection
    string_builder_append_u32le(builder, 2); // number of section
    string_builder_append_u32le(builder, 0); // flags

    // section __TEXT,__text
    string_builder_append_string(builder, S("__text\0\0\0\0\0\0\0\0\0\0")); // section name
    string_builder_append_string(builder, S("__TEXT\0\0\0\0\0\0\0\0\0\0")); // segment name
    u64 *section_text_vmaddr = string_builder_append_size(builder, 8); // section address
    u64 *section_text_size = string_builder_append_size(builder, 8); // section size
    u32 *section_text_offset = string_builder_append_size(builder, 4); // section file offset
    string_builder_append_u32le(builder, 2); // alignment
    string_builder_append_u32le(builder, 0); // relocations file offset
    string_builder_append_u32le(builder, 0); // number of relocations
    string_builder_append_u32le(builder, 0x80000400); // flag/type
    string_builder_append_u32le(builder, 0); // reserved1
    string_builder_append_u32le(builder, 0); // reserved2
    string_builder_append_u32le(builder, 0); // reserved3

    // section __TEXT,__cstring
    string_builder_append_string(builder, S("__cstring\0\0\0\0\0\0\0")); // section name
    string_builder_append_string(builder, S("__TEXT\0\0\0\0\0\0\0\0\0\0")); // segment name
    u64 *section_cstring_vmaddr = string_builder_append_size(builder, 8); // section address
    u64 *section_cstring_size = string_builder_append_size(builder, 8); // section size
    u32 *section_cstring_offset = string_builder_append_size(builder, 4); // section file offset
    string_builder_append_u32le(builder, 0); // alignment
    string_builder_append_u32le(builder, 0); // relocations file offset
    string_builder_append_u32le(builder, 0); // number of relocations
    string_builder_append_u32le(builder, 0x00000002); // flag/type
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
    u64 *linkedit_vmsize = string_builder_append_size(builder, 8);
    u64 *linkedit_offset = string_builder_append_size(builder, 8);
    u64 *linkedit_size = string_builder_append_size(builder, 8);
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

    u32 *symbol_table_offset = string_builder_append_size(builder, 4);
    string_builder_append_u32le(builder, symbol_table.count + 1); // number of symbol table entries
    u32 *string_table_offset = string_builder_append_size(builder, 4);
    u32 *string_table_size = string_builder_append_size(builder, 4);

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
    string_builder_append_u32le(builder, symbol_table.count + 1);
    string_builder_append_u32le(builder, symbol_table.count + 1);
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

    //
    // end load commands
    //

    u64 text_size = Align(string_builder_get_size(&codegen.section_text), 4);
    u64 cstring_size = Align(string_builder_get_size(&codegen.section_cstring), 4);

    u64 segment_size = load_commands_end + text_size + cstring_size;
    u64 padding_size = Align(segment_size, page_size) - segment_size;

    for (u64 i = 0; i < padding_size; i += 1)
    {
        string_builder_append_u8(builder, 0);
    }

    u64 text_start = string_builder_get_size(builder);

    *entry_point = text_start;

    string_builder_append_builder(builder, codegen.section_text);

    u64 text_end = string_builder_get_size(builder);

    *section_text_vmaddr = vm_base + text_start;
    *section_text_size = text_end - text_start;
    *section_text_offset = (u32) text_start;

    padding_size = (text_size + cstring_size) -
                   (string_builder_get_size(&codegen.section_text) +
                    string_builder_get_size(&codegen.section_cstring));

    for (u64 i = 0; i < padding_size; i += 1)
    {
        string_builder_append_u8(builder, 0);
    }

    u64 cstring_start = string_builder_get_size(builder);

    string_builder_append_builder(builder, codegen.section_cstring);

    u64 cstring_end = string_builder_get_size(builder);

    *section_cstring_vmaddr = vm_base + cstring_start;
    *section_cstring_size = cstring_end - cstring_start;
    *section_cstring_offset = (u32) cstring_start;

    u64 segment_text_end = string_builder_get_size(builder);

    *segment_text_vmsize = segment_text_end;
    *segment_text_size = segment_text_end;

    {
        for (s32 i = 0; i < codegen.patches.count; i += 1)
        {
            Patch *patch = codegen.patches.items + i;

            u64 instruction_address = vm_base + text_start + patch->instruction_offset;
            u64 string_address = vm_base + cstring_start + patch->string_offset;

            if (target_architecture == JulsArchitectureArm64)
            {
                u64 page_count = (string_address - instruction_address) / 4096;
                u64 offset = string_address & 0xFFF;

                *((u32 *) patch->patch + 0) = 0x90000000 | ((page_count & 0x3) << 29) | ((page_count & 0x1FFFFC) << 3) | ARM64_R1;
                *((u32 *) patch->patch + 1) = 0x91000000 | ((u32) offset << 10) | (ARM64_R1 << 5) | ARM64_R1;
            }
            else if (target_architecture == JulsArchitectureX86_64)
            {
                assert(!"not implemented");
            }
        }
    }

    // THIS MUST BE AT THE END OF THE FILE
    u64 linkedit_start = string_builder_get_size(builder);

    StringBuilder symbol_table_section;
    initialize_string_builder(&symbol_table_section, &default_allocator);

    u64 string_table_start = string_builder_get_size(builder);

    string_builder_append_u8(builder, 0);
    string_builder_append_string(builder, S("__mh_execute_header"));
    string_builder_append_u8(builder, 0);

    string_builder_append_u32le(&symbol_table_section, 1);
    string_builder_append_u8(&symbol_table_section, 0x0F); // symbol type
    string_builder_append_u8(&symbol_table_section, 1); // section number
    string_builder_append_u16le(&symbol_table_section, 0x10); // data info
    string_builder_append_u64le(&symbol_table_section, vm_base); // symbol address

    for (s32 i = 0; i < symbol_table.count; i += 1)
    {
        SymbolEntry *entry = symbol_table.items + i;
        u32 name_offset = (u32) (string_builder_get_size(builder) - string_table_start);

        string_builder_append_u32le(&symbol_table_section, name_offset);
        string_builder_append_u8(&symbol_table_section, 0x0F); // symbol type
        string_builder_append_u8(&symbol_table_section, 1); // section number
        string_builder_append_u16le(&symbol_table_section, 0); // data info
        string_builder_append_u64le(&symbol_table_section, vm_base + text_start + entry->offset); // symbol address

        string_builder_append_string(builder, entry->name);
        string_builder_append_u8(builder, 0);
    }

    u64 string_table_end = string_builder_get_size(builder);

    *string_table_offset = (u32) string_table_start;
    *string_table_size = (u32) (string_table_end - string_table_start);

    u64 symbol_table_start = string_builder_get_size(builder);

    string_builder_append_builder(builder, symbol_table_section);

    u64 symbol_table_end = string_builder_get_size(builder);

    *symbol_table_offset = (u32) symbol_table_start;

    u64 linkedit_end = string_builder_get_size(builder);

    *linkedit_vmaddr = vm_base + linkedit_start;
    *linkedit_offset = linkedit_start;
    *linkedit_vmsize = Align(linkedit_end - linkedit_start, page_size);
    *linkedit_size = linkedit_end - linkedit_start;
}
