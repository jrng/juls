#define REX_W 0x48
#define ModRM(mode, reg, rm) ((((mode) & 0x3) << 6) | (((reg) & 0x7) << 3) | ((rm) & 0x7))

typedef enum
{
    X64_RAX = 0,
    X64_RCX = 1,
    X64_RDX = 2,
    X64_RBX = 3,
    X64_RSP = 4,
    X64_RBP = 5,
    X64_RSI = 6,
    X64_RDI = 7,
} X64Register;

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

static inline void
x64_move_immediate32_into_register(StringBuilder *builder, X64Register reg, u32 value)
{
    string_builder_append_u8(builder, REX_W);
    string_builder_append_u8(builder, 0xC7);
    string_builder_append_u8(builder, ModRM(3, 0, reg));
    string_builder_append_u32le(builder, value);
}

static void
x64_generate_function(StringBuilder *code, Ast *func, JulsPlatform target_platform)
{
    assert(func->type == AST_TYPE_FUNCTION_DECLARATION);

    For(statement, func->children.first)
    {
        switch (statement->type)
        {
            case AST_TYPE_VARIABLE_DECLARATION:
            {
            } break;

            case AST_TYPE_FUNCTION_CALL:
            {
                assert(statement->left_expr);

                Ast *left = statement->left_expr;

                if ((left->type == AST_TYPE_IDENTIFIER) && strings_are_equal(left->name, S("exit")))
                {
                    // TODO: get return code from expression
                    if ((target_platform == JulsPlatformAndroid) ||
                        (target_platform == JulsPlatformLinux))
                    {
                        x64_move_immediate32_into_register(code, X64_RAX, 60);
                        x64_move_immediate32_into_register(code, X64_RDI, 123);
                        x64_syscall(code);
                    }
                    else if (target_platform == JulsPlatformMacOs)
                    {
                        x64_move_immediate32_into_register(code, X64_RAX, 0x02000001);
                        x64_move_immediate32_into_register(code, X64_RDI, 123);
                        x64_syscall(code);
                    }
                }
                else
                {
                    assert(!"not implemented");
                }
            } break;

            default:
            {
                fprintf(stderr, "error: ast type %u not supported in function declaration\n", statement->type);
            } break;
        }
    }
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
        x64_move_immediate32_into_register(code, X64_RAX, 231);

        // mov rdi, 42
        x64_move_immediate32_into_register(code, X64_RDI, 42);

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
        x64_move_immediate32_into_register(code, X64_RAX, 0x02000001);

        // mov rdi, 42
        x64_move_immediate32_into_register(code, X64_RDI, 42);

        // syscall
        x64_syscall(code);
    }

    u64 _start_size = string_builder_get_size(code) - _start_offset;

    array_append(symbol_table, ((SymbolEntry) { .name = S("_start"), .offset = _start_offset, .size = _start_size }));

    u64 jump_target = 0;

    Ast *decl = parser->global_declarations.first;

    For(decl, parser->global_declarations.first)
    {
        if (decl->type == AST_TYPE_FUNCTION_DECLARATION)
        {
            u64 offset = string_builder_get_size(code);

            if (strings_are_equal(entry_point_name, decl->name))
            {
                jump_target = offset;
            }

            x64_generate_function(code, decl, target_platform);

            x64_ret(code);

            u64 size = string_builder_get_size(code) - offset;

            array_append(symbol_table, ((SymbolEntry) { .name = decl->name, .offset = offset, .size = size }));
        }
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
