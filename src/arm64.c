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
    // MOV (wide immediate)
    u32 inst = 0xD2800000 | ((u32) value << 5) | reg;
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_move_inverted_immediate16(StringBuilder *builder, Arm64Register reg, u16 value)
{
    // MOV (inverted wide immediate)
    u32 inst = 0x92800000 | ((u32) value << 5) | reg;
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
    // STR (immediate) pre-index
    u32 inst = 0xF8000C00 | (((u32) -16 & 0x1FF) << 12) | ((u32) ARM64_SP << 5) | reg;
    string_builder_append_u32le(builder, inst);
}

static inline void
arm64_pop_register(StringBuilder *builder, Arm64Register reg)
{
    // LDR (immediate) post-index
    u32 inst = 0xF8400400 | (((u32) 16 & 0x1FF) << 12) | ((u32) ARM64_SP << 5) | reg;
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
            u32 inst = 0x39400000 | (((u32) src_stack_offset & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | dst_reg;
            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            // LDRH (immediate)
            u32 inst = 0x79400000 | (((u32) (src_stack_offset >> 1) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | dst_reg;
            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            // LDR (immediate)
            u32 inst = 0xB9400000 | (((u32) (src_stack_offset >> 2) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | dst_reg;
            string_builder_append_u32le(builder, inst);
        } break;

        case 8:
        {
            // LDR (immediate)
            u32 inst = 0xF9400000 | (((u32) (src_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | dst_reg;
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
            u32 inst = 0x39000000 | (((u32) dst_stack_offset & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | src_reg;
            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            // STRH (immediate)
            u32 inst = 0x79000000 | (((u32) (dst_stack_offset >> 1) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | src_reg;
            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            // STR (immediate)
            u32 inst = 0xB9000000 | (((u32) (dst_stack_offset >> 2) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | src_reg;
            string_builder_append_u32le(builder, inst);
        } break;

        case 8:
        {
            // STR (immediate)
            u32 inst = 0xF9000000 | (((u32) (dst_stack_offset >> 3) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | src_reg;
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
            u32 inst = 0x39400000 | (((u32) src_stack_offset & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);

            // STRB (immediate)
            inst = 0x39000000 | (((u32) dst_stack_offset & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            // LDRH (immediate)
            u32 inst = 0x79400000 | (((u32) (src_stack_offset >> 1) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);

            // STRH (immediate)
            inst = 0x79000000 | (((u32) (dst_stack_offset >> 1) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            // LDR (immediate)
            u32 inst = 0xB9400000 | (((u32) (src_stack_offset >> 2) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);

            // STR (immediate)
            inst = 0xB9000000 | (((u32) (dst_stack_offset >> 2) & 0xFFF) << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
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

        case 16:
        {
            assert(dst_stack_offset <= 0x3FF);
            assert(src_stack_offset <= 0x3FF);

            // LDP
            u32 inst = 0xA9400000 | (((u32) (src_stack_offset >> 3) & 0x7F) << 15) | ((u32) ARM64_R1 << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);

            // STP
            inst = 0xA9000000 | (((u32) (dst_stack_offset >> 3) & 0x7F) << 15) | ((u32) ARM64_R1 << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(builder, inst);
        } break;

        default:
        {
            assert(!"not implemented");
        } break;
    }
}

static inline void
arm64_compare_registers(StringBuilder *builder, Arm64Register a_reg, Arm64Register b_reg, u64 size)
{
    switch (size)
    {
        case 1:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // SUBS/CMP (extended register)
                inst = 0x6B208000 | ((u32) b_reg << 16) | ((u32) a_reg << 5) | a_reg;
            }
            else
            {
                // SUBS/CMP (extended register)
                inst = 0x6B200000 | ((u32) b_reg << 16) | ((u32) a_reg << 5) | a_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 2:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // SUBS/CMP (extended register)
                inst = 0x6B208000 | ((u32) b_reg << 16) | (0x1 << 13) | ((u32) a_reg << 5) | a_reg;
            }
            else
            {
                // SUBS/CMP (extended register)
                inst = 0x6B200000 | ((u32) b_reg << 16) | (0x1 << 13) | ((u32) a_reg << 5) | a_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 4:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // SUBS/CMP (extended register)
                inst = 0x64208000 | ((u32) b_reg << 16) | (0x2 << 13) | ((u32) a_reg << 5) | a_reg;
            }
            else
            {
                // SUBS/CMP (extended register)
                inst = 0x6B200000 | ((u32) b_reg << 16) | (0x2 << 13) | ((u32) a_reg << 5) | a_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;

        case 8:
        {
            u32 inst;

            if (false /* is_signed */)
            {
                // SUBS/CMP (extended register)
                inst = 0xEB208000 | ((u32) b_reg << 16) | (0x3 << 13) | ((u32) a_reg << 5) | a_reg;
            }
            else
            {
                // SUBS/CMP (extended register)
                inst = 0xEB200000 | ((u32) b_reg << 16) | (0x3 << 13) | ((u32) a_reg << 5) | a_reg;
            }

            string_builder_append_u32le(builder, inst);
        } break;
    }
}

static void
arm64_emit_cast(Compiler *compiler, Codegen *codegen, DatatypeId from_type_id, DatatypeId to_type_id)
{
    if (from_type_id == to_type_id)
    {
        return;
    }

    Datatype *from_type = get_datatype(&compiler->datatypes, from_type_id);
    Datatype *to_type   = get_datatype(&compiler->datatypes, to_type_id);

    u64 from_type_stack_size = Align(from_type->size, 16);
    u64 to_type_stack_size = Align(to_type->size, 16);

    if ((from_type->kind == DATATYPE_INTEGER) && (to_type->kind == DATATYPE_INTEGER))
    {
        if (from_type->size > to_type->size)
        {
            if (from_type_stack_size != to_type_stack_size)
            {
                arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 0, from_type->size);

                assert(from_type_stack_size <= 0xFFFF);
                assert(to_type_stack_size <= 0xFFFF);
                arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) (from_type_stack_size - to_type_stack_size));
                compiler->current_stack_offset -= from_type_stack_size - to_type_stack_size;

                arm64_copy_from_register_to_stack(&codegen->section_text, 0, ARM64_R0, to_type->size);
            }
        }
        else if (from_type->size < to_type->size)
        {
            assert(!"not implemented");
        }
    }
    else
    {
        assert(!"not implemented");
    }
}

static void
arm64_emit_expression(Compiler *compiler, Codegen *codegen, Ast *expr, JulsPlatform target_platform)
{
    switch (expr->kind)
    {
        case AST_KIND_LITERAL_BOOLEAN:
        {
            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_subtract_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
            compiler->current_stack_offset += stack_size;

            arm64_move_immediate16(&codegen->section_text, ARM64_R0, expr->_bool ? 1 : 0);
            arm64_copy_from_register_to_stack(&codegen->section_text, 0, ARM64_R0, datatype->size);
        } break;

        case AST_KIND_LITERAL_INTEGER:
        {
            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            if ((datatype->flags & DATATYPE_FLAG_UNSIGNED) ||
                (expr->_s64 >= 0))
            {
                if (expr->_u64 <= 0xFFFF)
                {
                    arm64_move_immediate16(&codegen->section_text, ARM64_R0, expr->_u16);
                }
                else
                {
                    assert(!"not implemented");
                }
            }
            else
            {
                u64 inverted = ~expr->_u64;

                if (inverted <= 0xFFFF)
                {
                    arm64_move_inverted_immediate16(&codegen->section_text, ARM64_R0, (u16) inverted);
                }
                else
                {
                    assert(!"not implemented");
                }
            }

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_subtract_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
            compiler->current_stack_offset += stack_size;

            arm64_copy_from_register_to_stack(&codegen->section_text, 0, ARM64_R0, datatype->size);
        } break;

        case AST_KIND_LITERAL_STRING:
        {
            u64 string_offset = string_builder_get_size(&codegen->section_cstring);
            string_builder_append_string(&codegen->section_cstring, expr->name);

            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_subtract_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
            compiler->current_stack_offset += stack_size;

            arm64_move_immediate16(&codegen->section_text, ARM64_R0, expr->name.count);

            u64 instruction_offset = string_builder_get_size(&codegen->section_text);
            void *patch_addr = string_builder_append_size(&codegen->section_text, 8);

            // STP
            u32 inst = 0xA9000000 | ((u32) ARM64_R1 << 10) | ((u32) ARM64_SP << 5) | ARM64_R0;
            string_builder_append_u32le(&codegen->section_text, inst);

            array_append(&codegen->patches, ((Patch) { .patch = patch_addr,
                                                       .instruction_offset = instruction_offset,
                                                       .string_offset = string_offset }));
        } break;

        case AST_KIND_IDENTIFIER:
        {
            assert(expr->decl);

            Ast *decl = expr->decl;

            assert(decl->stack_offset > 0);

            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_subtract_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
            compiler->current_stack_offset += stack_size;

            arm64_copy_from_stack_to_stack(&codegen->section_text, 0, compiler->current_stack_offset - decl->stack_offset, datatype->size);
        } break;

        case AST_KIND_EXPRESSION_EQUAL:
        case AST_KIND_EXPRESSION_NOT_EQUAL:
        case AST_KIND_EXPRESSION_COMPARE_LESS:
        case AST_KIND_EXPRESSION_COMPARE_GREATER:
        case AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL:
        case AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL:
        {
            arm64_emit_expression(compiler, codegen, expr->left_expr, target_platform);
            arm64_emit_expression(compiler, codegen, expr->right_expr, target_platform);

            Datatype *left_datatype  = get_datatype(&compiler->datatypes, expr->left_expr->type_id);
            Datatype *right_datatype = get_datatype(&compiler->datatypes, expr->right_expr->type_id);

            u64 max_datatype_size = left_datatype->size;

            if (right_datatype->size > max_datatype_size)
            {
                max_datatype_size = right_datatype->size;
            }

            u64 left_stack_size = Align(left_datatype->size, 16);
            u64 right_stack_size = Align(right_datatype->size, 16);
            u64 total_stack_size = left_stack_size + right_stack_size;

            assert(expr->type_id == compiler->basetype_bool);
            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R1, 0, right_datatype->size);
            arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, right_stack_size, left_datatype->size);

            // TODO: maybe sign extend arguments

            assert(stack_size <= 0xFFFF);
            assert(total_stack_size <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) (total_stack_size - stack_size));
            compiler->current_stack_offset -= total_stack_size - stack_size;

            arm64_compare_registers(&codegen->section_text, ARM64_R0, ARM64_R1, max_datatype_size);

            u32 cond = 0;

            bool is_signed = true; // TODO:

            if (is_signed)
            {
                if (expr->kind == AST_KIND_EXPRESSION_EQUAL)
                {
                    cond = 0x0;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_NOT_EQUAL)
                {
                    cond = 0x1;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS)
                {
                    cond = 0xB;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER)
                {
                    cond = 0xC;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL)
                {
                    cond = 0xD;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL)
                {
                    cond = 0xA;
                }
            }
            else
            {
                if (expr->kind == AST_KIND_EXPRESSION_EQUAL)
                {
                    cond = 0x0;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_NOT_EQUAL)
                {
                    cond = 0x1;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS)
                {
                    cond = 0x4;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER)
                {
                    cond = 0x8;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL)
                {
                    cond = 0x9;
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL)
                {
                    cond = 0x5;
                }
            }

            cond ^= 0x1;

            // CSET/CSINC
            u32 inst = 0x9A9F07E0 | ((cond & 0xF) << 12) | ARM64_R0;
            string_builder_append_u32le(&codegen->section_text, inst);

            arm64_copy_from_register_to_stack(&codegen->section_text, 0, ARM64_R0, datatype->size);
        } break;

        case AST_KIND_EXPRESSION_BINOP_ADD:
        case AST_KIND_EXPRESSION_BINOP_MINUS:
        {
            arm64_emit_expression(compiler, codegen, expr->left_expr, target_platform);
            arm64_emit_expression(compiler, codegen, expr->right_expr, target_platform);

            Datatype *left_datatype  = get_datatype(&compiler->datatypes, expr->left_expr->type_id);
            Datatype *right_datatype = get_datatype(&compiler->datatypes, expr->right_expr->type_id);

            u64 left_stack_size  = Align(left_datatype->size, 16);
            u64 right_stack_size = Align(right_datatype->size, 16);
            u64 total_stack_size = left_stack_size + right_stack_size;

            assert(expr->type_id);
            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R1, 0, right_datatype->size);
            arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, right_stack_size, left_datatype->size);

            // TODO: maybe sign extend arguments

            assert(stack_size <= 0xFFFF);
            assert(total_stack_size <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) (total_stack_size - stack_size));
            compiler->current_stack_offset -= total_stack_size - stack_size;

            if (expr->kind == AST_KIND_EXPRESSION_BINOP_ADD)
            {
                arm64_add_registers(&codegen->section_text, ARM64_R0, ARM64_R1, datatype->size);
            }
            else
            {
                assert(expr->kind == AST_KIND_EXPRESSION_BINOP_MINUS);
                arm64_subtract_registers(&codegen->section_text, ARM64_R0, ARM64_R1, datatype->size);
            }

            arm64_copy_from_register_to_stack(&codegen->section_text, 0, ARM64_R0, datatype->size);
        } break;

        case AST_KIND_FUNCTION_CALL:
        {
            assert(expr->left_expr);

            Ast *left = expr->left_expr;

            if (left->kind == AST_KIND_IDENTIFIER)
            {
                assert(expr->decl);

                Datatype *return_type = get_datatype(&compiler->datatypes, expr->decl->type_id);
                u64 return_type_stack_size = Align(return_type->size, 16);

                assert(return_type_stack_size <= 0xFFFF);
                arm64_subtract_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) return_type_stack_size);
                compiler->current_stack_offset += return_type_stack_size;
            }
            else
            {
                assert(!"not implemented");
            }

            u64 arguments_stack_size = 0;

            ForReversed(argument, expr->children.last)
            {
                // TODO: does the size match the expression?
                arm64_emit_expression(compiler, codegen, argument, target_platform);

                Datatype *datatype = get_datatype(&compiler->datatypes, argument->type_id);

                u64 stack_size = Align(datatype->size, 16);
                arguments_stack_size += stack_size;
            }

            if (left->kind == AST_KIND_IDENTIFIER)
            {
                if (strings_are_equal(left->name, S("exit")))
                {
                    if ((target_platform == JulsPlatformAndroid) ||
                        (target_platform == JulsPlatformLinux))
                    {
                        arm64_move_immediate16(&codegen->section_text, ARM64_R8, 93);
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 0, 8);
                        arm64_svc(&codegen->section_text, 0);
                    }
                    else if (target_platform == JulsPlatformMacOs)
                    {
                        arm64_move_immediate16(&codegen->section_text, ARM64_R16, 1);
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 0, 8);
                        arm64_svc(&codegen->section_text, 0x80);
                    }
                }
                else if (strings_are_equal(left->name, S("write")))
                {
                    if ((target_platform == JulsPlatformAndroid) ||
                        (target_platform == JulsPlatformLinux))
                    {
                        arm64_move_immediate16(&codegen->section_text, ARM64_R8, 64);
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 0, 4);
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R1, 16, 8);
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R2, 32, 8);
                        arm64_svc(&codegen->section_text, 0);
                    }
                    else if (target_platform == JulsPlatformMacOs)
                    {
                        arm64_move_immediate16(&codegen->section_text, ARM64_R16, 4);
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 0, 4);
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R1, 16, 8);
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R2, 32, 8);
                        arm64_svc(&codegen->section_text, 0x80);
                    }
                }
                else
                {
                    assert(expr->decl);

                    if (expr->decl->address == S64MAX)
                    {
                        u64 instruction_offset = string_builder_get_size(&codegen->section_text);
                        void *patch_addr = string_builder_append_size(&codegen->section_text, 4);

                        array_append(&codegen->function_call_patches,
                                     ((FunctionCallPatch) { .patch = patch_addr,
                                                            .instruction_offset = instruction_offset,
                                                            .function_decl = expr->decl }));
                    }
                    else
                    {
                        s64 jump_offset = string_builder_get_size(&codegen->section_text);
                        // BL
                        u32 inst = 0x94000000 | (((u32) (expr->decl->address - jump_offset) >> 2) & 0x3FFFFFF);
                        string_builder_append_u32le(&codegen->section_text, inst);
                    }
                }
            }
            else
            {
                assert(!"not implemented");
            }

            assert(arguments_stack_size <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) arguments_stack_size);
            compiler->current_stack_offset -= arguments_stack_size;
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

            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);
            Datatype *right_datatype = get_datatype(&compiler->datatypes, expr->right_expr->type_id);

            u64 stack_size = Align(datatype->size, 16);
            u64 right_stack_size = Align(right_datatype->size, 16);

            if (expr->kind != AST_KIND_ASSIGN)
            {
                assert(stack_size <= 0xFFFF);
                arm64_subtract_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
                compiler->current_stack_offset += stack_size;

                arm64_copy_from_stack_to_stack(&codegen->section_text, 0, compiler->current_stack_offset - decl->stack_offset, datatype->size);
            }

            arm64_emit_expression(compiler, codegen, expr->right_expr, target_platform);

            if (expr->kind != AST_KIND_ASSIGN)
            {
                arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R1, 0, right_datatype->size);
                arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, right_stack_size, datatype->size);

                // TODO: maybe sign extend arguments

                assert(right_stack_size <= 0xFFFF);
                arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) right_stack_size);
                compiler->current_stack_offset -= right_stack_size;

                if (expr->kind == AST_KIND_PLUS_ASSIGN)
                {
                    arm64_add_registers(&codegen->section_text, ARM64_R0, ARM64_R1, datatype->size);
                }
                else
                {
                    assert(expr->kind == AST_KIND_MINUS_ASSIGN);
                    arm64_subtract_registers(&codegen->section_text, ARM64_R0, ARM64_R1, datatype->size);
                }

                arm64_copy_from_register_to_stack(&codegen->section_text, 0, ARM64_R0, datatype->size);
            }

            // TODO: does the size match the expression?
            arm64_copy_from_stack_to_stack(&codegen->section_text, compiler->current_stack_offset - decl->stack_offset, 0, datatype->size);
        } break;

        case AST_KIND_MEMBER:
        {
            if (expr->left_expr->type_id == compiler->basetype_string)
            {
                if (strings_are_equal(expr->name, S("count")) ||
                    strings_are_equal(expr->name, S("data")))
                {
                    arm64_emit_expression(compiler, codegen, expr->left_expr, target_platform);

                    Datatype *left_datatype = get_datatype(&compiler->datatypes, expr->left_expr->type_id);
                    Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

                    u64 left_stack_size = Align(left_datatype->size, 16);
                    u64 stack_size = Align(datatype->size, 16);

                    if (strings_are_equal(expr->name, S("count")))
                    {
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 0, datatype->size);
                    }
                    else if (strings_are_equal(expr->name, S("data")))
                    {
                        arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 8, datatype->size);
                    }

                    assert(left_stack_size <= 0xFFFF);
                    assert(stack_size <= 0xFFFF);
                    arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) (left_stack_size - stack_size));
                    compiler->current_stack_offset -= left_stack_size - stack_size;

                    arm64_copy_from_register_to_stack(&codegen->section_text, 0, ARM64_R0, datatype->size);
                }
                else
                {
                    assert(!"not supported");
                }
            }
            else
            {
                assert(!"not supported");
            }
        } break;

        case AST_KIND_CAST:
        {
            arm64_emit_expression(compiler, codegen, expr->left_expr, target_platform);
            arm64_emit_cast(compiler, codegen, expr->left_expr->type_id, expr->type_id);
        } break;

        default:
        {
            printf("kind = %u\n", expr->kind);
            assert(!"expression not supported");
        } break;
    }
}

static void
arm64_emit_statement(Compiler *compiler, Codegen *codegen, Ast *statement, JulsPlatform target_platform,
                     Datatype *return_type, u64 return_type_stack_size)
{
    switch (statement->kind)
    {
        case AST_KIND_VARIABLE_DECLARATION:
        {
            Datatype *datatype = get_datatype(&compiler->datatypes, statement->type_id);

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_subtract_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
            compiler->current_stack_offset += stack_size;
            statement->stack_offset = compiler->current_stack_offset;

            compiler->stack_allocated[compiler->stack_allocated_index] += stack_size;

            if (statement->right_expr)
            {
                arm64_emit_expression(compiler, codegen, statement->right_expr, target_platform);

                // TODO: does the size match the expression?
                arm64_copy_from_stack_to_stack(&codegen->section_text, compiler->current_stack_offset - statement->stack_offset, 0, datatype->size);

                assert(stack_size <= 0xFFFF);
                arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
                compiler->current_stack_offset -= stack_size;
            }
        } break;

        case AST_KIND_IF:
        {
            arm64_emit_expression(compiler, codegen, statement->left_expr, target_platform);

            assert(statement->left_expr->type_id == compiler->basetype_bool);
            Datatype *datatype = get_datatype(&compiler->datatypes, statement->left_expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 0, datatype->size);

            assert(stack_size <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
            compiler->current_stack_offset -= stack_size;

            // SUBS (immediate)
            u32 inst = 0xF1000000 | ((u32) ARM64_R0 << 5) | ARM64_R0;
            string_builder_append_u32le(&codegen->section_text, inst);

            // JZ
            s64 else_offset = string_builder_get_size(&codegen->section_text);
            u32 *else_patch = string_builder_append_size(&codegen->section_text, 4);

            Ast *if_code = statement->children.first;
            Ast *else_code = 0;

            if (statement->children.first != statement->children.last)
            {
                else_code = statement->children.last;
            }

            arm64_emit_statement(compiler, codegen, if_code, target_platform, return_type, return_type_stack_size);

            s64 end_offset = string_builder_get_size(&codegen->section_text);
            u32 *end_patch = 0;

            if (else_code)
            {
                // B
                end_patch = string_builder_append_size(&codegen->section_text, 4);
            }

            s64 else_target = string_builder_get_size(&codegen->section_text);
            *else_patch = 0x54000000 | ((((u32) (else_target - else_offset) >> 2) & 0x7FFFF) << 5);

            if (else_code)
            {
                arm64_emit_statement(compiler, codegen, else_code, target_platform, return_type, return_type_stack_size);

                s64 end_target = string_builder_get_size(&codegen->section_text);
                *end_patch = 0x14000000 | (((u32) (end_target - end_offset) >> 2) & 0x3FFFFFF);
            }
        } break;

        case AST_KIND_FOR:
        {
            push_scope(compiler);

            arm64_emit_statement(compiler, codegen, statement->decl, target_platform, return_type, return_type_stack_size);

            s64 start_target = string_builder_get_size(&codegen->section_text);

            arm64_emit_expression(compiler, codegen, statement->left_expr, target_platform);

            assert(statement->left_expr->type_id == compiler->basetype_bool);
            Datatype *datatype = get_datatype(&compiler->datatypes, statement->left_expr->type_id);

            u64 stack_size = Align(datatype->size, 16);

            arm64_copy_from_stack_to_register(&codegen->section_text, ARM64_R0, 0, datatype->size);

            assert(stack_size <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
            compiler->current_stack_offset -= stack_size;

            // SUBS (immediate)
            u32 inst = 0xF1000000 | ((u32) ARM64_R0 << 5) | ARM64_R0;
            string_builder_append_u32le(&codegen->section_text, inst);

            // JZ
            s64 end_offset = string_builder_get_size(&codegen->section_text);
            u32 *end_patch = string_builder_append_size(&codegen->section_text, 4);

            arm64_emit_statement(compiler, codegen, statement->children.first, target_platform, return_type, return_type_stack_size);
            arm64_emit_expression(compiler, codegen, statement->right_expr, target_platform);

            Datatype *right_datatype = get_datatype(&compiler->datatypes, statement->right_expr->type_id);

            u64 right_stack_size = Align(right_datatype->size, 16);

            assert(right_stack_size <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) right_stack_size);
            compiler->current_stack_offset -= right_stack_size;

            s64 start_offset = string_builder_get_size(&codegen->section_text);
            inst = 0x14000000 | (((u32) (start_target - start_offset) >> 2) & 0x3FFFFFF);
            string_builder_append_u32le(&codegen->section_text, inst);

            s64 end_target = string_builder_get_size(&codegen->section_text);
            *end_patch = 0x54000000 | ((((u32) (end_target - end_offset) >> 2) & 0x7FFFF) << 5);

            u64 stack_allocated = compiler->stack_allocated[compiler->stack_allocated_index];
            pop_scope(compiler);

            assert(stack_allocated <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_allocated);
            compiler->current_stack_offset -= stack_allocated;
        } break;

        case AST_KIND_RETURN:
        {
            assert(statement->left_expr);

            arm64_emit_expression(compiler, codegen, statement->left_expr, target_platform);

            arm64_copy_from_stack_to_stack(&codegen->section_text, compiler->current_stack_offset - return_type_stack_size, 0, return_type->size);

            assert(return_type_stack_size <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) return_type_stack_size);
            compiler->current_stack_offset -= return_type_stack_size;

            u64 stack_allocated = 0;

            while (compiler->stack_allocated_index >= 0)
            {
                stack_allocated += compiler->stack_allocated[compiler->stack_allocated_index];
                pop_scope(compiler);
            }

            assert(stack_allocated <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_allocated);

            arm64_pop_register(&codegen->section_text, ARM64_R30); // restore link register
            arm64_ret(&codegen->section_text);
        } break;

        case AST_KIND_BLOCK:
        {
            push_scope(compiler);

            For(stmt, statement->children.first)
            {
                arm64_emit_statement(compiler, codegen, stmt, target_platform, return_type, return_type_stack_size);
            }

            u64 stack_allocated = compiler->stack_allocated[compiler->stack_allocated_index];
            pop_scope(compiler);

            assert(stack_allocated <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_allocated);
            compiler->current_stack_offset -= stack_allocated;
        } break;

        default:
        {
            arm64_emit_expression(compiler, codegen, statement, target_platform);

            Datatype *datatype = get_datatype(&compiler->datatypes, statement->type_id);

            u64 stack_size = Align(datatype->size, 16);

            assert(stack_size <= 0xFFFF);
            arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_size);
            compiler->current_stack_offset -= stack_size;
        } break;
    }
}

static void
arm64_emit_function(Compiler *compiler, Codegen *codegen, Ast *func, JulsPlatform target_platform)
{
    assert(func->kind == AST_KIND_FUNCTION_DECLARATION);

    func->address = string_builder_get_size(&codegen->section_text);

    compiler->current_stack_offset = 0;
    compiler->stack_allocated_index = -1;

    Datatype *return_type = get_datatype(&compiler->datatypes, func->type_id);
    u64 return_type_stack_size = Align(return_type->size, 16);
    compiler->current_stack_offset += return_type_stack_size;

    ForReversed(parameter, func->parameters.last)
    {
        Datatype *datatype = get_datatype(&compiler->datatypes, parameter->type_id);

        u64 stack_size = Align(datatype->size, 16);

        compiler->current_stack_offset += stack_size;
        parameter->stack_offset = compiler->current_stack_offset;
    }

    arm64_push_register(&codegen->section_text, ARM64_R30); // save link register
    compiler->current_stack_offset += 16;

    push_scope(compiler);

    For(statement, func->children.first)
    {
        arm64_emit_statement(compiler, codegen, statement, target_platform, return_type, return_type_stack_size);
    }

    if (!func->type_def)
    {
        assert(compiler->stack_allocated_index == 0);

        u64 stack_allocated = compiler->stack_allocated[compiler->stack_allocated_index];
        pop_scope(compiler);

        assert(stack_allocated <= 0xFFFF);
        arm64_add_immediate12(&codegen->section_text, ARM64_SP, ARM64_SP, (u16) stack_allocated);

        arm64_pop_register(&codegen->section_text, ARM64_R30); // restore link register
        arm64_ret(&codegen->section_text);
    }
}

static void
generate_arm64(Compiler *compiler, Codegen *codegen, SymbolTable *symbol_table, JulsPlatform target_platform)
{
    String entry_point_name = S("main");

    u64 jump_location = string_builder_get_size(&codegen->section_text);
    u32 *jump_patch = 0;

    u64 _start_offset = string_builder_get_size(&codegen->section_text);

    if ((target_platform == JulsPlatformAndroid) ||
        (target_platform == JulsPlatformLinux))
    {
        // bl main
        jump_patch = string_builder_append_size(&codegen->section_text, 4);

        // mov r8, #94
        arm64_move_immediate16(&codegen->section_text, ARM64_R8, 94);

        // mov r0, #0
        arm64_move_immediate16(&codegen->section_text, ARM64_R0, 0);

        // svc #0
        arm64_svc(&codegen->section_text, 0);
    }
    else if (target_platform == JulsPlatformWindows)
    {
        // bl main
        jump_patch = string_builder_append_size(&codegen->section_text, 4);

        // TODO: implement
        // where to put the exit code?
        arm64_ret(&codegen->section_text);
    }
    else if (target_platform == JulsPlatformMacOs)
    {
        // bl main
        jump_patch = string_builder_append_size(&codegen->section_text, 4);

        // mov r8, #1
        arm64_move_immediate16(&codegen->section_text, ARM64_R16, 1);

        // mov r0, #0
        arm64_move_immediate16(&codegen->section_text, ARM64_R0, 0);

        // svc #0
        arm64_svc(&codegen->section_text, 0x80);
    }

    u64 _start_size = string_builder_get_size(&codegen->section_text) - _start_offset;

    array_append(symbol_table, ((SymbolEntry) { .name = S("_start"), .offset = _start_offset, .size = _start_size }));

    u64 jump_target = 0;

    For(decl, compiler->global_declarations.children.first)
    {
        if (decl->kind == AST_KIND_FUNCTION_DECLARATION)
        {
            u64 offset = string_builder_get_size(&codegen->section_text);

            if (strings_are_equal(entry_point_name, decl->name))
            {
                jump_target = offset;
            }

            arm64_emit_function(compiler, codegen, decl, target_platform);

            u64 size = string_builder_get_size(&codegen->section_text) - offset;

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

    for (s32 i = 0; i < codegen->function_call_patches.count; i += 1)
    {
        FunctionCallPatch *patch = codegen->function_call_patches.items + i;
        Ast *function_decl = patch->function_decl;

        assert(function_decl->address != S64MAX);

        *(u32 *) patch->patch = 0x94000000 | (((u32) (function_decl->address - patch->instruction_offset) >> 2) & 0x3FFFFFF);
    }
}
