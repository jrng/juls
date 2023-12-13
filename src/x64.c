#define REX_W 0x48
#define ModRM(mode, reg, rm) ((((mode) & 0x3) << 6) | (((reg) & 0x7) << 3) | ((rm) & 0x7))
#define SIB(scale, index, base) ((((scale) & 0x3) << 6) | (((index) & 0x7) << 3) | ((base) & 0x7))

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
x64_move_immediate32_signed_into_register(StringBuilder *builder, X64Register reg, u32 value)
{
    string_builder_append_u8(builder, REX_W);
    string_builder_append_u8(builder, 0xC7);
    string_builder_append_u8(builder, ModRM(3, 0, reg));
    string_builder_append_u32le(builder, value);
}

static inline void
x64_move_immediate32_unsigned_into_register(StringBuilder *builder, X64Register reg, u32 value)
{
    string_builder_append_u8(builder, 0xB8 | reg);
    string_builder_append_u32le(builder, value);
}

static inline void
x64_push_register(StringBuilder *builder, X64Register reg)
{
    string_builder_append_u8(builder, 0x50 | reg);
}

static inline void
x64_pop_register(StringBuilder *builder, X64Register reg)
{
    string_builder_append_u8(builder, 0x58 | reg);
}

static inline void
x64_move_indirect_into_register(StringBuilder *builder, X64Register dst, X64Register base_reg, u8 offset)
{
    string_builder_append_u8(builder, REX_W);
    string_builder_append_u8(builder, 0x8B);
    string_builder_append_u8(builder, ModRM(1, dst, base_reg));
    string_builder_append_u8(builder, SIB(0, base_reg, base_reg));
    string_builder_append_u8(builder, offset);
}

static void
x64_emit_expression(Parser *parser, StringBuilder *code, Ast *expr)
{
    switch (expr->kind)
    {
        case AST_KIND_LITERAL_INTEGER:
        {
            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);
            assert(datatype->size == 8);

            if ((datatype->flags & DATATYPE_FLAG_UNSIGNED) ||
                (expr->_s64 >= 0))
            {
                if (expr->_u64 <= 0xFFFFFFFF)
                {
                    x64_move_immediate32_unsigned_into_register(code, X64_RAX, expr->_u32);
                }
                else
                {
                    assert(!"not implemented");
                }
            }
            else
            {
                if ((expr->_s64 >= S32MIN) && (expr->_s64 <= S32MAX))
                {
                    x64_move_immediate32_signed_into_register(code, X64_RAX, expr->_u32);
                }
                else
                {
                    assert(!"not implemented");
                }
            }

            x64_push_register(code, X64_RAX);
        } break;

        default:
        {
            assert(!"expression not supported");
        } break;
    }
}

static void
x64_emit_function(Parser *parser, StringBuilder *code, Ast *func, JulsPlatform target_platform)
{
    assert(func->kind == AST_KIND_FUNCTION_DECLARATION);

    For(statement, func->children.first)
    {
        switch (statement->kind)
        {
            case AST_KIND_VARIABLE_DECLARATION:
            {
            } break;

            case AST_KIND_FUNCTION_CALL:
            {
                assert(statement->left_expr);

                Ast *left = statement->left_expr;

                if ((left->kind == AST_KIND_IDENTIFIER) && strings_are_equal(left->name, S("exit")))
                {
                    For(argument, statement->children.first)
                    {
                        x64_emit_expression(parser, code, argument);
                    }

                    if ((target_platform == JulsPlatformAndroid) ||
                        (target_platform == JulsPlatformLinux))
                    {
                        x64_move_immediate32_unsigned_into_register(code, X64_RAX, 60);
                        x64_move_indirect_into_register(code, X64_RDI, X64_RSP, 0);
                        x64_syscall(code);
                    }
                    else if (target_platform == JulsPlatformMacOs)
                    {
                        x64_move_immediate32_unsigned_into_register(code, X64_RAX, 0x02000001);
                        x64_move_indirect_into_register(code, X64_RDI, X64_RSP, 0);
                        x64_syscall(code);
                    }

                    // TODO: just increment stack pointer
                    x64_pop_register(code, X64_RAX);
                }
                else
                {
                    assert(!"not implemented");
                }
            } break;

            default:
            {
                fprintf(stderr, "error: ast kind %u not supported in function declaration\n", statement->kind);
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
        x64_move_immediate32_unsigned_into_register(code, X64_RAX, 231);

        // mov rdi, 42
        x64_move_immediate32_unsigned_into_register(code, X64_RDI, 42);

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
        x64_move_immediate32_unsigned_into_register(code, X64_RAX, 0x02000001);

        // mov rdi, 42
        x64_move_immediate32_unsigned_into_register(code, X64_RDI, 42);

        // syscall
        x64_syscall(code);
    }

    u64 _start_size = string_builder_get_size(code) - _start_offset;

    array_append(symbol_table, ((SymbolEntry) { .name = S("_start"), .offset = _start_offset, .size = _start_size }));

    u64 jump_target = 0;

    For(decl, parser->global_declarations.children.first)
    {
        if (decl->kind == AST_KIND_FUNCTION_DECLARATION)
        {
            u64 offset = string_builder_get_size(code);

            if (strings_are_equal(entry_point_name, decl->name))
            {
                jump_target = offset;
            }

            x64_emit_function(parser, code, decl, target_platform);

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
