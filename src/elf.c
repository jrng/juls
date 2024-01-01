static void
generate_elf(StringBuilder *builder, Codegen codegen, SymbolTable symbol_table, JulsArchitecture target_architecture)
{
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
    string_builder_append_u16le(builder, 1); // e_phnum
    string_builder_append_u16le(builder, 64); // e_shentsize
    string_builder_append_u16le(builder, 5); // e_shnum
    string_builder_append_u16le(builder, 2); // e_shstrndx

    *e_phoff = string_builder_get_size(builder); // this needs to be little-endian

    // program header

    string_builder_append_u32le(builder, 1); // type
    string_builder_append_u32le(builder, 5); // flags

    u64 *p_offset = string_builder_append_size(builder, 8); // p_offset
    u64 *p_vaddr  = string_builder_append_size(builder, 8); // p_vaddr
    u64 *p_paddr  = string_builder_append_size(builder, 8); // p_paddr

    u64 text_size = string_builder_get_size(&codegen.section_text);

    string_builder_append_u64le(builder, text_size); // p_filesz
    string_builder_append_u64le(builder, text_size); // p_memsz
    string_builder_append_u64le(builder, 0x1000); // p_align

    u64 offset = string_builder_get_size(builder);
    u64 vaddr = 0x400000 + offset;

    *p_offset = offset;
    *p_vaddr = vaddr;
    *p_paddr = vaddr;
    *e_entry = vaddr;

    string_builder_append_builder(builder, codegen.section_text);

    // .shstrtab

    u64 shstrtab_offset = string_builder_get_size(builder);

    string_builder_append_string(builder, S("\0.shstrtab\0.text\0.symtab\0.strtab\0"));

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
        string_builder_append_u16le(&symbol_table_section, 1); // st_shndx
        string_builder_append_u64le(&symbol_table_section, vaddr + entry->offset); // st_value
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

    // .text

    string_builder_append_u32le(builder, 11); // sh_name
    string_builder_append_u32le(builder, 1); // sh_type
    string_builder_append_u64le(builder, 6); // sh_flags
    string_builder_append_u64le(builder, vaddr); // sh_addr
    string_builder_append_u64le(builder, offset); // sh_offset
    string_builder_append_u64le(builder, text_size); // sh_size
    string_builder_append_u32le(builder, 0); // sh_link
    string_builder_append_u32le(builder, 0); // sh_info
    string_builder_append_u64le(builder, 1); // sh_addralign
    string_builder_append_u64le(builder, 0); // sh_entsize

    // .shstrtab

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

    // .strtab

    string_builder_append_u32le(builder, 25); // sh_name
    string_builder_append_u32le(builder, 3); // sh_type
    string_builder_append_u64le(builder, 0); // sh_flags
    string_builder_append_u64le(builder, 0); // sh_addr
    string_builder_append_u64le(builder, strtab_offset); // sh_offset
    string_builder_append_u64le(builder, strtab_size); // sh_size
    string_builder_append_u32le(builder, 0); // sh_link
    string_builder_append_u32le(builder, 0); // sh_info
    string_builder_append_u64le(builder, 1); // sh_addralign
    string_builder_append_u64le(builder, 0); // sh_entsize

    // .symtab

    string_builder_append_u32le(builder, 17); // sh_name
    string_builder_append_u32le(builder, 2); // sh_type
    string_builder_append_u64le(builder, 0); // sh_flags
    string_builder_append_u64le(builder, 0); // sh_addr
    string_builder_append_u64le(builder, symtab_offset); // sh_offset
    string_builder_append_u64le(builder, symtab_size); // sh_size
    string_builder_append_u32le(builder, 3); // sh_link // NOTE: this links to the .strtab section above
    string_builder_append_u32le(builder, 1); // sh_info
    string_builder_append_u64le(builder, 1); // sh_addralign
    string_builder_append_u64le(builder, 24); // sh_entsize
}
