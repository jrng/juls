static void
generate_pe(StringBuilder *builder, Codegen codegen, SymbolTable symbol_table, JulsArchitecture target_architecture)
{
    // DOS header

    string_builder_append_string(builder, S("MZ"));

    for (s32 i = 0; i < 58; i += 1)
    {
        string_builder_append_u8(builder, 0);
    }

    string_builder_append_u32le(builder, string_builder_get_size(builder) + 4);

    // COFF header

    string_builder_append_string(builder, S("PE\0\0")); // Signature

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

    string_builder_append_u16le(builder, 0);      // NumberOfSections
    string_builder_append_u32le(builder, 0);      // TimeDateStamp
    string_builder_append_u32le(builder, 0);      // PointerToSymbolTable
    string_builder_append_u32le(builder, 0);      // NumberOfSymbols
    u16 *size_of_optional_header = string_builder_append_size(builder, 2); // SizeOfOptionalHeader
    string_builder_append_u16le(builder, 0x0022); // Characteristics

    // optional header

    u64 optional_header_offset = string_builder_get_size(builder);

    string_builder_append_u16le(builder, 0x020B); // Magic
    string_builder_append_u8(builder, 0); // MajorLinkerVersion
    string_builder_append_u8(builder, 0); // MinorLinkerVersion
    string_builder_append_u32le(builder, 0); // SizeOfCode
    string_builder_append_u32le(builder, 0); // SizeOfInitializedData
    string_builder_append_u32le(builder, 0); // SizeOfUninitializedData
    string_builder_append_u32le(builder, 0); // AddressOfEntryPoint
    string_builder_append_u32le(builder, 0); // BaseOfCode

    string_builder_append_u64le(builder, 0); // ImageBase
    string_builder_append_u32le(builder, 0); // SectionAlignment
    string_builder_append_u32le(builder, 0); // FileAlignment
    string_builder_append_u16le(builder, 0); // MajorOperatingSystemVersion
    string_builder_append_u16le(builder, 0); // MinorOperatingSystemVersion
    string_builder_append_u16le(builder, 0); // MajorImageVersion
    string_builder_append_u16le(builder, 0); // MinorImageVersion
    string_builder_append_u16le(builder, 0); // MajorSubsystemVersion
    string_builder_append_u16le(builder, 0); // MinorSubsystemVersion
    string_builder_append_u32le(builder, 0); // Win32VersionValue
    string_builder_append_u32le(builder, 0); // SizeOfImage
    string_builder_append_u32le(builder, 0); // SizeOfHeaders
    string_builder_append_u32le(builder, 0); // CheckSum
    string_builder_append_u16le(builder, 3); // Subsystem
    string_builder_append_u16le(builder, 0); // DllCharacteristics
    string_builder_append_u64le(builder, 2048); // SizeOfStackReserve
    string_builder_append_u64le(builder, 2048); // SizeOfStackCommit
    string_builder_append_u64le(builder, 0); // SizeOfHeapReserve
    string_builder_append_u64le(builder, 0); // SizeOfHeapCommit
    string_builder_append_u32le(builder, 0); // LoaderFlags
    string_builder_append_u32le(builder, 0); // NumberOfRvaAndSize

    *size_of_optional_header = (u16) (string_builder_get_size(builder) - optional_header_offset); // this needs to be little-endian
}
