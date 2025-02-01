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
x64_move_registers(StringBuilder *builder, X64Register dst_reg, X64Register src_reg)
{
    string_builder_append_u8(builder, REX_W);
    string_builder_append_u8(builder, 0x89);
    string_builder_append_u8(builder, ModRM(3, src_reg, dst_reg));
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
x64_copy_from_stack_to_register(StringBuilder *builder, X64Register dst_reg, s64 src_stack_offset, u64 size)
{
    switch (size)
    {
        case 1:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x8A);
                string_builder_append_u8(builder, ModRM(1, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x8A);
                string_builder_append_u8(builder, ModRM(2, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }
        } break;

        case 2:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }
        } break;

        case 4:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }
        } break;

        case 8:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, dst_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

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
x64_copy_from_register_to_stack(StringBuilder *builder, s64 dst_stack_offset, X64Register src_reg, u64 size)
{
    switch (size)
    {
        case 1:
        {
            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x88);
                string_builder_append_u8(builder, ModRM(1, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x88);
                string_builder_append_u8(builder, ModRM(2, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 2:
        {
            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 4:
        {
            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 8:
        {
            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, src_reg, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

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
x64_copy_from_stack_to_stack(StringBuilder *builder, s64 dst_stack_offset, s64 src_stack_offset, u64 size)
{
    switch (size)
    {
        case 1:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x8A);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x8A);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x88);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x88);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 2:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x66);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 4:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 8:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }
        } break;

        case 16:
        {
            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            src_stack_offset += 8;

            if ((src_stack_offset >= INT8_MIN) && (src_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(1, X64_RBX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) src_stack_offset);
            }
            else
            {
                assert((src_stack_offset >= INT32_MIN) && (src_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x8B);
                string_builder_append_u8(builder, ModRM(2, X64_RBX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) src_stack_offset);
            }

            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, X64_RAX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u32le(builder, (u32) dst_stack_offset);
            }

            dst_stack_offset += 8;

            if ((dst_stack_offset >= INT8_MIN) && (dst_stack_offset <= INT8_MAX))
            {
                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(1, X64_RBX, X64_RSP));
                string_builder_append_u8(builder, SIB(0, X64_RSP, X64_RSP));
                string_builder_append_u8(builder, (u8) dst_stack_offset);
            }
            else
            {
                assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

                string_builder_append_u8(builder, REX_W);
                string_builder_append_u8(builder, 0x89);
                string_builder_append_u8(builder, ModRM(2, X64_RBX, X64_RSP));
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
x64_setcc(StringBuilder *builder, u8 compare_func, s64 dst_stack_offset)
{
    assert((dst_stack_offset >= INT32_MIN) && (dst_stack_offset <= INT32_MAX));

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

static inline void
x64_commit_stack(Codegen *codegen, StringBuilder *builder)
{
    if (codegen->stack_allocated < codegen->stack_committed)
    {
        s64 size = codegen->stack_committed - codegen->stack_allocated;
        assert(size <= 0xFFFFFFFF);
        x64_add_immediate32_unsigned_to_register(builder, X64_RSP, (u32) size);
        codegen->stack_committed = codegen->stack_allocated;
    }
    else if (codegen->stack_allocated > codegen->stack_committed)
    {
        s64 size = codegen->stack_allocated - codegen->stack_committed;
        assert(size <= 0xFFFFFFFF);
        x64_subtract_immediate32_unsigned_from_register(builder, X64_RSP, (u32) size);
        codegen->stack_committed = codegen->stack_allocated;
    }
}

static void
x64_emit_cast(Compiler *compiler, Codegen *codegen, DatatypeId from_type_id, DatatypeId to_type_id)
{
    if (from_type_id == to_type_id)
    {
        return;
    }

    Datatype *from_type = get_datatype(&compiler->datatypes, from_type_id);
    Datatype *to_type   = get_datatype(&compiler->datatypes, to_type_id);

    u64 from_type_stack_size = from_type->size;
    u64 to_type_stack_size = to_type->size;

    if ((from_type->kind == DATATYPE_INTEGER) && (to_type->kind == DATATYPE_INTEGER))
    {
        if (from_type->size > to_type->size)
        {
            s64 from_type_stack_offset = codegen->stack_allocated;

            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, codegen->stack_committed - from_type_stack_offset, from_type->size);

            pop_stack(codegen, from_type_stack_size);
            s64 to_type_stack_offset = push_stack(codegen, to_type_stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            x64_copy_from_register_to_stack(&codegen->section_text, codegen->stack_committed - to_type_stack_offset, X64_RAX, to_type->size);
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
x64_emit_expression(Compiler *compiler, Codegen *codegen, Ast *expr, JulsPlatform target_platform)
{
    switch (expr->kind)
    {
        case AST_KIND_LITERAL_BOOLEAN:
        {
            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            expr->stack_offset = push_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, expr->_bool ? 1 : 0);
            x64_copy_from_register_to_stack(&codegen->section_text, codegen->stack_committed - expr->stack_offset, X64_RAX, datatype->size);
        } break;

        case AST_KIND_LITERAL_INTEGER:
        {
            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

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

            expr->stack_offset = push_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            x64_copy_from_register_to_stack(&codegen->section_text, codegen->stack_committed - expr->stack_offset, X64_RAX, datatype->size);
        } break;

        case AST_KIND_LITERAL_STRING:
        {
            u64 string_offset = string_builder_get_size(&codegen->section_cstring);
            string_builder_append_string(&codegen->section_cstring, expr->name);

            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            expr->stack_offset = push_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, expr->name.count);

            // lea
            string_builder_append_u8(&codegen->section_text, REX_W);
            string_builder_append_u8(&codegen->section_text, 0x8D);
            string_builder_append_u8(&codegen->section_text, ModRM(0, X64_RBX, X64_RBP /* RIP */));

            void *patch_addr = string_builder_append_size(&codegen->section_text, 4);
            u64 instruction_offset = string_builder_get_size(&codegen->section_text);

            x64_copy_from_register_to_stack(&codegen->section_text, (codegen->stack_committed - expr->stack_offset) + 0, X64_RAX, 8);
            x64_copy_from_register_to_stack(&codegen->section_text, (codegen->stack_committed - expr->stack_offset) + 8, X64_RBX, 8);

            array_append(&codegen->patches, ((Patch) { .patch = patch_addr,
                                                       .instruction_offset = instruction_offset,
                                                       .string_offset = string_offset }));
        } break;

        case AST_KIND_IDENTIFIER:
        {
            assert(expr->decl);

            Ast *decl = expr->decl;

            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            expr->stack_offset = push_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            x64_copy_from_stack_to_stack(&codegen->section_text, codegen->stack_committed - expr->stack_offset,
                                         codegen->stack_committed - decl->stack_offset, datatype->size);
        } break;

        case AST_KIND_EXPRESSION_EQUAL:
        case AST_KIND_EXPRESSION_NOT_EQUAL:
        case AST_KIND_EXPRESSION_COMPARE_LESS:
        case AST_KIND_EXPRESSION_COMPARE_GREATER:
        case AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL:
        case AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL:
        {
            x64_emit_expression(compiler, codegen, expr->left_expr, target_platform);
            x64_emit_expression(compiler, codegen, expr->right_expr, target_platform);

            Datatype *left_datatype  = get_datatype(&compiler->datatypes, expr->left_expr->type_id);
            Datatype *right_datatype = get_datatype(&compiler->datatypes, expr->right_expr->type_id);

            u64 max_datatype_size = left_datatype->size;

            if (right_datatype->size > max_datatype_size)
            {
                max_datatype_size = right_datatype->size;
            }

            u64 left_stack_size = left_datatype->size;
            u64 right_stack_size = right_datatype->size;

            assert(expr->type_id == compiler->basetype_bool);
            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            assert(expr->right_expr->stack_offset == codegen->stack_allocated);
            assert(expr->left_expr->stack_offset == (codegen->stack_allocated - right_datatype->size));
            x64_copy_from_stack_to_register(&codegen->section_text, X64_RBX, codegen->stack_committed - expr->right_expr->stack_offset, right_datatype->size);
            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, codegen->stack_committed - expr->left_expr->stack_offset, left_datatype->size);

            pop_stack(codegen, right_stack_size);
            pop_stack(codegen, left_stack_size);

            // TODO: maybe sign extend arguments

            expr->stack_offset = push_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            s64 dst_stack_offset = codegen->stack_committed - expr->stack_offset;

            x64_compare_registers(&codegen->section_text, X64_RAX, X64_RBX, max_datatype_size);

            bool is_signed = true; // TODO:

            if (is_signed)
            {
                if (expr->kind == AST_KIND_EXPRESSION_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x94, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_NOT_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x95, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS)
                {
                    x64_setcc(&codegen->section_text, 0x9C, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER)
                {
                    x64_setcc(&codegen->section_text, 0x9F, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x9E, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x9D, dst_stack_offset);
                }
            }
            else
            {
                if (expr->kind == AST_KIND_EXPRESSION_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x94, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_NOT_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x95, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS)
                {
                    x64_setcc(&codegen->section_text, 0x92, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER)
                {
                    x64_setcc(&codegen->section_text, 0x97, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_LESS_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x96, dst_stack_offset);
                }
                else if (expr->kind == AST_KIND_EXPRESSION_COMPARE_GREATER_EQUAL)
                {
                    x64_setcc(&codegen->section_text, 0x93, dst_stack_offset);
                }
            }
        } break;

        case AST_KIND_EXPRESSION_BINOP_ADD:
        case AST_KIND_EXPRESSION_BINOP_MINUS:
        {
            x64_emit_expression(compiler, codegen, expr->left_expr, target_platform);
            x64_emit_expression(compiler, codegen, expr->right_expr, target_platform);

            Datatype *left_datatype  = get_datatype(&compiler->datatypes, expr->left_expr->type_id);
            Datatype *right_datatype = get_datatype(&compiler->datatypes, expr->right_expr->type_id);

            u64 left_stack_size = left_datatype->size;
            u64 right_stack_size = right_datatype->size;

            assert(expr->type_id);
            Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

            u64 stack_size = datatype->size;

            assert(expr->right_expr->stack_offset == codegen->stack_allocated);
            assert(expr->left_expr->stack_offset == (codegen->stack_allocated - right_datatype->size));
            x64_copy_from_stack_to_register(&codegen->section_text, X64_RBX, codegen->stack_committed - expr->right_expr->stack_offset, right_datatype->size);
            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, codegen->stack_committed - expr->left_expr->stack_offset, left_datatype->size);

            pop_stack(codegen, right_stack_size);
            pop_stack(codegen, left_stack_size);

            // TODO: maybe sign extend arguments

            expr->stack_offset = push_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            if (expr->kind == AST_KIND_EXPRESSION_BINOP_ADD)
            {
                x64_add_registers(&codegen->section_text, X64_RAX, X64_RBX, datatype->size);
            }
            else
            {
                assert(expr->kind == AST_KIND_EXPRESSION_BINOP_MINUS);
                x64_subtract_registers(&codegen->section_text, X64_RAX, X64_RBX, datatype->size);
            }

            x64_copy_from_register_to_stack(&codegen->section_text, codegen->stack_committed - expr->stack_offset, X64_RAX, datatype->size);
        } break;

        case AST_KIND_FUNCTION_CALL:
        {
            assert(expr->left_expr);

            Ast *left = expr->left_expr;

            if (left->kind == AST_KIND_IDENTIFIER)
            {
                assert(expr->decl);

                Datatype *return_type = get_datatype(&compiler->datatypes, expr->decl->type_id);

                u64 return_type_stack_size = return_type->size;

                expr->stack_offset = push_stack(codegen, return_type_stack_size);
            }
            else
            {
                assert(!"not implemented");
            }

            u64 arguments_stack_size = 0;

            // TODO: forward
            ForReversed(argument, expr->children.last)
            {
                // TODO: does the size match the expression?
                x64_emit_expression(compiler, codegen, argument, target_platform);

                Datatype *datatype = get_datatype(&compiler->datatypes, argument->type_id);

                u64 stack_size = datatype->size;
                arguments_stack_size += stack_size;
            }

            if (left->kind == AST_KIND_IDENTIFIER)
            {
                if (strings_are_equal(left->name, S("exit")))
                {
                    if ((target_platform == JulsPlatformAndroid) ||
                        (target_platform == JulsPlatformLinux))
                    {
                        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 60);
                        x64_copy_from_stack_to_register(&codegen->section_text, X64_RDI, 0, 4);
                        x64_syscall(&codegen->section_text);
                    }
                    else if (target_platform == JulsPlatformMacOs)
                    {
                        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 0x02000001);
                        x64_copy_from_stack_to_register(&codegen->section_text, X64_RDI, 0, 4);
                        x64_syscall(&codegen->section_text);
                    }
                }
                else if (strings_are_equal(left->name, S("write")))
                {
                    if ((target_platform == JulsPlatformAndroid) ||
                        (target_platform == JulsPlatformLinux))
                    {
                        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 1);
                        x64_copy_from_stack_to_register(&codegen->section_text, X64_RDI, 0, 4);
                        x64_copy_from_stack_to_register(&codegen->section_text, X64_RSI, 4, 8);
                        x64_copy_from_stack_to_register(&codegen->section_text, X64_RDX, 12, 8);
                        x64_syscall(&codegen->section_text);
                    }
                    else if (target_platform == JulsPlatformMacOs)
                    {
                        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 0x02000004);
                        x64_copy_from_stack_to_register(&codegen->section_text, X64_RDI, 0, 4);
                        x64_copy_from_stack_to_register(&codegen->section_text, X64_RSI, 4, 8);
                        x64_copy_from_stack_to_register(&codegen->section_text, X64_RDX, 12, 8);
                        x64_syscall(&codegen->section_text);
                    }
                }
                else
                {
                    assert(expr->decl);

                    string_builder_append_u8(&codegen->section_text, 0xE8);

                    if (expr->decl->address == S64MAX)
                    {
                        void *patch_addr = string_builder_append_size(&codegen->section_text, 4);
                        u64 instruction_offset = string_builder_get_size(&codegen->section_text);

                        array_append(&codegen->function_call_patches,
                                     ((FunctionCallPatch) { .patch = patch_addr,
                                                            .instruction_offset = instruction_offset,
                                                            .function_decl = expr->decl }));
                    }
                    else
                    {
                        s64 jump_offset = string_builder_get_size(&codegen->section_text) + 4;
                        string_builder_append_u32le(&codegen->section_text, (u32) (expr->decl->address - jump_offset));
                    }

                }
            }
            else
            {
                assert(!"not implemented");
            }

            pop_stack(codegen, arguments_stack_size);
            x64_commit_stack(codegen, &codegen->section_text);
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

            if (expr->kind == AST_KIND_ASSIGN)
            {
                x64_emit_expression(compiler, codegen, expr->right_expr, target_platform);
                assert(expr->right_expr->stack_offset == codegen->stack_allocated);
                x64_copy_from_stack_to_stack(&codegen->section_text, codegen->stack_committed - decl->stack_offset,
                                             codegen->stack_committed - expr->right_expr->stack_offset, datatype->size);

                expr->stack_offset = expr->right_expr->stack_offset;
            }
            else
            {
                x64_emit_expression(compiler, codegen, expr->right_expr, target_platform);
                assert(expr->right_expr->stack_offset == codegen->stack_allocated);
                x64_copy_from_stack_to_register(&codegen->section_text, X64_RBX, codegen->stack_committed - expr->right_expr->stack_offset, right_datatype->size);
                x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, codegen->stack_committed - decl->stack_offset, datatype->size);

                expr->stack_offset = expr->right_expr->stack_offset;

                // TODO: maybe sign extend arguments

                if (expr->kind == AST_KIND_PLUS_ASSIGN)
                {
                    x64_add_registers(&codegen->section_text, X64_RAX, X64_RBX, datatype->size);
                }
                else
                {
                    assert(expr->kind == AST_KIND_MINUS_ASSIGN);
                    x64_subtract_registers(&codegen->section_text, X64_RAX, X64_RBX, datatype->size);
                }

                x64_copy_from_register_to_stack(&codegen->section_text, codegen->stack_committed - decl->stack_offset, X64_RAX, datatype->size);
                x64_copy_from_register_to_stack(&codegen->section_text, codegen->stack_committed - expr->stack_offset, X64_RAX, datatype->size);
            }
        } break;

        case AST_KIND_MEMBER:
        {
            if (expr->left_expr->type_id == compiler->basetype_string)
            {
                if (strings_are_equal(expr->name, S("count")) ||
                    strings_are_equal(expr->name, S("data")))
                {
                    Datatype *left_datatype = get_datatype(&compiler->datatypes, expr->left_expr->type_id);
                    Datatype *datatype = get_datatype(&compiler->datatypes, expr->type_id);

                    u64 left_stack_size = left_datatype->size;
                    u64 stack_size = datatype->size;

                    expr->stack_offset = push_stack(codegen, stack_size);
                    x64_commit_stack(codegen, &codegen->section_text);

                    x64_emit_expression(compiler, codegen, expr->left_expr, target_platform);

                    if (strings_are_equal(expr->name, S("count")))
                    {
                        x64_copy_from_stack_to_stack(&codegen->section_text, codegen->stack_committed - expr->stack_offset,
                                                     (codegen->stack_committed - expr->left_expr->stack_offset) + 0, datatype->size);
                    }
                    else if (strings_are_equal(expr->name, S("data")))
                    {
                        x64_copy_from_stack_to_stack(&codegen->section_text, codegen->stack_committed - expr->stack_offset,
                                                     (codegen->stack_committed - expr->left_expr->stack_offset) + 8, datatype->size);
                    }

                    pop_stack(codegen, left_stack_size);
                    x64_commit_stack(codegen, &codegen->section_text);
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
            x64_emit_expression(compiler, codegen, expr->left_expr, target_platform);
            x64_emit_cast(compiler, codegen, expr->left_expr->type_id, expr->type_id);

            // TODO: hack
            expr->stack_offset = codegen->stack_allocated;
        } break;

        default:
        {
            printf("kind = %u\n", expr->kind);
            assert(!"expression not supported");
        } break;
    }
}

static void
x64_emit_statement(Compiler *compiler, Codegen *codegen, Ast *statement, JulsPlatform target_platform,
                   Datatype *return_type, s64 return_value_stack_offset)
{
    switch (statement->kind)
    {
        case AST_KIND_VARIABLE_DECLARATION:
        {
            Datatype *datatype = get_datatype(&compiler->datatypes, statement->type_id);

            statement->stack_offset = allocate_stack(codegen, datatype->size);

            if (statement->right_expr)
            {
                Datatype *right_datatype = get_datatype(&compiler->datatypes, statement->right_expr->type_id);

                u64 right_stack_size = right_datatype->size;

                x64_emit_expression(compiler, codegen, statement->right_expr, target_platform);

                assert(statement->right_expr->stack_offset == codegen->stack_allocated);
                // TODO: does the size match the expression?
                x64_copy_from_stack_to_stack(&codegen->section_text, codegen->stack_committed - statement->stack_offset,
                                             codegen->stack_committed - statement->right_expr->stack_offset, datatype->size);

                pop_stack(codegen, right_stack_size);
                x64_commit_stack(codegen, &codegen->section_text);
            }
        } break;

        case AST_KIND_IF:
        {
            x64_emit_expression(compiler, codegen, statement->left_expr, target_platform);

            assert(statement->left_expr->type_id == compiler->basetype_bool);
            Datatype *datatype = get_datatype(&compiler->datatypes, statement->left_expr->type_id);

            u64 stack_size = datatype->size;

            assert(statement->left_expr->stack_offset == codegen->stack_allocated);
            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, codegen->stack_committed - statement->left_expr->stack_offset, datatype->size);

            pop_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

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

            push_scope(codegen);

            x64_emit_statement(compiler, codegen, if_code, target_platform, return_type, return_value_stack_offset);

            pop_scope(codegen);
            x64_commit_stack(codegen, &codegen->section_text);

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
                push_scope(codegen);

                x64_emit_statement(compiler, codegen, else_code, target_platform, return_type, return_value_stack_offset);

                pop_scope(codegen);
                x64_commit_stack(codegen, &codegen->section_text);

                s64 end_target = string_builder_get_size(&codegen->section_text);
                *end_patch = (s32) (end_target - end_offset);
            }
        } break;

        case AST_KIND_FOR:
        {
            push_scope(codegen);

            x64_emit_statement(compiler, codegen, statement->decl, target_platform, return_type, return_value_stack_offset);

            s64 start_target = string_builder_get_size(&codegen->section_text);

            x64_emit_expression(compiler, codegen, statement->left_expr, target_platform);

            assert(statement->left_expr->type_id == compiler->basetype_bool);
            Datatype *datatype = get_datatype(&compiler->datatypes, statement->left_expr->type_id);

            u64 stack_size = datatype->size;

            assert(statement->left_expr->stack_offset == codegen->stack_allocated);
            x64_copy_from_stack_to_register(&codegen->section_text, X64_RAX, codegen->stack_committed - statement->left_expr->stack_offset, datatype->size);

            pop_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            x64_compare_al_to_zero(&codegen->section_text, X64_RAX);

            // JE
            string_builder_append_u8(&codegen->section_text, 0x0F);
            string_builder_append_u8(&codegen->section_text, 0x84);
            s32 *end_patch = string_builder_append_size(&codegen->section_text, 4);
            s64 end_offset = string_builder_get_size(&codegen->section_text);

            x64_emit_statement(compiler, codegen, statement->children.first, target_platform, return_type, return_value_stack_offset);
            x64_emit_expression(compiler, codegen, statement->right_expr, target_platform);

            Datatype *right_datatype = get_datatype(&compiler->datatypes, statement->right_expr->type_id);

            u64 right_stack_size = right_datatype->size;

            pop_stack(codegen, right_stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            string_builder_append_u8(&codegen->section_text, 0xE9);
            s32 *start_patch = string_builder_append_size(&codegen->section_text, 4);
            s64 start_offset = string_builder_get_size(&codegen->section_text);
            *start_patch = (start_target - start_offset);

            s64 end_target = string_builder_get_size(&codegen->section_text);
            *end_patch = (s32) (end_target - end_offset);

            pop_scope(codegen);
            x64_commit_stack(codegen, &codegen->section_text);
        } break;

        case AST_KIND_RETURN:
        {
            assert(statement->left_expr);

            Datatype *datatype = get_datatype(&compiler->datatypes, statement->left_expr->type_id);

            u64 stack_size = datatype->size;

            x64_emit_expression(compiler, codegen, statement->left_expr, target_platform);

            assert(statement->left_expr->stack_offset == codegen->stack_allocated);
            x64_copy_from_stack_to_stack(&codegen->section_text, codegen->stack_committed - return_value_stack_offset,
                                         codegen->stack_committed - statement->left_expr->stack_offset, return_type->size);

            pop_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);

            assert(codegen->stack_scope_index > 0);
            assert(codegen->stack_allocated == codegen->stack_scopes[codegen->stack_scope_index]);

            if (codegen->stack_committed > 0)
            {
                assert(codegen->stack_committed <= 0xFFFFFFFF);
                x64_add_immediate32_unsigned_to_register(&codegen->section_text, X64_RSP, (u32) codegen->stack_committed);
            }

            x64_ret(&codegen->section_text);
        } break;

        case AST_KIND_BLOCK:
        {
            push_scope(codegen);

            For(stmt, statement->children.first)
            {
                x64_emit_statement(compiler, codegen, stmt, target_platform, return_type, return_value_stack_offset);
            }

            pop_scope(codegen);
            x64_commit_stack(codegen, &codegen->section_text);
        } break;

        default:
        {
            x64_emit_expression(compiler, codegen, statement, target_platform);

            Datatype *datatype = get_datatype(&compiler->datatypes, statement->type_id);
            u64 stack_size = datatype->size;

            pop_stack(codegen, stack_size);
            x64_commit_stack(codegen, &codegen->section_text);
        } break;
    }
}

static void
x64_emit_function(Compiler *compiler, Codegen *codegen, Ast *func, JulsPlatform target_platform)
{
    assert(func->kind == AST_KIND_FUNCTION_DECLARATION);

    func->address = string_builder_get_size(&codegen->section_text);

    codegen->stack_allocated = 0;
    codegen->stack_committed = 0;
    codegen->stack_scopes[0] = 0;
    codegen->stack_scope_index = 0;

    s64 stack_offset = 0;

    // that's the return address
    stack_offset -= 8;

    // TODO: reverse
    For(parameter, func->parameters.last)
    {
        Datatype *datatype = get_datatype(&compiler->datatypes, parameter->type_id);
        u64 stack_size = datatype->size;

        parameter->stack_offset = stack_offset;
        stack_offset -= stack_size;
    }

    Datatype *return_type = get_datatype(&compiler->datatypes, func->type_id);

    s64 return_value_stack_offset = stack_offset;

    push_scope(codegen);

    For(statement, func->children.first)
    {
        x64_emit_statement(compiler, codegen, statement, target_platform, return_type, return_value_stack_offset);
    }

    pop_scope(codegen);
    assert(codegen->stack_scope_index == 0);

    if (!func->type_def)
    {
        x64_commit_stack(codegen, &codegen->section_text);
        x64_ret(&codegen->section_text);
    }
}

static void
generate_x64(Compiler *compiler, Codegen *codegen, SymbolTable *symbol_table, JulsPlatform target_platform)
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
    else if (target_platform == JulsPlatformWindows)
    {
        // call main
        string_builder_append_u8(&codegen->section_text, 0xE8);
        jump_patch = string_builder_append_size(&codegen->section_text, 4);
        jump_location = string_builder_get_size(&codegen->section_text);

        // mov rax, 42
        x64_move_immediate32_unsigned_into_register(&codegen->section_text, X64_RAX, 42);

        // ret
        x64_ret(&codegen->section_text);
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

    For(decl, compiler->global_declarations.children.first)
    {
        if (decl->kind == AST_KIND_FUNCTION_DECLARATION)
        {
            u64 offset = string_builder_get_size(&codegen->section_text);

            if (strings_are_equal(entry_point_name, decl->name))
            {
                jump_target = offset;
            }

            x64_emit_function(compiler, codegen, decl, target_platform);

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

    for (s32 i = 0; i < codegen->function_call_patches.count; i += 1)
    {
        FunctionCallPatch *patch = codegen->function_call_patches.items + i;
        Ast *function_decl = patch->function_decl;

        assert(function_decl->address != S64MAX);

        *(s32 *) patch->patch = (s32) (function_decl->address - patch->instruction_offset);
    }
}
