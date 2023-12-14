typedef enum
{
    ARM64_R0  = 0,
    ARM64_R1  = 1,
    ARM64_R2  = 2,
    ARM64_R3  = 3,
    ARM64_R4  = 4,
    ARM64_R5  = 5,
    ARM64_R6  = 6,
    ARM64_R7  = 7,
    ARM64_R8  = 8,
    ARM64_R9  = 9,
    ARM64_R10 = 10,
    ARM64_R11 = 11,
    ARM64_R12 = 12,
    ARM64_R13 = 13,
    ARM64_R14 = 14,
    ARM64_R15 = 15,
    ARM64_R16 = 16,
    ARM64_R17 = 17,
    ARM64_R18 = 18,
    ARM64_R19 = 19,
    ARM64_R20 = 20,
    ARM64_R21 = 21,
    ARM64_R22 = 22,
    ARM64_R23 = 23,
    ARM64_R24 = 24,
    ARM64_R25 = 25,
    ARM64_R26 = 26,
    ARM64_R27 = 27,
    ARM64_R28 = 28,
    ARM64_R29 = 29,
    ARM64_R30 = 30,
    ARM64_SP  = 31,
} Arm64Register;

static inline void
arm64_svc(StringBuilder *builder, u16 arg)
{
    u32 inst = 0xD4000001 | ((u32) arg << 5);
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_bl(StringBuilder *builder, s32 offset)
{
    u32 inst = 0x94000000 | ((u32) offset & 0x3FFFFFF);
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_move_immediate16(StringBuilder *builder, Arm64Register reg, u16 value)
{
    u32 inst = 0xD2800000 | ((u32) value << 5) | reg;
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_store_register(StringBuilder *builder, Arm64Register store_reg, Arm64Register base_reg, s16 offset)
{
    u32 inst = 0xF8000C00 | (((u32) offset & 0x1FF) << 12) | ((u32) base_reg << 5) | store_reg;
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_load_register(StringBuilder *builder, Arm64Register load_reg, Arm64Register base_reg, s16 offset)
{
    u32 inst = 0xF8400400 | (((u32) offset & 0x1FF) << 12) | ((u32) base_reg << 5) | load_reg;
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_ret(StringBuilder *builder)
{
    u32 inst = 0xD65F0000 | ((u32) ARM64_R30 << 5);
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_push_register(StringBuilder *builder, Arm64Register reg)
{
    arm64_store_register(builder, reg, ARM64_SP, -16);
}

static inline void
arm64_pop_register(StringBuilder *builder, Arm64Register reg)
{
    arm64_load_register(builder, reg, ARM64_SP, 16);
}

static inline void
arm64_move_indirect_into_register(StringBuilder *builder, Arm64Register dst, Arm64Register base_reg, u16 offset)
{
    assert(!(offset & 7));

    // LDR (immediate)
    u32 inst = 0xF9400000 | (((u32) (offset >> 3) & 0xFFF) << 10) | ((u32) base_reg << 5) | dst;
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_add_immediate12(StringBuilder *builder, Arm64Register dst_reg, Arm64Register src_reg, u16 value)
{
    assert(value <= 0xFFF);

    // ADD (immediate)
    u32 inst = 0x91000000 | (((u32) value & 0xFFF) << 10) | ((u32) src_reg << 5) | dst_reg;
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_subtract_immediate12(StringBuilder *builder, Arm64Register dst_reg, Arm64Register src_reg, u16 value)
{
    assert(value <= 0xFFF);

    // SUB (immediate)
    u32 inst = 0xD1000000 | (((u32) value & 0xFFF) << 10) | ((u32) src_reg << 5) | dst_reg;
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_add_registers(StringBuilder *builder, Arm64Register dst_reg, Arm64Register src_reg, u64 size)
{
    switch (size)
    {
        case 1:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // ADD (extended register)
                inst = 0x0B208000 | ((u32) src_reg << 16) | ((u32) dst_reg << 5) | dst_reg;
            }
            else
            {
                // ADD (extended register)
                inst = 0x0B200000 | ((u32) src_reg << 16) | ((u32) dst_reg << 5) | dst_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // ADD (extended register)
                inst = 0x0B208000 | ((u32) src_reg << 16) | (0x1 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }
            else
            {
                // ADD (extended register)
                inst = 0x0B200000 | ((u32) src_reg << 16) | (0x1 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // ADD (extended register)
                inst = 0x0B208000 | ((u32) src_reg << 16) | (0x2 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }
            else
            {
                // ADD (extended register)
                inst = 0x0B200000 | ((u32) src_reg << 16) | (0x2 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 8:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // ADD (extended register)
                inst = 0x8B208000 | ((u32) src_reg << 16) | (0x3 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }
            else
            {
                // ADD (extended register)
                inst = 0x8B200000 | ((u32) src_reg << 16) | (0x3 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;
    }
}

static inline void
arm64_subtract_registers(StringBuilder *builder, Arm64Register dst_reg, Arm64Register src_reg, u64 size)
{
    switch (size)
    {
        case 1:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // SUB (extended register)
                inst = 0x4B208000 | ((u32) src_reg << 16) | ((u32) dst_reg << 5) | dst_reg;
            }
            else
            {
                // SUB (extended register)
                inst = 0x4B200000 | ((u32) src_reg << 16) | ((u32) dst_reg << 5) | dst_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // SUB (extended register)
                inst = 0x4B208000 | ((u32) src_reg << 16) | (0x1 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }
            else
            {
                // SUB (extended register)
                inst = 0x4B200000 | ((u32) src_reg << 16) | (0x1 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // SUB (extended register)
                inst = 0x44208000 | ((u32) src_reg << 16) | (0x2 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }
            else
            {
                // SUB (extended register)
                inst = 0x4B200000 | ((u32) src_reg << 16) | (0x2 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 8:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // SUB (extended register)
                inst = 0xCB208000 | ((u32) src_reg << 16) | (0x3 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }
            else
            {
                // SUB (extended register)
                inst = 0xCB200000 | ((u32) src_reg << 16) | (0x3 << 13) | ((u32) dst_reg << 5) | dst_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;
    }
}

static inline void
arm64_copy_from_stack_to_register(StringBuilder *builder, Arm64Register dst_reg, u64 src_stack_offset, u64 size)
{
    assert(!(src_stack_offset & 7));
    assert(src_stack_offset <= 0x7FFF);

    switch (size)
    {
        case 1:
        {
            // LDRB (immediate)
            u32 inst = 0x39400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            // LDRH (immediate)
            u32 inst = 0x79400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            // LDR (immediate)
            u32 inst = 0xB9400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 8:
        {
            // LDR (immediate)
            u32 inst = 0xF9400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        default:
        {
            assert(!"not allowed");
        } break;
    }
}

static inline void
arm64_copy_from_register_to_stack(StringBuilder *builder, u64 dst_stack_offset, Arm64Register src_reg, u64 size)
{
    assert(!(dst_stack_offset & 7));
    assert(dst_stack_offset <= 0x7FFF);

    switch (size)
    {
        case 1:
        {
            // STRB (immediate)
            u32 inst = 0x39000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            // STRH (immediate)
            u32 inst = 0x79000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            // STR (immediate)
            u32 inst = 0xB9000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 8:
        {
            // STR (immediate)
            u32 inst = 0xF9000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        default:
        {
            assert(!"not allowed");
        } break;
    }
}

static inline void
arm64_copy_from_stack_to_stack(StringBuilder *builder, u64 dst_stack_offset, u64 src_stack_offset, u64 size)
{
    assert(!(dst_stack_offset & 7));
    assert(!(src_stack_offset & 7));

    assert(dst_stack_offset <= 0x7FFF);
    assert(src_stack_offset <= 0x7FFF);

    switch (size)
    {
        case 1:
        {
            // LDRB (immediate)
            u32 inst = 0x39400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);

            // STRB (immediate)
            inst = 0x39000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            // LDRH (immediate)
            u32 inst = 0x79400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);

            // STRH (immediate)
            inst = 0x79000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            // LDR (immediate)
            u32 inst = 0xB9400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);

            // STR (immediate)
            inst = 0xB9000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 8:
        {
            // LDR (immediate)
            u32 inst = 0xF9400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);

            // STR (immediate)
            inst = 0xF9000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        default:
        {
            assert(!"not implemented");
        } break;
    }
}

static void
arm64_emit_expression(Parser *parser, StringBuilder *code, Ast *expr)
{
    switch (expr->kind)
    {
        case AST_KIND_LITERAL_BOOLEAN:
        {
            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_subtract_immediate12(code, ARM64_SP, ARM64_SP, (u16) stack_size);
            parser->current_stack_offset += stack_size;

            arm64_move_immediate16(code, ARM64_R0, expr->_bool ? 1 : 0);
            arm64_copy_from_register_to_stack(code, 0, ARM64_R0, datatype->size);
        } break;

        case AST_KIND_LITERAL_INTEGER:
        {
            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            if ((datatype->flags & DATATYPE_FLAG_UNSIGNED) ||
                (expr->_s64 >= 0))
            {
                if (expr->_u64 <= 0xFFFF)
                {
                    arm64_move_immediate16(code, ARM64_R0, expr->_u16);
                }
                else
                {
                    assert(!"not implemented");
                }
            }
            else
            {
                assert(!"not implemented");
            }

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_subtract_immediate12(code, ARM64_SP, ARM64_SP, (u16) stack_size);
            parser->current_stack_offset += stack_size;

            arm64_copy_from_register_to_stack(code, 0, ARM64_R0, datatype->size);
        } break;

        case AST_KIND_IDENTIFIER:
        {
            assert(expr->decl);

            Ast *decl = expr->decl;

            assert(decl->stack_offset > 0);

            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_subtract_immediate12(code, ARM64_SP, ARM64_SP, (u16) stack_size);
            parser->current_stack_offset += stack_size;

            arm64_copy_from_stack_to_stack(code, 0, parser->current_stack_offset - decl->stack_offset, datatype->size);
        } break;

        case AST_KIND_EXPRESSION_BINOP_ADD:
        case AST_KIND_EXPRESSION_BINOP_MINUS:
        {
            arm64_emit_expression(parser, code, expr->left_expr);
            arm64_emit_expression(parser, code, expr->right_expr);

            Datatype *left_datatype = get_datatype(&parser->datatypes, expr->left_expr->type_id);
            Datatype *right_datatype = get_datatype(&parser->datatypes, expr->right_expr->type_id);

            u64 left_stack_size  = Align(left_datatype->size, 16);
            u64 right_stack_size = Align(right_datatype->size, 16);
            u64 total_stack_size = left_stack_size + right_stack_size;

            assert(expr->type_id);
            Datatype *datatype = get_datatype(&parser->datatypes, expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            arm64_copy_from_stack_to_register(code, ARM64_R1, 0, right_datatype->size);
            arm64_copy_from_stack_to_register(code, ARM64_R0, right_stack_size, left_datatype->size);

            // TODO: maybe sign extend arguments

            assert(stack_size <= 0xFFFF);
            assert(total_stack_size <= 0xFFFF);
            arm64_add_immediate12(code, ARM64_SP, ARM64_SP, (u16) (total_stack_size - stack_size));
            parser->current_stack_offset -= total_stack_size - stack_size;

            if (expr->kind == AST_KIND_EXPRESSION_BINOP_ADD)
            {
                arm64_add_registers(code, ARM64_R0, ARM64_R1, datatype->size);
            }
            else
            {
                assert(expr->kind == AST_KIND_EXPRESSION_BINOP_MINUS);
                arm64_subtract_registers(code, ARM64_R0, ARM64_R1, datatype->size);
            }

            arm64_copy_from_register_to_stack(code, 0, ARM64_R0, datatype->size);
        } break;

        default:
        {
            assert(!"expression not supported");
        } break;
    }
}

static void
arm64_emit_function(Parser *parser, StringBuilder *code, Ast *func, JulsPlatform target_platform)
{
    assert(func->kind == AST_KIND_FUNCTION_DECLARATION);

    parser->current_stack_offset = 0;

    For(statement, func->children.first)
    {
        switch (statement->kind)
        {
            case AST_KIND_VARIABLE_DECLARATION:
            {
                Datatype *datatype = get_datatype(&parser->datatypes, statement->type_id);

                u64 stack_size = Align(datatype->size, 16);

                assert(stack_size <= 0xFFFF);
                arm64_subtract_immediate12(code, ARM64_SP, ARM64_SP, (u16) stack_size);
                parser->current_stack_offset += stack_size;
                statement->stack_offset = parser->current_stack_offset;

                if (statement->right_expr)
                {
                    arm64_emit_expression(parser, code, statement->right_expr);

                    // TODO: does the size match the expression?
                    arm64_copy_from_stack_to_stack(code, parser->current_stack_offset - statement->stack_offset, 0, datatype->size);

                    assert(stack_size <= 0xFFFF);
                    arm64_add_immediate12(code, ARM64_SP, ARM64_SP, (u16) stack_size);
                    parser->current_stack_offset -= stack_size;
                }
            } break;

            case AST_KIND_FUNCTION_CALL:
            {
                assert(statement->left_expr);

                Ast *left = statement->left_expr;

                u64 arguments_stack_size = 0;

                For(argument, statement->children.first)
                {
                    arm64_emit_expression(parser, code, argument);

                    Datatype *datatype = get_datatype(&parser->datatypes, argument->type_id);

                    u64 stack_size = datatype->size;
                    arguments_stack_size += stack_size;
                }

                if ((left->kind == AST_KIND_IDENTIFIER) && strings_are_equal(left->name, S("exit")))
                {
                    if ((target_platform == JulsPlatformAndroid) ||
                        (target_platform == JulsPlatformLinux))
                    {
                        arm64_move_immediate16(code, ARM64_R8, 93);
                        arm64_move_indirect_into_register(code, ARM64_R0, ARM64_SP, 0);
                        arm64_svc(code, 0);
                    }
                    else if (target_platform == JulsPlatformMacOs)
                    {
                        arm64_move_immediate16(code, ARM64_R16, 1);
                        arm64_move_indirect_into_register(code, ARM64_R0, ARM64_SP, 0);
                        arm64_svc(code, 0x80);
                    }
                }
                else
                {
                    assert(!"not implemented");
                }

                assert(arguments_stack_size <= 0xFFFF);
                arm64_add_immediate12(code, ARM64_SP, ARM64_SP, (u16) arguments_stack_size);
                parser->current_stack_offset -= arguments_stack_size;
            } break;

            default:
            {
                fprintf(stderr, "error: ast kind %u not supported in function declaration\n", statement->kind);
            } break;
        }
    }
}

static void
generate_arm64(Parser *parser, StringBuilder *code, SymbolTable *symbol_table, JulsPlatform target_platform)
{
    String entry_point_name = S("main");

    u64 jump_location = string_builder_get_size(code);
    u32 *jump_patch = 0;

    u64 _start_offset = string_builder_get_size(code);

    if ((target_platform == JulsPlatformAndroid) ||
        (target_platform == JulsPlatformLinux))
    {
        // bl main
        jump_patch = string_builder_append_size(code, 4);

        // mov r8, #94
        arm64_move_immediate16(code, ARM64_R8, 94);

        // mov r0, #42
        arm64_move_immediate16(code, ARM64_R0, 42);

        // svc #0
        arm64_svc(code, 0);
    }
    else if (target_platform == JulsPlatformMacOs)
    {
        // bl main
        jump_patch = string_builder_append_size(code, 4);

        // mov r8, #1
        arm64_move_immediate16(code, ARM64_R16, 1);

        // mov r0, #42
        arm64_move_immediate16(code, ARM64_R0, 42);

        // svc #0
        arm64_svc(code, 0x80);
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

            arm64_store_register(code, ARM64_R30, ARM64_SP, -16);

            arm64_emit_function(parser, code, decl, target_platform);

            arm64_load_register(code, ARM64_R30, ARM64_SP, 16);
            arm64_ret(code);

            u64 size = string_builder_get_size(code) - offset;

            array_append(symbol_table, ((SymbolEntry) { .name = decl->name, .offset = offset, .size = size }));
        }
    }

    if (jump_target > 0)
    {
        *jump_patch = 0x94000000 | (((u32) (jump_target - jump_location) >> 2) & 0x3FFFFFF);
    }
    else
    {
        fprintf(stderr, "error: there is no entry point '%.*s'\n", (int) entry_point_name.count, entry_point_name.data);
    }
}
