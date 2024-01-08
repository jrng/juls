#define JULS_PLATFORM_ANDROID 0
#define JULS_PLATFORM_WINDOWS 0
#define JULS_PLATFORM_LINUX 0
#define JULS_PLATFORM_MACOS 0

#if defined(__ANDROID__)
#  undef JULS_PLATFORM_ANDROID
#  define JULS_PLATFORM_ANDROID 1
#elif defined(_WIN32)
#  undef JULS_PLATFORM_WINDOWS
#  define JULS_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#  undef JULS_PLATFORM_LINUX
#  define JULS_PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#  undef JULS_PLATFORM_MACOS
#  define JULS_PLATFORM_MACOS 1
#endif

#define JULS_ARCHITECTURE_ARM64 0
#define JULS_ARCHITECTURE_X86_64 0

#if JULS_PLATFORM_WINDOWS
#  if defined(_M_AMD64)
#    undef JULS_ARCHITECTURE_X86_64
#    define JULS_ARCHITECTURE_X86_64 1
#  endif
#elif JULS_PLATFORM_ANDROID || JULS_PLATFORM_LINUX || JULS_PLATFORM_MACOS
#  if defined(__x86_64__)
#    undef JULS_ARCHITECTURE_X86_64
#    define JULS_ARCHITECTURE_X86_64 1
#  elif defined(__aarch64__)
#    undef JULS_ARCHITECTURE_ARM64
#    define JULS_ARCHITECTURE_ARM64 1
#  endif
#endif

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define S32MIN ((s32) 0x80000000)
#define S32MAX ((s32) 0x7FFFFFFF)

#define S64MIN ((s64) 0x8000000000000000)
#define S64MAX ((s64) 0x7FFFFFFFFFFFFFFF)

#define U64MAX ((u64) 0xFFFFFFFFFFFFFFFF)

#define Align(value, alignment) (((value) + (alignment) - (s64) 1) & ~((alignment) - (s64) 1))
#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct File File;

typedef enum
{
    FILE_MODE_READ  = (1 << 0),
    FILE_MODE_WRITE = (1 << 1),
} FileMode;

typedef enum
{
    FILE_PERMISSION_READABLE    = (1 << 0),
    FILE_PERMISSION_WRITEABLE   = (1 << 1),
    FILE_PERMISSION_EXECUTABLE  = (1 << 2),
} FilePermission;

static inline void *allocate(u64 size);
static inline void deallocate(void *ptr, u64 size);

#include "allocator.c"

static Allocator default_allocator;

#define array_append(array, item)                                                                                                           \
    do {                                                                                                                                    \
        if ((array)->count >= (array)->allocated)                                                                                           \
        {                                                                                                                                   \
            (array)->items     = realloc(&default_allocator, (array)->items,                                                                \
                                         (array)->allocated * sizeof(*(array)->items),                                                      \
                                         (((array)->allocated == 0) ? 16 : 2 * (array)->allocated) * sizeof(*(array)->items), 8, false);    \
            (array)->allocated = ((array)->allocated == 0) ? 16 : 2 * (array)->allocated;                                                   \
        }                                                                                                                                   \
        (array)->items[(array)->count++] = (item);                                                                                          \
    } while (0);

#include "strings.c"
#include "lexer.c"
#include "ast.c"
#include "parser.c"
#include "type_checking.c"

#if JULS_PLATFORM_ANDROID
#  include "unix.c"
#elif JULS_PLATFORM_WINDOWS
#elif JULS_PLATFORM_LINUX
#  include "unix.c"
#elif JULS_PLATFORM_MACOS
#  include "unix.c"
#  include <sys/wait.h>
#endif

static const u32 JULS_VERSION_MAJOR = 1;
static const u32 JULS_VERSION_MINOR = 0;
static const u32 JULS_VERSION_BUILD = 0;

typedef enum
{
    JulsPlatformAndroid = 0,
    JulsPlatformWindows = 1,
    JulsPlatformLinux   = 2,
    JulsPlatformMacOs   = 3,
} JulsPlatform;

typedef enum
{
    JulsArchitectureArm64  = 0,
    JulsArchitectureX86_64 = 1,
} JulsArchitecture;

static const String platform_names[] = {
    S("Android"),
    S("Windows"),
    S("Linux"),
    S("macOs"),
};

static const String architecture_names[] = {
    S("Arm64"),
    S("x86_64"),
};

#if JULS_PLATFORM_ANDROID
static JulsPlatform default_platform = JulsPlatformAndroid;
#elif JULS_PLATFORM_WINDOWS
static JulsPlatform default_platform = JulsPlatformWindows;
#elif JULS_PLATFORM_LINUX
static JulsPlatform default_platform = JulsPlatformLinux;
#elif JULS_PLATFORM_MACOS
static JulsPlatform default_platform = JulsPlatformMacOs;
#endif

#if JULS_ARCHITECTURE_ARM64
static JulsArchitecture default_architecture = JulsArchitectureArm64;
#elif JULS_ARCHITECTURE_X86_64
static JulsArchitecture default_architecture = JulsArchitectureX86_64;
#endif

#define MAX_STRING_BUFFER_COUNT 1024

typedef struct StringBuffer
{
    struct StringBuffer *next;
    s64 count;
    u8 data[MAX_STRING_BUFFER_COUNT];
} StringBuffer;

typedef struct
{
    StringBuffer *first_buffer;
    StringBuffer *current_buffer;
    Allocator *allocator;
} StringBuilder;

static void
initialize_string_builder(StringBuilder *builder, Allocator *allocator)
{
    builder->allocator = allocator;

    StringBuffer *buffer = alloc_type(builder->allocator, StringBuffer, 16, false);

    buffer->next = 0;
    buffer->count = 0;

    builder->first_buffer = buffer;
    builder->current_buffer = buffer;
}

static s64
string_builder_get_size(StringBuilder *builder)
{
    s64 result = 0;

    StringBuffer *buffer = builder->first_buffer;

    while (buffer)
    {
        result += buffer->count;
        buffer = buffer->next;
    }

    return result;
}

static void
string_builder_ensure_space(StringBuilder *builder, s64 size)
{
    if (!builder->first_buffer || ((builder->current_buffer->count + size) > MAX_STRING_BUFFER_COUNT))
    {
        StringBuffer *buffer = alloc_type(builder->allocator, StringBuffer, 16, false);

        buffer->next = 0;
        buffer->count = 0;

        if (builder->first_buffer)
        {
            builder->current_buffer->next = buffer;
        }
        else
        {
            builder->first_buffer = buffer;
        }

        builder->current_buffer = buffer;
    }
}

static void
string_builder_append_string(StringBuilder *builder, String str)
{
    string_builder_ensure_space(builder, 1);

    u8 *bytes = str.data;
    s64 bytes_to_write = str.count;

    int64_t count = MAX_STRING_BUFFER_COUNT - builder->current_buffer->count;

    if (count > bytes_to_write)
    {
        count = bytes_to_write;
    }

    bytes_to_write -= count;
    u8 *dst = builder->current_buffer->data + builder->current_buffer->count;
    builder->current_buffer->count += count;

    while (count--)
    {
        *dst++ = *bytes++;
    }

    while (bytes_to_write)
    {
        // TODO: just add a new string buffer
        string_builder_ensure_space(builder, 1);

        count = MAX_STRING_BUFFER_COUNT;

        if (count > bytes_to_write)
        {
            count = bytes_to_write;
        }

        bytes_to_write -= count;
        dst = builder->current_buffer->data;
        builder->current_buffer->count = count;

        while (count--)
        {
            *dst++ = *bytes++;
        }
    }
}

static void
string_builder_append_u8(StringBuilder *builder, u8 value)
{
    string_builder_ensure_space(builder, 1);

    StringBuffer *buffer = builder->current_buffer;
    buffer->data[buffer->count] = value;
    buffer->count += 1;
}

static void *
string_builder_append_size(StringBuilder *builder, s64 size)
{
    string_builder_ensure_space(builder, size);

    StringBuffer *buffer = builder->current_buffer;
    void *result = buffer->data + buffer->count;
    buffer->count += size;

    return result;
}

// TODO: @Speed
static void
string_builder_append_u16le(StringBuilder *builder, u16 value)
{
    string_builder_append_u8(builder, (u8) (value >>  0));
    string_builder_append_u8(builder, (u8) (value >>  8));
}

// TODO: @Speed
static void
string_builder_append_u32le(StringBuilder *builder, u32 value)
{
    string_builder_append_u8(builder, (u8) (value >>  0));
    string_builder_append_u8(builder, (u8) (value >>  8));
    string_builder_append_u8(builder, (u8) (value >> 16));
    string_builder_append_u8(builder, (u8) (value >> 24));
}

// TODO: @Speed
static void
string_builder_append_u64le(StringBuilder *builder, u64 value)
{
    string_builder_append_u8(builder, (u8) (value >>  0));
    string_builder_append_u8(builder, (u8) (value >>  8));
    string_builder_append_u8(builder, (u8) (value >> 16));
    string_builder_append_u8(builder, (u8) (value >> 24));
    string_builder_append_u8(builder, (u8) (value >> 32));
    string_builder_append_u8(builder, (u8) (value >> 40));
    string_builder_append_u8(builder, (u8) (value >> 48));
    string_builder_append_u8(builder, (u8) (value >> 56));
}

static void
string_builder_append_builder(StringBuilder *builder, StringBuilder append)
{
    if (append.first_buffer)
    {
        if (builder->first_buffer)
        {
            builder->current_buffer->next = append.first_buffer;
            builder->current_buffer = append.current_buffer;
        }
        else
        {
            builder->first_buffer = append.first_buffer;
            builder->current_buffer = append.current_buffer;
        }
    }
}

static String
read_entire_file(Allocator *allocator, String filename)
{
    String result = { 0 };

    File *file = open_file(allocator, filename, FILE_MODE_READ);

    if (file)
    {
        result.count = get_file_size(file);
        result.data = alloc_array(allocator, u8, result.count, 8, false);

        read_file(file, result.data, 0, result.count);
        close_file(file);
    }

    return result;
}

typedef struct
{
    String name;
    u64 offset;
    u64 size;
} SymbolEntry;

typedef struct
{
    s32 count;
    s32 allocated;
    SymbolEntry *items;
} SymbolTable;

typedef struct
{
    void *patch;
    u64 instruction_offset;
    u64 string_offset;
} Patch;

typedef struct
{
    s32 count;
    s32 allocated;
    Patch *items;
} PatchArray;

typedef struct
{
    StringBuilder section_text;
    StringBuilder section_cstring;
    PatchArray patches;
} Codegen;

#include "arm64.c"
#include "x64.c"
#include "elf.c"
#include "macho.c"

static void
generate_code(Parser *parser, Codegen *codegen, SymbolTable *symbol_table, JulsPlatform target_platform, JulsArchitecture target_architecture)
{
    if (target_architecture == JulsArchitectureArm64)
    {
        generate_arm64(parser, codegen, symbol_table, target_platform);
    }
    else
    {
        assert(target_architecture == JulsArchitectureX86_64);

        generate_x64(parser, codegen, symbol_table, target_platform);
    }
}

int main(s32 argument_count, char **arguments)
{
    String input_filename = { 0 };
    String output_filename = { 0 };

    JulsPlatform target_platform = default_platform;
    JulsArchitecture target_architecture = default_architecture;

    for (s32 i = 1; i < argument_count; i += 1)
    {
        String argument = C(arguments[i]);

        if (strings_are_equal(argument, S("-o")))
        {
            i += 1;

            if (i < argument_count)
            {
                output_filename = C(arguments[i]);
            }
            else
            {
                // TODO: print error
            }
        }
        else if (strings_are_equal(argument, S("-h")) || strings_are_equal(argument, S("--help")))
        {
            fprintf(stderr, "USAGE: juls [options] file\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "OPTIONS:\n");
            fprintf(stderr, "  --architecture <name>   Set the target architecture. Valid architecture names are:\n");
            fprintf(stderr, "                            arm64, aarch64, amd64, x86_64, x86-64, x64\n");
            fprintf(stderr, "  -h, --help              List all available options\n");
            fprintf(stderr, "  -o <file>               Write output binary to <file>\n");
            fprintf(stderr, "  --platform <name>       Set the target platform. Valid platform names are:\n");
            fprintf(stderr, "                            android, windows, linux, macos\n");
            fprintf(stderr, "  --version               Print the compiler version\n");
            fprintf(stderr, "\n");

            return 0;
        }
        else if (strings_are_equal(argument, S("--version")))
        {
            String platform_name = platform_names[default_platform];
            String architecture_name = architecture_names[default_architecture];

            fprintf(stderr, "version: %u.%u.%u\n", JULS_VERSION_MAJOR, JULS_VERSION_MINOR, JULS_VERSION_BUILD);
            fprintf(stderr, "default platform: %.*s\n", (int) platform_name.count, platform_name.data);
            fprintf(stderr, "default architecture: %.*s\n", (int) architecture_name.count, architecture_name.data);

            return 0;
        }
        else if (strings_are_equal(argument, S("--platform")))
        {
            i += 1;

            if (i < argument_count)
            {
                String platform_name = C(arguments[i]);

                if (strings_are_equal(platform_name, S("android")))
                {
                    target_platform = JulsPlatformAndroid;
                }
                else if (strings_are_equal(platform_name, S("windows")))
                {
                    target_platform = JulsPlatformWindows;
                }
                else if (strings_are_equal(platform_name, S("linux")))
                {
                    target_platform = JulsPlatformLinux;
                }
                else if (strings_are_equal(platform_name, S("macos")))
                {
                    target_platform = JulsPlatformMacOs;
                }
                else
                {
                    fprintf(stderr, "error: unknown platform '%.*s'\n", (int) platform_name.count, platform_name.data);
                    fprintf(stderr, "  valid platform names are: android, windows, linux, macos\n");
                    return 0;
                }
            }
        }
        else if (strings_are_equal(argument, S("--architecture")))
        {
            i += 1;

            if (i < argument_count)
            {
                String architecture_name = C(arguments[i]);

                if (strings_are_equal(architecture_name, S("arm64")) ||
                    strings_are_equal(architecture_name, S("aarch64")))
                {
                    target_architecture = JulsArchitectureArm64;
                }
                else if (strings_are_equal(architecture_name, S("amd64")) ||
                         strings_are_equal(architecture_name, S("x86_64")) ||
                         strings_are_equal(architecture_name, S("x86-64")) ||
                         strings_are_equal(architecture_name, S("x64")))
                {
                    target_architecture = JulsArchitectureX86_64;
                }
                else
                {
                    fprintf(stderr, "error: unknown architecture '%.*s'\n", (int) architecture_name.count, architecture_name.data);
                    fprintf(stderr, "  valid architecture names are: arm64, aarch64, amd64, x86_64, x86-64, x64\n");
                    return 0;
                }
            }
        }
        else
        {
            input_filename = argument;
        }
    }

    if (!input_filename.count)
    {
        fprintf(stderr, "error: no input file.\n");
        return 0;
    }

    if (!output_filename.count)
    {
        // TODO: derive from input_filename
        output_filename = S("output");
    }

    String input_file = read_entire_file(&default_allocator, input_filename);

    Parser parser;
    parser.has_error = false;
    parser.lexer.start = 0;
    parser.lexer.current = 0;
    parser.lexer.input = input_file;
    parser.ast_nodes.first_bucket = 0;
    parser.ast_nodes.last_bucket = 0;
    parser.global_declarations.kind = AST_KIND_GLOBAL_SCOPE;
    parser.global_declarations.parent = 0;
    parser.global_declarations.children.first = 0;
    parser.global_declarations.children.last = 0;

    parser.datatypes.count = 0;
    parser.datatypes.allocated = 0;
    parser.datatypes.items = 0;

    // index 0 is the invalid datatype
    array_append(&parser.datatypes, ((Datatype) { 0 }));

    parser.basetype_void = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_VOID, .flags = 0, .ref = 0, .name = S("void"), .size = 0 }));

    parser.basetype_bool = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_BOOLEAN, .flags = 0, .ref = 0, .name = S("bool"), .size = 1 }));

    parser.basetype_s8 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_INTEGER, .flags = 0, .ref = 0, .name = S("s8"), .size = 1 }));
    parser.basetype_s16 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_INTEGER, .flags = 0, .ref = 0, .name = S("s16"), .size = 2 }));
    parser.basetype_s32 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_INTEGER, .flags = 0, .ref = 0, .name = S("s32"), .size = 4 }));
    parser.basetype_s64 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_INTEGER, .flags = 0, .ref = 0, .name = S("s64"), .size = 8 }));

    parser.basetype_u8 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_INTEGER, .flags = DATATYPE_FLAG_UNSIGNED, .ref = 0, .name = S("u8"), .size = 1 }));
    parser.basetype_u16 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_INTEGER, .flags = DATATYPE_FLAG_UNSIGNED, .ref = 0, .name = S("u16"), .size = 2 }));
    parser.basetype_u32 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_INTEGER, .flags = DATATYPE_FLAG_UNSIGNED, .ref = 0, .name = S("u32"), .size = 4 }));
    parser.basetype_u64 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_INTEGER, .flags = DATATYPE_FLAG_UNSIGNED, .ref = 0, .name = S("u64"), .size = 8 }));

    parser.basetype_f32 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_FLOAT, .flags = 0, .ref = 0, .name = S("f32"), .size = 4 }));
    parser.basetype_f64 = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_FLOAT, .flags = 0, .ref = 0, .name = S("f64"), .size = 8 }));
    parser.basetype_string = parser.datatypes.count;
    array_append(&parser.datatypes, ((Datatype) { .kind = DATATYPE_STRING, .flags = 0, .ref = 0, .name = S("string"), .size = 16 }));

    parse(&parser);
    type_checking(&parser);

    Codegen codegen;

    initialize_string_builder(&codegen.section_text, &default_allocator);
    initialize_string_builder(&codegen.section_cstring, &default_allocator);
    codegen.patches.count = 0;
    codegen.patches.allocated = 0;
    codegen.patches.items = 0;

    SymbolTable symbol_table = { 0 };

    generate_code(&parser, &codegen, &symbol_table, target_platform, target_architecture);

    StringBuilder builder;
    initialize_string_builder(&builder, &default_allocator);

    if ((target_platform == JulsPlatformAndroid) ||
        (target_platform == JulsPlatformLinux))
    {
        generate_elf(&builder, codegen, symbol_table, target_architecture);
    }
    else if (target_platform == JulsPlatformMacOs)
    {
        generate_macho(&builder, codegen, symbol_table, target_architecture);
    }

    File *output_file = create_file(&default_allocator, output_filename,
                                    FILE_PERMISSION_READABLE | FILE_PERMISSION_WRITEABLE | FILE_PERMISSION_EXECUTABLE);

    if (output_file)
    {
        u64 offset = 0;
        StringBuffer *buffer = builder.first_buffer;

        while (buffer)
        {
            write_file(output_file, buffer->data, offset, buffer->count);
            offset += buffer->count;
            buffer = buffer->next;
        }

        close_file(output_file);
    }

#if JULS_PLATFORM_MACOS
    if (target_platform == JulsPlatformMacOs)
    {
        pid_t pid = fork();

        if (pid > 0)
        {
            for (;;)
            {
                int status;

                if (waitpid(pid, &status, 0) < 0)
                {
                    break;
                }

                if (WIFEXITED(status) || WIFSIGNALED(status))
                {
                    break;
                }
            }
        }
        else if (pid == 0)
        {
            s32 ret = execlp("codesign", "codesign", "-s", "-", to_c_string(&default_allocator, output_filename), 0);
        }
    }
#endif

    free_all(&default_allocator);

    return 0;
}
