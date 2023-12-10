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

            arm64_store_register(code, ARM64_R30, ARM64_SP, -16);
            arm64_load_register(code, ARM64_R30, ARM64_SP, 16);
            arm64_ret(code);

            u64 size = string_builder_get_size(code) - offset;

            array_append(symbol_table, ((SymbolEntry) { .name = elem->name, .offset = offset, .size = size }));
        }

        elem = elem->next;
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
