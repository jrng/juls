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

static inline void
x64_add_immediate32_unsigned_to_register(StringBuilder *builder, X64Register reg, u32 value)
{
    if (value <= 0xFF)
    {
        // TODO: do we need this 64bit extension here?
        // are the high bits set to zero if this is omitted?
        string_builder_append_u8(builder, REX_W);
        string_builder_append_u8(builder, 0x83);
        string_builder_append_u8(builder, ModRM(3, 0, reg));
        string_builder_append_u8(builder, (u8) value);
    }
    else
    {
        // TODO: do we need this 64bit extension here?
        // are the high 32bit set to zero if this is omitted?
        string_builder_append_u8(builder, REX_W);
        string_builder_append_u8(builder, 0x81);
        string_builder_append_u8(builder, ModRM(3, 0, reg));
        string_builder_append_u32le(builder, value);
    }
}

static inline void
x64_subtract_immediate32_unsigned_from_register(StringBuilder *builder, X64Register reg, u32 value)
{
    if (value <= 0xFF)
    {
        // TODO: do we need this 64bit extension here?
        // are the high bits set to zero if this is omitted?
        string_builder_append_u8(builder, REX_W);
        string_builder_append_u8(builder, 0x83);
        string_builder_append_u8(builder, ModRM(3, 5, reg));
        string_builder_append_u8(builder, (u8) value);
    }
    else
    {
        // TODO: do we need this 64bit extension here?
        // are the high 32bit set to zero if this is omitted?
        string_builder_append_u8(builder, REX_W);
        string_builder_append_u8(builder, 0x81);
        string_builder_append_u8(builder, ModRM(3, 5, reg));
        string_builder_append_u32le(builder, value);
    }
}

static inline void
x64_add_registers(StringBuilder *builder, X64Register dst_reg, X64Register src_reg, u64 size)
{
    switch (size)
    {
        case 1:
        {
            string_builder_append_u8(builder, 0x00);
            string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
        } break;

        case 2:
        {
            string_builder_append_u8(builder, 0x66);
            string_builder_append_u8(builder, 0x01);
            string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
        } break;

        case 4:
        {
            string_builder_append_u8(builder, 0x01);
            string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
        } break;

        case 8:
        {
            string_builder_append_u8(builder, REX_W);
            string_builder_append_u8(builder, 0x01);
            string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
        } break;
    }
}

static inline void
x64_subtract_registers(StringBuilder *builder, X64Register dst_reg, X64Register src_reg, u64 size)
{
    switch (size)
    {
        case 1:
        {
            string_builder_append_u8(builder, 0x28);
            string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
        } break;

        case 2:
        {
            string_builder_append_u8(builder, 0x66);
            string_builder_append_u8(builder, 0x29);
            string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
        } break;

        case 4:
        {
            string_builder_append_u8(builder, 0x29);
            string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
        } break;

        case 8:
        {
            string_builder_append_u8(builder, REX_W);
            string_builder_append_u8(builder, 0x29);
            string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
        } break;
    }
}

static inline void
x64_copy_from_stack_to_register(StringBuilder *builder, X64Register dst_reg, u64 src_stack_offset, u64 size)
{
    switch (size)
    {
        case 1:
        {
            if (src_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x8A);
                string_builder_append_u8(builder, ModRM(1, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert(src_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x8A);
                string_builder_append_u8(builder, ModRM(2, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }
        } break;

        case 2:
        {
            if (src_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert(src_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }
        } break;

        case 4:
        {
            if (src_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert(src_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }
        } break;

        case 8:
        {
            if (src_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert(src_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }
        } break;

        default:
        {
            assert(!"not allowed");
        } break;
    }
}

static inline void
x64_copy_from_register_to_stack(StringBuilder *builder, u64 dst_stack_offset, X64Register src_reg, u64 size)
{
    switch (size)
    {
        case 1:
        {
            if (dst_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x88);
                string_builder_append_u8(builder, ModRM(1, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert(dst_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x88);
                string_builder_append_u8(builder, ModRM(2, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 2:
        {
            if (dst_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert(dst_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 4:
        {
            if (dst_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert(dst_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 8:
        {
            if (dst_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert(dst_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        default:
        {
            assert(!"not allowed");
        } break;
    }
}

static inline void
x64_copy_from_stack_to_stack(StringBuilder *builder, u64 dst_stack_offset, u64 src_stack_offset, u64 size)
{
    switch (size)
    {
        case 1:
        {
            if (src_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x8A);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert(src_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x8A);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if (dst_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x88);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert(dst_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x88);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 2:
        {
            if (src_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert(src_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if (dst_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert(dst_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 4:
        {
            if (src_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert(src_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if (dst_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert(dst_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 8:
        {
            if (src_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert(src_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if (dst_stack_offset <= 0xFF)
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert(dst_stack_offset <= 0xFFFFFFFF);

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        default:
        {
            assert(!"not implemented");
        } break;
    }
}

static inline void
x64_compare_registers(StringBuilder *builder, X64Register a_reg, X64Register b_reg, u64 size)
{
    switch (size)
    {
        case 1:
        {
            string_builder_append_u8(builder, 0x38);
            string_builder_append_u8(builder, ModRM(3, b_reg, a_reg));
        } break;

        case 2:
        {
            string_builder_append_u8(builder, 0x66);
            string_builder_append_u8(builder, 0x39);
            string_builder_append_u8(builder, ModRM(3, b_reg, a_reg));
        } break;

        case 4:
        {
            string_builder_append_u8(builder, 0x39);
            string_builder_append_u8(builder, ModRM(3, b_reg, a_reg));
        } break;

        case 8:
        {
            string_builder_append_u8(builder, REX_W);
            string_builder_append_u8(builder, 0x39);
            string_builder_append_u8(builder, ModRM(3, b_reg, a_reg));
        } break;
    }
}

static inline void
x64_setcc(StringBuilder *builder, u8 compare_func, u64 dst_stack_offset)
{
    string_builder_append_u8(builder, 0x0F);
    string_builder_append_u8(builder, compare_func);
    string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
    string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
    string_builder_append_u32le(builder, (u32) dst_stack_offset);
}

static inline void
x64_compare_al_to_zero(StringBuilder *builder, X64Register reg)
{
    string_builder_append_u8(builder, 0x3C);
    string_builder_append_u8(builder, 0x00);
}

static void
x64_emit_expression(Parser *parser, Codegen *codegen, Ast *expr, JulsPlatform target_platform)
{
    switch (expr->kind)
    {
        case AST_KIND_LITERAL_BOOLEAN:
        {
            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            assert(stack_size <= 0xFFFFFFFF);
            x64_subtract_immediate32_unsigned_from_register(&codegen->section_text, X64_RSP, (u32) stack_size);
            parser->current_stack_offset += stack_size;

            x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, expr->_bool ? 1 : 0);
            x64_copy_from_register_to_stack(&codegen->section_text, 0, X64_RAX, datatype->size);
        } break;

        case AST_KIND_LITERAL_INTEGER:
        {
            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            if ((datatype->flags & DATATYPE_FLAG_UNSIGNED) ||
                (expr->_s64 >= 0))
            {
                if (expr->_u64 <= 0xFFFFFFFF)
                {
                    x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, expr->_u32);
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
                    x64_move_immediate32_signed_into_register(&codegen->section_text, X64_RAX, expr->_u32);
                }
                else
                {
                    assert(!"not implemented");
                }
            }

            u64 stack_size = datatype->size;

            assert(stack_size <= 0xFFFFFFFF);
            x64_subtract_immediate32_unsigned_from_register(&codegen->section_text, X64_RSP, (u32) stack_size);
            parser->current_stack_offset += stack_size;

            x64_copy_from_register_to_stack(&codegen->section_text, 0, X64_RAX, datatype->size);
        } break;

        case AST_KIND_IDENTIFIER:
        {
            assert(expr->decl);

            Ast *decl = expr->decl;

            assert(decl->stack_offset > 0);

            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            assert(stack_size <= 0xFFFFFFFF);
            x64_subtract_immediate32_unsigned_from_register(&codegen->section_text, X64_RSP, (u32) stack_size);
            parser->current_stack_offset += stack_size;

            x64_copy_from_stack_to_stack(&codegen->section_text, 0, parser->current_stack_offset - decl->stack_offset, datatype->size);
        } break;

        case AST_KIND_EXPRESSION_EQUAL:
        case AST_KIND_EXPRESSION_NOT_EQUAL:
        case AST_KIND_EXPRESSION_COMPARE_LESS:
        case AST_KIND_EXPRESSION_COMPARE_GREATER:
        case AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL:
        case AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL:
        {
            x64_emit_expression(parser, codegen, expr->left_expr, target_platform);
            x64_emit_expression(parser, codegen, expr->right_expr, target_platform);

            Datatype *left_datatype = get_datatype(&parser->datatypes, expr->left_expr->type_id);
            Datatype *right_datatype = get_datatype(&parser->datatypes, expr->right_expr->type_id);

            u64 max_datatype_size = left_datatype->size;

            if (right_datatype->size > max_datatype_size)
            {
                max_datatype_size = right_datatype->size;
            }

            u64 left_stack_size = left_datatype->size;
            u64 right_stack_size = right_datatype->size;
            u64 total_stack_size = left_stack_size + right_stack_size;

            assert(expr->type_id == parser->basetype_bool);
            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            x64_copy_from_stack_to_register(&codegen->section_text, X64_RBX, 0, right_datatype->size);
            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, right_stack_size, left_datatype->size);

            // TODO: maybe sign extend arguments

            assert(stack_size <= 0xFFFFFFFF);
            assert(total_stack_size <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) (total_stack_size - stack_size));
            parser->current_stack_offset -= total_stack_size - stack_size;

            x64_compare_registers(&codegen->section_text, X64_RAX, X64_RBX, max_datatype_size);

            bool is_signed = true; // TODO:

            if (is_signed)
            {
                if (expr->kind == AST_KIND_EXPRESSION_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x94, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_NOT_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x95, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS)
                {
                    x64_setcc(&codegen->section_text, 0x9C, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER)
                {
                    x64_setcc(&codegen->section_text, 0x9F, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x9E, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x9D, 0);
                }
            }
            else
            {
                if (expr->kind == AST_KIND_EXPRESSION_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x94, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_NOT_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x95, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS)
                {
                    x64_setcc(&codegen->section_text, 0x92, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER)
                {
                    x64_setcc(&codegen->section_text, 0x97, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x96, 0);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x93, 0);
                }
            }
        } break;

        case AST_KIND_EXPRESSION_BINOP_ADD:
        case AST_KIND_EXPRESSION_BINOP_MINUS:
        {
            x64_emit_expression(parser, codegen, expr->left_expr, target_platform);
            x64_emit_expression(parser, codegen, expr->right_expr, target_platform);

            Datatype *left_datatype = get_datatype(&parser->datatypes, expr->left_expr->type_id);
            Datatype *right_datatype = get_datatype(&parser->datatypes, expr->right_expr->type_id);

            u64 left_stack_size = left_datatype->size;
            u64 right_stack_size = right_datatype->size;
            u64 total_stack_size = left_stack_size + right_stack_size;

            assert(expr->type_id);
            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            x64_copy_from_stack_to_register(&codegen->section_text, X64_RBX, 0, right_datatype->size);
            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, right_stack_size, left_datatype->size);

            // TODO: maybe sign extend arguments

            assert(stack_size <= 0xFFFFFFFF);
            assert(total_stack_size <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) (total_stack_size - stack_size));
            parser->current_stack_offset -= total_stack_size - stack_size;

            if (expr->kind == AST_KIND_EXPRESSION_BINOP_ADD)
            {
                x64_add_registers(&codegen->section_text, X64_RAX, X64_RBX, datatype->size);
            }
            else
            {
                assert(expr->kind == AST_KIND_EXPRESSION_BINOP_MINUS);
                x64_subtract_registers(&codegen->section_text, X64_RAX, X64_RBX, datatype->size);
            }

            x64_copy_from_register_to_stack(&codegen->section_text, 0, X64_RAX, datatype->size);
        } break;

        case AST_KIND_FUNCTION_CALL:
        {
            assert(expr->left_expr);

            Ast *left = expr->left_expr;

            if ((left->kind == AST_KIND_IDENTIFIER) && strings_are_equal(left->name, S("exit")))
            {
            }
            else if (left->kind == AST_KIND_IDENTIFIER)
            {
                assert(expr->decl);

                Datatype *return_type = get_datatype(&parser->datatypes, expr->decl->type_id);
                u64 return_type_stack_size = return_type->size;

                assert(return_type_stack_size <= 0xFFFFFFFF);
                x64_subtract_immediate32_unsigned_from_register(&codegen->section_text, X64_RSP, (u32) return_type_stack_size);
                parser->current_stack_offset += return_type_stack_size;
            }
            else
            {
                assert(!"not implemented");
            }

            u64 arguments_stack_size = 0;

            For(argument, expr->children.first)
            {
                // TODO: does the size match the expression?
                x64_emit_expression(parser, codegen, argument, target_platform);

                Datatype *datatype = get_datatype(&parser->datatypes, argument->type_id);

                u64 stack_size = datatype->size;
                arguments_stack_size += stack_size;
            }

            if ((left->kind == AST_KIND_IDENTIFIER) && strings_are_equal(left->name, S("exit")))
            {
                if ((target_platform == JulsPlatformAndroid) ||
                    (target_platform == JulsPlatformLinux))
                {
                    x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 60);
                    x64_move_indirect_into_register(&codegen->section_text, X64_RDI, X64_RSP, 0);
                    x64_syscall(&codegen->section_text);
                }
                else if (target_platform == JulsPlatformMacOs)
                {
                    x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 0x02000001);
                    x64_move_indirect_into_register(&codegen->section_text, X64_RDI, X64_RSP, 0);
                    x64_syscall(&codegen->section_text);
                }
            }
            else if (left->kind == AST_KIND_IDENTIFIER)
            {
                assert(expr->decl);
                assert(expr->decl->address != S64MAX);

                string_builder_append_u8(&codegen->section_text, 0xE8);
                s64 jump_offset = string_builder_get_size(&codegen->section_text) + 4;
                string_builder_append_u32le(&codegen->section_text, (u32) (expr->decl->address - jump_offset));
            }
            else
            {
                assert(!"not implemented");
            }

            assert(arguments_stack_size <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) arguments_stack_size);
            parser->current_stack_offset -= arguments_stack_size;
        } break;

        case AST_KIND_ASSIGN:
        case AST_KIND_PLUS_ASSIGN:
        case AST_KIND_MINUS_ASSIGN:
        // case AST_KIND_MUL_ASSIGN:
        // case AST_KIND_DIV_ASSIGN:
        // case AST_KIND_OR_ASSIGN:
        // case AST_KIND_AND_ASSIGN:
        // case AST_KIND_XOR_ASSIGN:
        {
            assert(expr->decl);

            Ast *decl = expr->decl;

            assert(decl->stack_offset > 0);

            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);
            Datatype *right_datatype = get_datatype(&parser->datatypes, expr->right_expr->type_id);

            u64 stack_size = datatype->size;
            u64 right_stack_size = right_datatype->size;

            if (expr->kind != AST_KIND_ASSIGN)
            {
                assert(stack_size <= 0xFFFFFFFF);
                x64_subtract_immediate32_unsigned_from_register(&codegen->section_text, X64_RSP, (u32) stack_size);
                parser->current_stack_offset += stack_size;

                x64_copy_from_stack_to_stack(&codegen->section_text, 0, parser->current_stack_offset - decl->stack_offset, datatype->size);
            }

            x64_emit_expression(parser, codegen, expr->right_expr, target_platform);

            if (expr->kind != AST_KIND_ASSIGN)
            {
                x64_copy_from_stack_to_register(&codegen->section_text, X64_RBX, 0, right_datatype->size);
                x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, right_stack_size, datatype->size);

                // TODO: maybe sign extend arguments

                assert(right_stack_size <= 0xFFFFFFFF);
                x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) right_stack_size);
                parser->current_stack_offset -= right_stack_size;

                if (expr->kind == AST_KIND_PLUS_ASSIGN)
                {
                    x64_add_registers(&codegen->section_text, X64_RAX, X64_RBX, datatype->size);
                }
                else
                {
                    assert(expr->kind == AST_KIND_MINUS_ASSIGN);
                    x64_subtract_registers(&codegen->section_text, X64_RAX, X64_RBX, datatype->size);
                }

                x64_copy_from_register_to_stack(&codegen->section_text, 0, X64_RAX, datatype->size);
            }

            // TODO: does the size match the expression?
            x64_copy_from_stack_to_stack(&codegen->section_text, parser->current_stack_offset - decl->stack_offset, 0, datatype->size);
        } break;

        default:
        {
            printf("kind = %u\n", expr->kind);
            assert(!"expression not supported");
        } break;
    }
}

static void
x64_emit_statement(Parser *parser, Codegen *codegen, Ast *statement, JulsPlatform target_platform,
                   Datatype *return_type, u64 return_type_stack_size)
{
    switch (statement->kind)
    {
        case AST_KIND_VARIABLE_DECLARATION:
        {
            Datatype *datatype = get_datatype(&parser->datatypes, statement->type_id);

            u64 stack_size = datatype->size;

            assert(stack_size <= 0xFFFFFFFF);
            x64_subtract_immediate32_unsigned_from_register(&codegen->section_text, X64_RSP, (u32) stack_size);
            parser->current_stack_offset += stack_size;
            statement->stack_offset = parser->current_stack_offset;

            parser->stack_allocated[parser->stack_allocated_index] += stack_size;

            if (statement->right_expr)
            {
                x64_emit_expression(parser, codegen, statement->right_expr, target_platform);

                // TODO: does the size match the expression?
                x64_copy_from_stack_to_stack(&codegen->section_text, parser->current_stack_offset - statement->stack_offset, 0, datatype->size);

                assert(stack_size <= 0xFFFFFFFF);
                x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) stack_size);
                parser->current_stack_offset -= stack_size;
            }
        } break;

        case AST_KIND_IF:
        {
            x64_emit_expression(parser, codegen, statement->left_expr, target_platform);

            assert(statement->left_expr->type_id == parser->basetype_bool);
            Datatype *datatype = get_datatype(&parser->datatypes, statement->left_expr->type_id);

            u64 stack_size = datatype->size;

            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, 0, datatype->size);

            assert(stack_size <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) stack_size);
            parser->current_stack_offset -= stack_size;

            x64_compare_al_to_zero(&codegen->section_text, X64_RAX);

            // JE
            string_builder_append_u8(&codegen->section_text, 0x0F);
            string_builder_append_u8(&codegen->section_text, 0x84);
            s32 *else_patch = string_builder_append_size(&codegen->section_text, 4);
            s64 else_offset = string_builder_get_size(&codegen->section_text);

            Ast *if_code = statement->children.first;
            Ast *else_code = 0;

            if (statement->children.first != statement->children.last)
            {
                else_code = statement->children.last;
            }

            x64_emit_statement(parser, codegen, if_code, target_platform, return_type, return_type_stack_size);

            s32 *end_patch = 0;

            if (else_code)
            {
                // JMP
                string_builder_append_u8(&codegen->section_text, 0xE9);
                end_patch = string_builder_append_size(&codegen->section_text, 4);
            }

            s64 else_target = string_builder_get_size(&codegen->section_text);
            *else_patch = (s32) (else_target - else_offset);

            s64 end_offset = else_target;

            if (else_code)
            {
                x64_emit_statement(parser, codegen, else_code, target_platform, return_type, return_type_stack_size);

                s64 end_target = string_builder_get_size(&codegen->section_text);
                *end_patch = (s32) (end_target - end_offset);
            }
        } break;

        case AST_KIND_FOR:
        {
            push_scope(parser);

            x64_emit_statement(parser, codegen, statement->decl, target_platform, return_type, return_type_stack_size);

            s64 start_target = string_builder_get_size(&codegen->section_text);

            x64_emit_expression(parser, codegen, statement->left_expr, target_platform);

            assert(statement->left_expr->type_id == parser->basetype_bool);
            Datatype *datatype = get_datatype(&parser->datatypes, statement->left_expr->type_id);

            u64 stack_size = datatype->size;

            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, 0, datatype->size);

            assert(stack_size <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) stack_size);
            parser->current_stack_offset -= stack_size;

            x64_compare_al_to_zero(&codegen->section_text, X64_RAX);

            // JE
            string_builder_append_u8(&codegen->section_text, 0x0F);
            string_builder_append_u8(&codegen->section_text, 0x84);
            s32 *end_patch = string_builder_append_size(&codegen->section_text, 4);
            s64 end_offset = string_builder_get_size(&codegen->section_text);

            x64_emit_statement(parser, codegen, statement->children.first, target_platform, return_type, return_type_stack_size);
            x64_emit_expression(parser, codegen, statement->right_expr, target_platform);

            Datatype *right_datatype = get_datatype(&parser->datatypes, statement->right_expr->type_id);

            u64 right_stack_size = right_datatype->size;

            assert(right_stack_size <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) right_stack_size);
            parser->current_stack_offset -= right_stack_size;

            string_builder_append_u8(&codegen->section_text, 0xE9);
            s32 *start_patch = string_builder_append_size(&codegen->section_text, 4);
            s64 start_offset = string_builder_get_size(&codegen->section_text);
            *start_patch = (start_target - start_offset);

            s64 end_target = string_builder_get_size(&codegen->section_text);
            *end_patch = (s32) (end_target - end_offset);

            u64 stack_allocated = parser->stack_allocated[parser->stack_allocated_index];
            pop_scope(parser);

            assert(stack_allocated <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) stack_allocated);
            parser->current_stack_offset -= stack_allocated;
        } break;

        case AST_KIND_RETURN:
        {
            assert(statement->left_expr);

            x64_emit_expression(parser, codegen, statement->left_expr, target_platform);

            x64_copy_from_stack_to_stack(&codegen->section_text, parser->current_stack_offset - return_type_stack_size, 0, return_type->size);

            assert(return_type_stack_size <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) return_type_stack_size);
            parser->current_stack_offset -= return_type_stack_size;

            u64 stack_allocated = 0;

            while (parser->stack_allocated_index >= 0)
            {
                stack_allocated += parser->stack_allocated[parser->stack_allocated_index];
                pop_scope(parser);
            }

            assert(stack_allocated <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) stack_allocated);

            x64_ret(&codegen->section_text);
        } break;

        case AST_KIND_BLOCK:
        {
            push_scope(parser);

            For(stmt, statement->children.first)
            {
                x64_emit_statement(parser, codegen, stmt, target_platform, return_type, return_type_stack_size);
            }

            u64 stack_allocated = parser->stack_allocated[parser->stack_allocated_index];
            pop_scope(parser);

            assert(stack_allocated <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) stack_allocated);
            parser->current_stack_offset -= stack_allocated;
        } break;

        default:
        {
            x64_emit_expression(parser, codegen, statement, target_platform);

            Datatype *datatype = get_datatype(&parser->datatypes, statement->type_id);

            u64 stack_size = datatype->size;

            assert(stack_size <= 0xFFFFFFFF);
            x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) stack_size);
            parser->current_stack_offset -= stack_size;
        } break;
    }
}

static void
x64_emit_function(Parser *parser, Codegen *codegen, Ast *func, JulsPlatform target_platform)
{
    assert(func->kind == AST_KIND_FUNCTION_DECLARATION);

    func->address = string_builder_get_size(&codegen->section_text);

    parser->current_stack_offset = 0;
    parser->stack_allocated_index = -1;

    Datatype *return_type = get_datatype(&parser->datatypes, func->type_id);
    u64 return_type_stack_size = return_type->size;
    parser->current_stack_offset += return_type_stack_size;

    ForReversed(parameter, func->parameters.first)
    {
        Datatype *datatype = get_datatype(&parser->datatypes, parameter->type_id);

        u64 stack_size = datatype->size;

        parser->current_stack_offset += stack_size;
        parameter->stack_offset = parser->current_stack_offset;
    }

    // that's the call return address
    parser->current_stack_offset += 8;

    push_scope(parser);

    For(statement, func->children.first)
    {
        x64_emit_statement(parser, codegen, statement, target_platform, return_type, return_type_stack_size);
    }

    if (!func->type_def)
    {
        assert(parser->stack_allocated_index == 0);

        u64 stack_allocated = parser->stack_allocated[parser->stack_allocated_index];
        pop_scope(parser);

        assert(stack_allocated <= 0xFFFFFFFF);
        x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) stack_allocated);

        x64_ret(&codegen->section_text);
    }
}

static void
generate_x64(Parser *parser, Codegen *codegen, SymbolTable *symbol_table, JulsPlatform target_platform)
{
    String entry_point_name = S("main");

    u64 jump_location = 0;
    s32 *jump_patch = 0;

    u64 _start_offset = string_builder_get_size(&codegen->section_text);

    if ((target_platform == JulsPlatformAndroid) ||
        (target_platform == JulsPlatformLinux))
    {
        // call main
        string_builder_append_u8(&codegen->section_text, 0xE8);
        jump_patch = string_builder_append_size(&codegen->section_text, 4);
        jump_location = string_builder_get_size(&codegen->section_text);

        // mov rax, 231
        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 231);

        // mov rdi, 0
        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RDI, 0);

        // syscall
        x64_syscall(&codegen->section_text);
    }
    else if (target_platform == JulsPlatformMacOs)
    {
        // call main
        string_builder_append_u8(&codegen->section_text, 0xE8);
        jump_patch = string_builder_append_size(&codegen->section_text, 4);
        jump_location = string_builder_get_size(&codegen->section_text);

        // mov rax, 0x02000001
        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 0x02000001);

        // mov rdi, 0
        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RDI, 0);

        // syscall
        x64_syscall(&codegen->section_text);
    }

    u64 _start_size = string_builder_get_size(&codegen->section_text) - _start_offset;

    array_append(symbol_table, ((SymbolEntry) { .name = S("_start"), .offset = _start_offset, .size = _start_size }));

    u64 jump_target = 0;

    For(decl, parser->global_declarations.children.first)
    {
        if (decl->kind == AST_KIND_FUNCTION_DECLARATION)
        {
            u64 offset = string_builder_get_size(&codegen->section_text);

            if (strings_are_equal(entry_point_name, decl->name))
            {
                jump_target = offset;
            }

            x64_emit_function(parser, codegen, decl, target_platform);

            u64 size = string_builder_get_size(&codegen->section_text) - offset;

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
