static inline void
x64_syscall(StringBuilder *builder)
{
    string_builder_append_u16le(builder, 0x050F);
}

static inline void
x64_ret(StringBuilder *builder)
{
    string_builder_append_u8(builder, 0xC3);
}

static void
generate_x64(Parser *parser, StringBuilder *code, SymbolTable *symbol_table, JulsPlatform target_platform)
{
    String entry_point_name = S("main");

    u64 jump_location = 0;
    s32 *jump_patch = 0;

    u64 _start_offset = string_builder_get_size(code);

    if ((target_platform == JulsPlatformAndroid) ||
        (target_platform == JulsPlatformLinux))
    {
        // call main
        string_builder_append_u8(code, 0xE8);
        jump_patch = string_builder_append_size(code, 4);
        jump_location = string_builder_get_size(code);

        // mov rax, 231
        string_builder_append_u8(code, 0x48);
        string_builder_append_u8(code, 0xC7);
        string_builder_append_u8(code, 0xC0);
        string_builder_append_u32le(code, 231);

        // mov rdi, 42
        string_builder_append_u8(code, 0x48);
        string_builder_append_u8(code, 0xC7);
        string_builder_append_u8(code, 0xC7);
        string_builder_append_u32le(code, 42);

        // syscall
        x64_syscall(code);
    }
    else if (target_platform == JulsPlatformMacOs)
    {
        // call main
        string_builder_append_u8(code, 0xE8);
        jump_patch = string_builder_append_size(code, 4);

        jump_location = string_builder_get_size(code);

        // mov rax, 0x02000001
        string_builder_append_u8(code, 0x48);
        string_builder_append_u8(code, 0xC7);
        string_builder_append_u8(code, 0xC0);
        string_builder_append_u32le(code, 0x02000001);

        // mov rdi, 42
        string_builder_append_u8(code, 0x48);
        string_builder_append_u8(code, 0xC7);
        string_builder_append_u8(code, 0xC7);
        string_builder_append_u32le(code, 42);

        // syscall
        x64_syscall(code);
    }

    u64 _start_size = string_builder_get_size(code) - _start_offset;

    array_append(symbol_table, ((SymbolEntry) { .name = S("_start"), .offset = _start_offset, .size = _start_size }));

    u64 jump_target = 0;

    Ast *elem = parser->global_declarations.first;

    while (elem)
    {
        if (elem->type == AST_TYPE_FUNCTION_DECLARATION)
        {
            u64 offset = string_builder_get_size(code);

            if (strings_are_equal(entry_point_name, elem->name))
            {
                jump_target = offset;
            }

            x64_ret(code);

            u64 size = string_builder_get_size(code) - offset;

            array_append(symbol_table, ((SymbolEntry) { .name = elem->name, .offset = offset, .size = size }));
        }

        elem = elem->next;
    }

    if (jump_target > 0)
    {
        *jump_patch = (s32) (jump_target - jump_location);
    }
    else
    {
        fprintf(stderr, "error: there is no entry point '%.*s'\n", (int) entry_point_name.count, entry_point_name.data);
    }
}
