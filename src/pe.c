static void
generate_pe(StringBuilder *builder, Codegen codegen, SymbolTable symbol_table, JulsArchitecture target_architecture)
{
    u32 section_alignment = 1 << 12; // 4096
    u32 file_alignment    = 1 << 9;  // 512

    // DOS header

    string_builder_append_string(builder, S("MZ"));

    for (s32 i = 0; i < 58; i += 1)
    {
        string_builder_append_u8(builder, 0);
    }

    string_builder_append_u32le(builder, string_builder_get_size(builder) + 4);

    // PE header

    string_builder_append_string(builder, S("PE\0\0")); // Signature

    // COFF header

    if (target_architecture == JulsArchitectureArm64)
    {
        string_builder_append_u16le(builder, 0xAA64);
    }
    else if (target_architecture == JulsArchitectureX86_64)
    {
        string_builder_append_u16le(builder, 0x8664);
    }
    else
    {
        string_builder_append_u16le(builder, 0);
    }

    u16 *number_of_sections =
        string_builder_append_size(builder, 2);       // NumberOfSections
    string_builder_append_u32le(builder, 0);          // TimeDateStamp
    string_builder_append_u32le(builder, 0);          // PointerToSymbolTable
    string_builder_append_u32le(builder, 0);          // NumberOfSymbols
    u16 *size_of_optional_header =
        string_builder_append_size(builder, 2);       // SizeOfOptionalHeader
    string_builder_append_u16le(builder, 0x0022);     // Characteristics

    // optional header

    u64 optional_header_offset = string_builder_get_size(builder);

    string_builder_append_u16le(builder, 0x020B);     // Magic
    string_builder_append_u8(builder, 0);             // MajorLinkerVersion
    string_builder_append_u8(builder, 0);             // MinorLinkerVersion
    string_builder_append_u32le(builder, 0);          // SizeOfCode
    string_builder_append_u32le(builder, 0);          // SizeOfInitializedData
    string_builder_append_u32le(builder, 0);          // SizeOfUninitializedData
    u32 *address_of_entry_point =
        string_builder_append_size(builder, 4);       // AddressOfEntryPoint
    string_builder_append_u32le(builder, 0);          // BaseOfCode

    string_builder_append_u64le(builder, 0x00400000); // ImageBase
    string_builder_append_u32le(builder, section_alignment); // SectionAlignment
    string_builder_append_u32le(builder, file_alignment);    // FileAlignment
    string_builder_append_u16le(builder, 0);          // MajorOperatingSystemVersion
    string_builder_append_u16le(builder, 0);          // MinorOperatingSystemVersion
    string_builder_append_u16le(builder, 0);          // MajorImageVersion
    string_builder_append_u16le(builder, 0);          // MinorImageVersion
    string_builder_append_u16le(builder, 6);          // MajorSubsystemVersion
    string_builder_append_u16le(builder, 0);          // MinorSubsystemVersion
    string_builder_append_u32le(builder, 0);          // Win32VersionValue
    u32 *size_of_image =
        string_builder_append_size(builder, 4);       // SizeOfImage
    u32 *size_of_header =
        string_builder_append_size(builder, 4);       // SizeOfHeader
    string_builder_append_u32le(builder, 0);          // CheckSum
    string_builder_append_u16le(builder, 3);          // Subsystem
    string_builder_append_u16le(builder, 0);          // DllCharacteristics
    string_builder_append_u64le(builder, 0x100000);   // SizeOfStackReserve
    string_builder_append_u64le(builder, 0x1000);     // SizeOfStackCommit
    string_builder_append_u64le(builder, 0);          // SizeOfHeapReserve
    string_builder_append_u64le(builder, 0);          // SizeOfHeapCommit
    string_builder_append_u32le(builder, 0);          // LoaderFlags
    string_builder_append_u32le(builder, 16);         // NumberOfRvaAndSize

    // export directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // import directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // resource directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // exception directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // security directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // base relocation table
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // debug directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // architecture specific data
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // global pointer register relative virtual address
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // thread local storage directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // load configuration directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // bound import directory
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // import address table
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // delay import table
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // com or .net descriptor table (CLI header)
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    // reserved
    string_builder_append_u32le(builder, 0); // VirtualAddress
    string_builder_append_u32le(builder, 0); // Size

    *size_of_optional_header = (u16) (string_builder_get_size(builder) - optional_header_offset); // this needs to be little-endian

    // section table

    u16 section_header_index = 0;

    // .text

    string_builder_append_string(builder, S(".text\0\0\0")); // Name
    u32 *text_virtual_size =
        string_builder_append_size(builder, 4);              // VirtualSize
    u32 *text_virtual_address =
        string_builder_append_size(builder, 4);              // VirtualAddress
    u32 *text_size_of_raw_data =
        string_builder_append_size(builder, 4);              // SizeOfRawData
    u32 *text_pointer_to_raw_data =
        string_builder_append_size(builder, 4);              // PointerToRawData
    string_builder_append_u32le(builder, 0);                 // PointerToRelocations
    string_builder_append_u32le(builder, 0);                 // PointerToLinenumbers
    string_builder_append_u16le(builder, 0);                 // NumberOfRelocations
    string_builder_append_u16le(builder, 0);                 // NumberOfLinenumbers
    string_builder_append_u32le(builder, 0x60000020);        // Characteristics
    section_header_index += 1;

    // .rdata

    u32 *rdata_virtual_size = 0;
    u32 *rdata_virtual_address = 0;
    u32 *rdata_size_of_raw_data = 0;
    u32 *rdata_pointer_to_raw_data = 0;

    if (string_builder_get_size(&codegen.section_cstring) > 0)
    {
        string_builder_append_string(builder, S(".rdata\0\0"));  // Name
        rdata_virtual_size =
            string_builder_append_size(builder, 4);              // VirtualSize
        rdata_virtual_address =
            string_builder_append_size(builder, 4);              // VirtualAddress
        rdata_size_of_raw_data =
            string_builder_append_size(builder, 4);              // SizeOfRawData
        rdata_pointer_to_raw_data =
            string_builder_append_size(builder, 4);              // PointerToRawData
        string_builder_append_u32le(builder, 0);                 // PointerToRelocations
        string_builder_append_u32le(builder, 0);                 // PointerToLinenumbers
        string_builder_append_u16le(builder, 0);                 // NumberOfRelocations
        string_builder_append_u16le(builder, 0);                 // NumberOfLinenumbers
        string_builder_append_u32le(builder, 0x40000040);        // Characteristics
        section_header_index += 1;
    }

    *number_of_sections = section_header_index;

    string_builder_align(builder, file_alignment, 0);

    *size_of_header = (u32) string_builder_get_size(builder);

    // .text

    u64 text_start = string_builder_get_size(builder);

    string_builder_append_builder(builder, codegen.section_text);
    string_builder_align(builder, file_alignment, 0);

    u64 text_end = string_builder_get_size(builder);

    u64 vaddr = Align(text_start, section_alignment);

    *address_of_entry_point = vaddr;
    *text_virtual_size = (u32) (text_end - text_start);
    *text_virtual_address = vaddr;
    *text_pointer_to_raw_data = (u32) text_start;
    *text_size_of_raw_data = Align((u32) (text_end - text_start), file_alignment);

    vaddr += text_end - text_start;
    vaddr = Align(vaddr, section_alignment);

    // .rdata

    if (string_builder_get_size(&codegen.section_cstring) > 0)
    {
        u64 rdata_start = string_builder_get_size(builder);

        string_builder_append_builder(builder, codegen.section_cstring);
        string_builder_align(builder, file_alignment, 0);

        u64 rdata_end = string_builder_get_size(builder);

        *rdata_virtual_size = (u32) (rdata_end - rdata_start);
        *rdata_virtual_address = vaddr;
        *rdata_pointer_to_raw_data = (u32) rdata_start;
        *rdata_size_of_raw_data = Align((u32) (rdata_end - rdata_start), file_alignment);

        vaddr += rdata_end - rdata_start;
        vaddr = Align(vaddr, section_alignment);
    }

    *size_of_image = (u32) vaddr;
}
