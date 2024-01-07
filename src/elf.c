static void
generate_elf(StringBuilder *builder, Codegen codegen, SymbolTable symbol_table, JulsArchitecture target_architecture)
{
    u64 page_size = 1 << 12; // 4096

    string_builder_append_string(builder, S("\x7F""ELF"));

    string_builder_append_u8(builder, 2); // 1 - 32bit, 2 - 64bit
    string_builder_append_u8(builder, 1); // 1 - little endian, 2 - big endian
    string_builder_append_u8(builder, 1); // elf version, has to be 1
    string_builder_append_u8(builder, 0); // osabi
    string_builder_append_u8(builder, 0); // abiversion
    string_builder_append_u8(builder, 0); // padding
    string_builder_append_u16le(builder, 0);
    string_builder_append_u32le(builder, 0);

    string_builder_append_u16le(builder, 2); // type, 2 - executable

    if (target_architecture == JulsArchitectureArm64)
    {
        string_builder_append_u16le(builder, 0xB7);
    }
    else if (target_architecture == JulsArchitectureX86_64)
    {
        string_builder_append_u16le(builder, 0x3E);
    }
    else
    {
        string_builder_append_u16le(builder, 0x00);
    }

    string_builder_append_u32le(builder, 1); // version

    u64 *e_entry = string_builder_append_size(builder, 8); // e_entry
    u64 *e_phoff = string_builder_append_size(builder, 8); // e_phoff
    u64 *e_shoff = string_builder_append_size(builder, 8); // e_shoff

    string_builder_append_u32le(builder, 0); // flags
    string_builder_append_u16le(builder, 64); // size of this file header

    string_builder_append_u16le(builder, 56); // e_phentsize
    u16 *program_header_count = string_builder_append_size(builder, 2); // e_phnum
    string_builder_append_u16le(builder, 64); // e_shentsize
    u16 *section_header_count = string_builder_append_size(builder, 2); // e_shnum
    u16 *string_table_index = string_builder_append_size(builder, 2); // e_shstrndx

    *e_phoff = string_builder_get_size(builder); // this needs to be little-endian

    // program header

    u16 program_header_index = 0;

    string_builder_append_u32le(builder, 1); // type
    string_builder_append_u32le(builder, 4); // flags
    u64 *section_cstring_offset     = string_builder_append_size(builder, 8); // p_offset
    u64 *section_cstring_vaddr      = string_builder_append_size(builder, 8); // p_vaddr
    u64 *section_cstring_paddr      = string_builder_append_size(builder, 8); // p_paddr
    u64 *section_cstring_file_size  = string_builder_append_size(builder, 8); // p_filesz
    u64 *section_cstring_mem_size   = string_builder_append_size(builder, 8); // p_memsz
    string_builder_append_u64le(builder, page_size); // p_align
    program_header_index += 1;


    string_builder_append_u32le(builder, 1); // type
    string_builder_append_u32le(builder, 5); // flags
    u64 *section_text_offset = string_builder_append_size(builder, 8); // p_offset
    u64 *section_text_vaddr  = string_builder_append_size(builder, 8); // p_vaddr
    u64 *section_text_paddr  = string_builder_append_size(builder, 8); // p_paddr
    u64 text_size = string_builder_get_size(&codegen.section_text);
    string_builder_append_u64le(builder, text_size); // p_filesz
    string_builder_append_u64le(builder, text_size); // p_memsz
    string_builder_append_u64le(builder, page_size); // p_align
    program_header_index += 1;

    *program_header_count = program_header_index;

    u64 vaddr = 0x200000;

    // .rodata

    u64 cstring_offset = string_builder_get_size(builder);

    string_builder_append_builder(builder, codegen.section_cstring);

    u64 current_size = string_builder_get_size(builder);
    u64 padding_size = Align(current_size, 4) - current_size;

    for (u64 i = 0; i < padding_size; i += 1)
    {
        string_builder_append_u8(builder, 0);
    }

    u64 cstring_end = string_builder_get_size(builder);

    u64 cstring_vaddr = vaddr;

    *section_cstring_offset     = 0;
    *section_cstring_vaddr      = cstring_vaddr;
    *section_cstring_paddr      = cstring_vaddr;
    *section_cstring_file_size  = cstring_end; // p_filesz
    *section_cstring_mem_size   = cstring_end; // p_memsz

    vaddr += cstring_end;
    vaddr = Align(vaddr, page_size);

    // .text

    u64 text_offset = string_builder_get_size(builder);

    string_builder_append_builder(builder, codegen.section_text);

    vaddr += text_offset;
    u64 text_vaddr = vaddr;

    *section_text_offset = text_offset;
    *section_text_vaddr = text_vaddr;
    *section_text_paddr = text_vaddr;

    *e_entry = text_vaddr;

    vaddr += text_size;
    vaddr = Align(vaddr, page_size);

    {
        for (s32 i = 0; i < codegen.patches.count; i += 1)
        {
            Patch *patch = codegen.patches.items + i;

            u64 instruction_address = text_vaddr + patch->instruction_offset;
            u64 string_address = cstring_vaddr + cstring_offset + patch->string_offset;

            if (target_architecture == JulsArchitectureArm64)
            {
                u64 string_page = string_address / 4096;
                u64 instruction_page = instruction_address / 4096;

                s64 page_count = string_page - instruction_page;
                u64 offset = string_address & 0xFFF;

                // ADRP
                *((u32 *) patch->patch + 0) = 0x90000000 | ((page_count & 0x3) << 29) | ((page_count & 0x1FFFFC) << 3) | ARM64_R1;
                // ADD (immediate)
                *((u32 *) patch->patch + 1) = 0x91000000 | ((u32) offset << 10) | (ARM64_R1 << 5) | ARM64_R1;
            }
            else if (target_architecture == JulsArchitectureX86_64)
            {
                *(s32 *) patch->patch = (s32) ((s64) string_address - (s64) instruction_address);
            }
        }
    }

    // .shstrtab

    u64 shstrtab_offset = string_builder_get_size(builder);

    string_builder_append_string(builder, S("\0.shstrtab\0.text\0.rodata\0.symtab\0.strtab\0"));

    u64 shstrtab_size = string_builder_get_size(builder) - shstrtab_offset;

    // .strtab

    StringBuilder symbol_table_section;
    initialize_string_builder(&symbol_table_section, &default_allocator);

    string_builder_append_u32le(&symbol_table_section, 0); // st_name
    string_builder_append_u8(&symbol_table_section, 0);    // st_info
    string_builder_append_u8(&symbol_table_section, 0);    // st_other
    string_builder_append_u16le(&symbol_table_section, 0); // st_shndx
    string_builder_append_u64le(&symbol_table_section, 0); // st_value
    string_builder_append_u64le(&symbol_table_section, 0); // st_size

    u64 strtab_offset = string_builder_get_size(builder);

    string_builder_append_u8(builder, 0);

    for (s32 i = 0; i < symbol_table.count; i += 1)
    {
        SymbolEntry *entry = symbol_table.items + i;
        u32 name_offset = (u32) (string_builder_get_size(builder) - strtab_offset);

        string_builder_append_u32le(&symbol_table_section, name_offset); // st_name
        string_builder_append_u8(&symbol_table_section, 0x12);    // st_info
        string_builder_append_u8(&symbol_table_section, 0);    // st_other
        string_builder_append_u16le(&symbol_table_section, 2); // st_shndx
        string_builder_append_u64le(&symbol_table_section, text_vaddr + entry->offset); // st_value
        string_builder_append_u64le(&symbol_table_section, entry->size); // st_size

        string_builder_append_string(builder, entry->name);
        string_builder_append_u8(builder, 0);
    }

    u64 strtab_size = string_builder_get_size(builder) - strtab_offset;

    // .symtab

    u64 symtab_offset = string_builder_get_size(builder);

    string_builder_append_builder(builder, symbol_table_section);

    u64 symtab_size = string_builder_get_size(builder) - symtab_offset;

    *e_shoff = string_builder_get_size(builder); // this needs to be little-endian

    // section header

    u16 section_header_index = 0;

    // NULL section

    string_builder_append_u32le(builder, 0); // sh_name
    string_builder_append_u32le(builder, 0); // sh_type
    string_builder_append_u64le(builder, 0); // sh_flags
    string_builder_append_u64le(builder, 0); // sh_addr
    string_builder_append_u64le(builder, 0); // sh_offset
    string_builder_append_u64le(builder, 0); // sh_size
    string_builder_append_u32le(builder, 0); // sh_link
    string_builder_append_u32le(builder, 0); // sh_info
    string_builder_append_u64le(builder, 0); // sh_addralign
    string_builder_append_u64le(builder, 0); // sh_entsize
    section_header_index += 1;

    // .rodata

    string_builder_append_u32le(builder, 17); // sh_name
    string_builder_append_u32le(builder, 1); // sh_type
    string_builder_append_u64le(builder, 2); // sh_flags
    string_builder_append_u64le(builder, cstring_vaddr + cstring_offset); // sh_addr
    string_builder_append_u64le(builder, cstring_offset); // sh_offset
    string_builder_append_u64le(builder, cstring_end - cstring_offset); // sh_size
    string_builder_append_u32le(builder, 0); // sh_link
    string_builder_append_u32le(builder, 0); // sh_info
    string_builder_append_u64le(builder, 1); // sh_addralign
    string_builder_append_u64le(builder, 0); // sh_entsize
    section_header_index += 1;

    // .text

    assert(section_header_index == 2);

    string_builder_append_u32le(builder, 11); // sh_name
    string_builder_append_u32le(builder, 1); // sh_type
    string_builder_append_u64le(builder, 6); // sh_flags
    string_builder_append_u64le(builder, text_vaddr); // sh_addr
    string_builder_append_u64le(builder, text_offset); // sh_offset
    string_builder_append_u64le(builder, text_size); // sh_size
    string_builder_append_u32le(builder, 0); // sh_link
    string_builder_append_u32le(builder, 0); // sh_info
    string_builder_append_u64le(builder, 1); // sh_addralign
    string_builder_append_u64le(builder, 0); // sh_entsize
    section_header_index += 1;

    // .shstrtab

    *string_table_index = section_header_index;

    string_builder_append_u32le(builder, 1); // sh_name
    string_builder_append_u32le(builder, 3); // sh_type
    string_builder_append_u64le(builder, 0); // sh_flags
    string_builder_append_u64le(builder, 0); // sh_addr
    string_builder_append_u64le(builder, shstrtab_offset); // sh_offset
    string_builder_append_u64le(builder, shstrtab_size); // sh_size
    string_builder_append_u32le(builder, 0); // sh_link
    string_builder_append_u32le(builder, 0); // sh_info
    string_builder_append_u64le(builder, 1); // sh_addralign
    string_builder_append_u64le(builder, 0); // sh_entsize
    section_header_index += 1;

    // .strtab

    u16 strtab_index = section_header_index;

    string_builder_append_u32le(builder, 33); // sh_name
    string_builder_append_u32le(builder, 3); // sh_type
    string_builder_append_u64le(builder, 0); // sh_flags
    string_builder_append_u64le(builder, 0); // sh_addr
    string_builder_append_u64le(builder, strtab_offset); // sh_offset
    string_builder_append_u64le(builder, strtab_size); // sh_size
    string_builder_append_u32le(builder, 0); // sh_link
    string_builder_append_u32le(builder, 0); // sh_info
    string_builder_append_u64le(builder, 1); // sh_addralign
    string_builder_append_u64le(builder, 0); // sh_entsize
    section_header_index += 1;

    // .symtab

    string_builder_append_u32le(builder, 25); // sh_name
    string_builder_append_u32le(builder, 2); // sh_type
    string_builder_append_u64le(builder, 0); // sh_flags
    string_builder_append_u64le(builder, 0); // sh_addr
    string_builder_append_u64le(builder, symtab_offset); // sh_offset
    string_builder_append_u64le(builder, symtab_size); // sh_size
    string_builder_append_u32le(builder, strtab_index); // sh_link // NOTE: this links to the .strtab section above
    string_builder_append_u32le(builder, 1); // sh_info
    string_builder_append_u64le(builder, 1); // sh_addralign
    string_builder_append_u64le(builder, 24); // sh_entsize
    section_header_index += 1;

    *section_header_count = section_header_index;
}
