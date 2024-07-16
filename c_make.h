// c_make.h

// TODO:
// - support android as a platform
// - add logging api
// - documentation
// - improve c_make_command_to_string()
// - improve c_make_command_append_command_line()
// - support comments in config file

#ifndef __C_MAKE_INCLUDE__
#define __C_MAKE_INCLUDE__

#define C_MAKE_PLATFORM_ANDROID 0
#define C_MAKE_PLATFORM_WINDOWS 0
#define C_MAKE_PLATFORM_LINUX   0
#define C_MAKE_PLATFORM_MACOS   0
#define C_MAKE_PLATFORM_WEB     0

#if defined(__ANDROID__)
#  undef C_MAKE_PLATFORM_ANDROID
#  define C_MAKE_PLATFORM_ANDROID 1
#elif defined(_WIN32)
#  undef C_MAKE_PLATFORM_WINDOWS
#  define C_MAKE_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#  undef C_MAKE_PLATFORM_LINUX
#  define C_MAKE_PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#  undef C_MAKE_PLATFORM_MACOS
#  define C_MAKE_PLATFORM_MACOS 1
#elif defined(__wasm__)
#  undef C_MAKE_PLATFORM_WEB
#  define C_MAKE_PLATFORM_WEB 1
#endif

#define C_MAKE_ARCHITECTURE_AMD64   0
#define C_MAKE_ARCHITECTURE_AARCH64 0
#define C_MAKE_ARCHITECTURE_WASM32  0

#if C_MAKE_PLATFORM_WINDOWS
#  if defined(__MINGW32__)
#    if defined(__x86_64__)
#      undef C_MAKE_ARCHITECTURE_AMD64
#      define C_MAKE_ARCHITECTURE_AMD64 1
#    elif defined(__aarch64__)
#      undef C_MAKE_ARCHITECTURE_AARCH64
#      define C_MAKE_ARCHITECTURE_AARCH64 1
#    endif
#  else
#    if defined(_M_AMD64)
#      undef C_MAKE_ARCHITECTURE_AMD64
#      define C_MAKE_ARCHITECTURE_AMD64 1
#    endif
#  endif
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
#  if defined(__x86_64__)
#    undef C_MAKE_ARCHITECTURE_AMD64
#    define C_MAKE_ARCHITECTURE_AMD64 1
#  elif defined(__aarch64__)
#    undef C_MAKE_ARCHITECTURE_AARCH64
#    define C_MAKE_ARCHITECTURE_AARCH64 1
#  endif
#elif C_MAKE_PLATFORM_WEB
#  if defined(__wasm__)
#    undef C_MAKE_ARCHITECTURE_WASM32
#    define C_MAKE_ARCHITECTURE_WASM32
#  endif
#endif

#define _CMakeStr(str) #str
#define CMakeStr(str) _CMakeStr(str)

#define CMakeArrayCount(array) (sizeof(array)/sizeof((array)[0]))

#define CMakeNArgs(...) __CMakeNArgs(_CMakeNArgs(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define __CMakeNArgs(x) x
#define _CMakeNArgs(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, n, ...) n

#define CMakeStringLiteral(str) c_make_make_string((char *) (str), sizeof(str) - 1)
#define CMakeCString(str) c_make_make_string((char *) (str), c_make_get_c_string_length(str))

#define c_make_string_concat(...) c_make_string_concat_va(CMakeNArgs(__VA_ARGS__), __VA_ARGS__)

#define c_make_c_string_concat(...) c_make_c_string_concat_va(CMakeNArgs(__VA_ARGS__), __VA_ARGS__)
#define c_make_c_string_path_concat(...) c_make_c_string_path_concat_va(CMakeNArgs(__VA_ARGS__), __VA_ARGS__)

#define c_make_command_append(command, ...) c_make_command_append_va(command, CMakeNArgs(__VA_ARGS__), __VA_ARGS__ )

#if defined(C_MAKE_STATIC)
#  define C_MAKE_DEF static
#else
#  define C_MAKE_DEF extern
#endif

#if C_MAKE_PLATFORM_WINDOWS

#  define UNICODE
#  define _UNICODE
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

typedef HANDLE CMakeProcessId;

#  define CMakeInvalidProcessId INVALID_HANDLE_VALUE

#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS

#  include <sys/wait.h>

typedef pid_t CMakeProcessId;

#  define CMakeInvalidProcessId (-1)

#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#  include <stdbool.h>
#endif

typedef enum CMakeTarget
{
    CMakeTargetSetup   = 0,
    CMakeTargetBuild   = 1,
    CMakeTargetInstall = 2,
} CMakeTarget;

#if !defined(C_MAKE_NO_ENTRY_POINT)

#  define C_MAKE_ENTRY() void _c_make_entry_(CMakeTarget c_make_target)

C_MAKE_ENTRY();

#endif // !defined(C_MAKE_NO_ENTRY_POINT)

typedef enum CMakeLogLevel
{
    CMakeLogLevelRaw     = 0,
    CMakeLogLevelInfo    = 1,
    CMakeLogLevelWarning = 2,
    CMakeLogLevelError   = 3,
} CMakeLogLevel;

typedef enum CMakePlatform
{
    CMakePlatformAndroid = 0,
    CMakePlatformWindows = 1,
    CMakePlatformLinux   = 2,
    CMakePlatformMacOs   = 3,
    CMakePlatformWeb     = 4,
} CMakePlatform;

typedef enum CMakeArchitecture
{
    CMakeArchitectureAmd64   = 0,
    CMakeArchitectureAarch64 = 1,
    CMakeArchitectureWasm32  = 2,
} CMakeArchitecture;

typedef enum CMakeBuildType
{
    CMakeBuildTypeDebug    = 0,
    CMakeBuildTypeRelDebug = 1,
    CMakeBuildTypeRelease  = 2,
} CMakeBuildType;

typedef struct CMakeMemory
{
    size_t used;
    size_t allocated;
    void *base;
} CMakeMemory;

typedef struct CMakeString
{
    size_t count;
    char *data;
} CMakeString;

#define CMakeStringFmt ".*s"
#define CMakeStringArg(str) (int) (str).count, (str).data

typedef struct CMakeCommand
{
    size_t count;
    size_t allocated;
    const char **items;
} CMakeCommand;

typedef struct CMakeConfigValue
{
    bool is_valid;
    const char *val;
} CMakeConfigValue;

typedef struct CMakeConfigEntry
{
    CMakeString key;
    CMakeString value;
} CMakeConfigEntry;

typedef struct CMakeConfig
{
    size_t count;
    size_t allocated;
    CMakeConfigEntry *items;
} CMakeConfig;

typedef struct CMakeContext
{
    bool verbose;

    CMakePlatform target_platform;
    CMakeArchitecture target_architecture;
    CMakeBuildType build_type;

    const char *build_path;
    const char *source_path;

    CMakeConfig config;
    CMakeMemory private_memory;
    CMakeMemory public_memory;

    bool shell_initialized;

    const char *reset;
    const char *color_black;
    const char *color_red;
    const char *color_green;
    const char *color_yellow;
    const char *color_blue;
    const char *color_magenta;
    const char *color_cyan;
    const char *color_white;
    const char *color_bright_black;
    const char *color_bright_red;
    const char *color_bright_green;
    const char *color_bright_yellow;
    const char *color_bright_blue;
    const char *color_bright_magenta;
    const char *color_bright_cyan;
    const char *color_bright_white;
} CMakeContext;

static inline CMakeString
c_make_make_string(void *data, size_t count)
{
    CMakeString result;
    result.count = count;
    result.data = (char *) data;
    return result;
}

static inline size_t
c_make_get_c_string_length(const char *str)
{
    size_t length = 0;
    while (*str++) length += 1;
    return length;
}

static inline CMakePlatform
c_make_get_host_platform(void)
{
#if C_MAKE_PLATFORM_ANDROID
    return CMakePlatformAndroid;
#elif C_MAKE_PLATFORM_WINDOWS
    return CMakePlatformWindows;
#elif C_MAKE_PLATFORM_LINUX
    return CMakePlatformLinux;
#elif C_MAKE_PLATFORM_MACOS
    return CMakePlatformMacOs;
#elif C_MAKE_PLATFORM_WEB
    return CMakePlatformWeb;
#endif
}

static inline CMakeArchitecture
c_make_get_host_architecture(void)
{
#if C_MAKE_ARCHITECTURE_AMD64
    return CMakeArchitectureAmd64;
#elif C_MAKE_ARCHITECTURE_AARCH64
    return CMakeArchitectureAarch64;
#elif C_MAKE_ARCHITECTURE_WASM32
    return CMakeArchitectureWasm32;
#endif
}

static inline const char *
c_make_get_platform_name(CMakePlatform platform)
{
    const char *name = "";

    switch (platform)
    {
        case CMakePlatformAndroid: name = "android"; break;
        case CMakePlatformWindows: name = "windows"; break;
        case CMakePlatformLinux:   name = "linux";   break;
        case CMakePlatformMacOs:   name = "macos";   break;
        case CMakePlatformWeb:     name = "web";     break;
    }

    return name;
}

static inline const char *
c_make_get_architecture_name(CMakeArchitecture architecture)
{
    const char *name = "";

    switch (architecture)
    {
        case CMakeArchitectureAmd64:   name = "amd64";   break;
        case CMakeArchitectureAarch64: name = "aarch64"; break;
        case CMakeArchitectureWasm32:  name = "wasm32";  break;
    }

    return name;
}

static inline bool
c_make_compiler_is_msvc(const char *compiler)
{
    return (compiler && (compiler[0] == 'c') && (compiler[1] == 'l') && (compiler[2] == 0)) ? true : false;
}

C_MAKE_DEF void c_make_log(CMakeLogLevel log_level, const char *format, ...);

C_MAKE_DEF void *c_make_memory_allocate(CMakeMemory *memory, size_t size);
C_MAKE_DEF void *c_make_memory_reallocate(CMakeMemory *memory, void *old_ptr, size_t old_size, size_t new_size);
C_MAKE_DEF size_t c_make_memory_get_used(CMakeMemory *memory);
C_MAKE_DEF void c_make_memory_set_used(CMakeMemory *memory, size_t used);

C_MAKE_DEF void *c_make_allocate(size_t size);
C_MAKE_DEF size_t c_make_memory_save(void);
C_MAKE_DEF void c_make_memory_restore(size_t saved);

C_MAKE_DEF void c_make_command_append_va(CMakeCommand *command, size_t count, ...);
C_MAKE_DEF void c_make_command_append_slice(CMakeCommand *command, size_t count, const char **items);
C_MAKE_DEF void c_make_command_append_command_line(CMakeCommand *command, const char *str);
C_MAKE_DEF CMakeString c_make_command_to_string(CMakeCommand command);

C_MAKE_DEF bool c_make_strings_are_equal(CMakeString a, CMakeString b);
C_MAKE_DEF CMakeString c_make_copy_string(CMakeMemory *memory, CMakeString str);
C_MAKE_DEF CMakeString c_make_string_split_left(CMakeString *str, char c);
C_MAKE_DEF CMakeString c_make_string_trim(CMakeString str);
C_MAKE_DEF size_t c_make_string_find(CMakeString str, CMakeString pattern);
C_MAKE_DEF char *c_make_string_to_c_string(CMakeMemory *memory, CMakeString str);

C_MAKE_DEF CMakePlatform c_make_get_target_platform(void);
C_MAKE_DEF CMakeArchitecture c_make_get_target_architecture(void);
C_MAKE_DEF CMakeBuildType c_make_get_build_type(void);
C_MAKE_DEF const char *c_make_get_build_path(void);
C_MAKE_DEF const char *c_make_get_source_path(void);
C_MAKE_DEF const char *c_make_get_install_prefix(void);

C_MAKE_DEF const char *c_make_get_host_c_compiler(void);
C_MAKE_DEF const char *c_make_get_target_c_compiler(void);
C_MAKE_DEF const char *c_make_get_target_c_flags(void);
C_MAKE_DEF const char *c_make_get_host_cpp_compiler(void);
C_MAKE_DEF const char *c_make_get_target_cpp_compiler(void);
C_MAKE_DEF const char *c_make_get_target_cpp_flags(void);

C_MAKE_DEF void c_make_config_set(const char *key, const char *value);
C_MAKE_DEF CMakeConfigValue c_make_config_get(const char *key);

C_MAKE_DEF bool c_make_store_config(const char *file_name);

C_MAKE_DEF bool c_make_needs_rebuild(const char *output_file, size_t input_file_count, const char **input_files);
C_MAKE_DEF bool c_make_needs_rebuild_single_source(const char *output_file, const char *input_file);

C_MAKE_DEF bool c_make_file_exists(const char *file_name);
C_MAKE_DEF bool c_make_directory_exists(const char *directory_name);
C_MAKE_DEF bool c_make_create_directory(const char *directory_name);
C_MAKE_DEF bool c_make_read_entire_file(const char *file_name, CMakeString *content);
C_MAKE_DEF bool c_make_write_entire_file(const char *file_name, CMakeString content);
C_MAKE_DEF bool c_make_copy_file(const char *src_file, const char *dst_file);
C_MAKE_DEF bool c_make_rename_file(const char *old_file_name, const char *new_file_name);
C_MAKE_DEF bool c_make_delete_file(const char *file_name);

C_MAKE_DEF bool c_make_has_slash_or_backslash(const char *path);
C_MAKE_DEF const char *c_make_find_program(const char *program_name);

C_MAKE_DEF CMakeString c_make_string_concat_va(size_t count, ...);

C_MAKE_DEF char *c_make_c_string_concat_va(size_t count, ...);
C_MAKE_DEF char *c_make_c_string_path_concat_va(size_t count, ...);

C_MAKE_DEF CMakeProcessId c_make_command_run(CMakeCommand command);
C_MAKE_DEF bool c_make_process_wait(CMakeProcessId process_id);
C_MAKE_DEF bool c_make_command_run_and_wait(CMakeCommand command);

static inline bool
c_make_config_is_enabled(const char *key, bool fallback)
{
    bool result = fallback;
    CMakeConfigValue config_value = c_make_config_get(key);

    if (config_value.is_valid)
    {
        const char *val = config_value.val;

        if (val && (val[0] == 'o') && (val[1] == 'n') && (val[2] == 0))
        {
            result = true;
        }
        else
        {
            result = false;
        }
    }

    return result;
}

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#if C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS

#  include <time.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <sys/stat.h>

#endif

#endif // __C_MAKE_INCLUDE__

#if defined(C_MAKE_IMPLEMENTATION)

static CMakeContext _c_make_context;

#if !defined(c_make_strlen)
#  include <string.h>
#  define c_make_strcmp(a, b) strcmp(a, b)
#  define c_make_strdup(a) strdup(a)
#endif

#if !defined(c_make_malloc)
#  include <stdlib.h>
#  define c_make_malloc(a) malloc(a)
#endif

#if C_MAKE_PLATFORM_WINDOWS

static inline LPWSTR
c_make_utf8_to_utf16(CMakeMemory *memory, const char *utf8_str)
{
    size_t utf8_length = c_make_get_c_string_length(utf8_str);
    size_t utf16_size = 2 * (utf8_length + 1);
    LPWSTR utf16_str = (LPWSTR) c_make_memory_allocate(memory, utf16_size);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, utf16_str, utf16_size);
    return utf16_str;
}

#endif

C_MAKE_DEF void
c_make_log(CMakeLogLevel log_level, const char *format, ...)
{
    if (!_c_make_context.shell_initialized)
    {
#if C_MAKE_PLATFORM_WINDOWS
        HANDLE std_error = GetStdHandle(STD_ERROR_HANDLE);
        DWORD mode = 0;

        if (GetConsoleMode(std_error, &mode) && SetConsoleMode(std_error, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING ))
#else
        int fileno(FILE *);

        if (isatty(fileno(stderr)))
#endif
        {
            _c_make_context.reset                = "\x1b[0m";
            _c_make_context.color_black          = "\x1b[30m";
            _c_make_context.color_red            = "\x1b[31m";
            _c_make_context.color_green          = "\x1b[32m";
            _c_make_context.color_yellow         = "\x1b[33m";
            _c_make_context.color_blue           = "\x1b[34m";
            _c_make_context.color_magenta        = "\x1b[35m";
            _c_make_context.color_cyan           = "\x1b[36m";
            _c_make_context.color_white          = "\x1b[37m";
            _c_make_context.color_bright_black   = "\x1b[1;30m";
            _c_make_context.color_bright_red     = "\x1b[1;31m";
            _c_make_context.color_bright_green   = "\x1b[1;32m";
            _c_make_context.color_bright_yellow  = "\x1b[1;33m";
            _c_make_context.color_bright_blue    = "\x1b[1;34m";
            _c_make_context.color_bright_magenta = "\x1b[1;35m";
            _c_make_context.color_bright_cyan    = "\x1b[1;36m";
            _c_make_context.color_bright_white   = "\x1b[1;37m";
        }
        else
        {
            _c_make_context.reset                = "";
            _c_make_context.color_black          = "";
            _c_make_context.color_red            = "";
            _c_make_context.color_green          = "";
            _c_make_context.color_yellow         = "";
            _c_make_context.color_blue           = "";
            _c_make_context.color_magenta        = "";
            _c_make_context.color_cyan           = "";
            _c_make_context.color_white          = "";
            _c_make_context.color_bright_black   = "";
            _c_make_context.color_bright_red     = "";
            _c_make_context.color_bright_green   = "";
            _c_make_context.color_bright_yellow  = "";
            _c_make_context.color_bright_blue    = "";
            _c_make_context.color_bright_magenta = "";
            _c_make_context.color_bright_cyan    = "";
            _c_make_context.color_bright_white   = "";
        }

        _c_make_context.shell_initialized = true;
    }

    switch (log_level)
    {
        case CMakeLogLevelRaw:
        {
        } break;

        case CMakeLogLevelInfo:
        {
            fprintf(stderr, "%s-- ", _c_make_context.color_bright_white);
        } break;

        case CMakeLogLevelWarning:
        {
            fprintf(stderr, "%s-- %swarning: ",
                    _c_make_context.color_bright_white,
                    _c_make_context.color_bright_yellow);
        } break;

        case CMakeLogLevelError:
        {
            fprintf(stderr, "%s-- %serror: ",
                    _c_make_context.color_bright_white,
                    _c_make_context.color_bright_red);
        } break;
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "%s", _c_make_context.reset);
    fflush(stderr);
    va_end(args);
}

C_MAKE_DEF void *
c_make_memory_allocate(CMakeMemory *memory, size_t size)
{
    size = (size + 15) & ~15;

    if (memory->allocated == 0)
    {
        memory->used = 0;
        memory->allocated = 16 * 1024 * 1024;
        memory->base = c_make_malloc(memory->allocated);
    }

    void *result = 0;

    if ((memory->used + size) <= memory->allocated)
    {
        result = (unsigned char *) memory->base + memory->used;
        memory->used += size;
    }

    return result;
}

C_MAKE_DEF void *
c_make_memory_reallocate(CMakeMemory *memory, void *old_ptr, size_t old_size, size_t new_size)
{
    assert(new_size > old_size);

    old_size = (old_size + 15) & ~15;
    new_size = (new_size + 15) & ~15;

    void *end_ptr  = (unsigned char *) old_ptr + old_size;
    void *next_ptr = (unsigned char *) memory->base + memory->used;

    void *result = 0;

    if (!old_ptr || (end_ptr != next_ptr))
    {
        result = c_make_memory_allocate(memory, new_size);

        if (old_ptr && result)
        {
            // TODO: take advantage of the fact that old_size is a multiple of 16
            unsigned char *src = (unsigned char *) old_ptr;
            unsigned char *dst = (unsigned char *) result;

            while (old_size--)
            {
                *dst++ = *src++;
            }
        }
    }
    else
    {
        size_t size = new_size - old_size;

        if ((memory->used + size) <= memory->allocated)
        {
            result = old_ptr;
            memory->used += size;
        }
    }

    return result;
}

C_MAKE_DEF size_t
c_make_memory_get_used(CMakeMemory *memory)
{
    return memory->used;
}

C_MAKE_DEF void
c_make_memory_set_used(CMakeMemory *memory, size_t used)
{
    assert(used <= memory->used);
    assert(!(used & 15));

    memory->used = used;
}

C_MAKE_DEF void *
c_make_allocate(size_t size)
{
    return c_make_memory_allocate(&_c_make_context.public_memory, size);
}

C_MAKE_DEF size_t
c_make_memory_save(void)
{
    return c_make_memory_get_used(&_c_make_context.public_memory);
}

C_MAKE_DEF void
c_make_memory_restore(size_t saved)
{
    c_make_memory_set_used(&_c_make_context.public_memory, saved);
}

C_MAKE_DEF void
c_make_command_append_va(CMakeCommand *command, size_t count, ...)
{
    if ((command->count + count) > command->allocated)
    {
        size_t grow = 16;

        if (count > grow)
        {
            grow = count;
        }

        size_t old_count = command->allocated;
        command->allocated += grow;
        command->items = (const char **) c_make_memory_reallocate(&_c_make_context.public_memory,
                                                                  (void *) command->items,
                                                                  old_count * sizeof(const char *),
                                                                  command->allocated * sizeof(const char *));
    }

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        command->items[command->count] = va_arg(args, const char *);
        command->count += 1;
    }

    va_end(args);
}

C_MAKE_DEF void
c_make_command_append_slice(CMakeCommand *command, size_t count, const char **items)
{
    if ((command->count + count) > command->allocated)
    {
        size_t grow = 16;

        if (count > grow)
        {
            grow = count;
        }

        size_t old_count = command->allocated;
        command->allocated += grow;
        command->items = (const char **) c_make_memory_reallocate(&_c_make_context.public_memory,
                                                                  (void *) command->items,
                                                                  old_count * sizeof(const char *),
                                                                  command->allocated * sizeof(const char *));
    }

    for (size_t i = 0; i < count; i += 1)
    {
        command->items[command->count] = items[i];
        command->count += 1;
    }
}

C_MAKE_DEF void
c_make_command_append_command_line(CMakeCommand *command, const char *str)
{
    if (str)
    {
        while (*str)
        {
            while (*str == ' ') str += 1;

            const char *start = str;
            while (*str && (*str != ' ')) str += 1;
            size_t length = str - start;

            if (length)
            {
                char *c_str = (char *) c_make_memory_allocate(&_c_make_context.public_memory, length + 1);
                char *dst = c_str;
                const char *src = start;

                for (size_t i = 0; i < length; i += 1)
                {
                    *dst++ = *src++;
                }

                *dst = 0;

                c_make_command_append_slice(command, 1, (const char **) &c_str);
            }
        }
    }
}

C_MAKE_DEF CMakeString
c_make_command_to_string(CMakeCommand command)
{
    CMakeString result = { 0 };

    for (size_t i = 0; i < command.count; i += 1)
    {
        const char *str = command.items[i];
        result.count += c_make_get_c_string_length(str) + 1;
    }

    if (result.count > 0)
    {
        result.data = (char *) c_make_memory_allocate(&_c_make_context.public_memory, result.count);
        char *dst = result.data;

        for (size_t i = 0; i < command.count; i += 1)
        {
            const char *str = command.items[i];
            while (*str) *dst++ = *str++;

            *dst++ = ' ';
        }

        result.count -= 1;
    }

    return result;
}

C_MAKE_DEF bool
c_make_strings_are_equal(CMakeString a, CMakeString b)
{
    if (a.count != b.count)
    {
        return false;
    }

    for (size_t i = 0; i < a.count; i += 1)
    {
        if (a.data[i] != b.data[i])
        {
            return false;
        }
    }

    return true;
}

C_MAKE_DEF CMakeString
c_make_copy_string(CMakeMemory *memory, CMakeString str)
{
    CMakeString result;
    result.count = str.count;
    result.data = (char *) c_make_memory_allocate(memory, str.count + 1);

    for (size_t i = 0; i < str.count; i += 1)
    {
        result.data[i] = str.data[i];
    }

    result.data[result.count] = 0;

    return result;
}

C_MAKE_DEF CMakeString
c_make_string_split_left(CMakeString *str, char c)
{
    CMakeString result = *str;

    while (str->count)
    {
        if (str->data[0] == c)
        {
            break;
        }

        str->count -= 1;
        str->data += 1;
    }

    result.count = str->data - result.data;

    if (str->count)
    {
        str->count -= 1;
        str->data += 1;
    }

    return result;
}

C_MAKE_DEF CMakeString
c_make_string_trim(CMakeString str)
{
    while (str.count && (str.data[str.count - 1] == ' '))
    {
        str.count -= 1;
    }

    while (str.count && (str.data[0] == ' '))
    {
        str.count -= 1;
        str.data += 1;
    }

    return str;
}

C_MAKE_DEF size_t
c_make_string_find(CMakeString str, CMakeString pattern)
{
    size_t index = 0;

    for (; index < str.count; index += 1)
    {
        CMakeString slice;
        slice.count = pattern.count;
        slice.data  = str.data + index;

        size_t max_count = str.count - index;

        if (slice.count > max_count)
        {
            slice.count = max_count;
        }

        if (c_make_strings_are_equal(slice, pattern))
        {
            return index;
        }
    }

    return index;
}

C_MAKE_DEF char *
c_make_string_to_c_string(CMakeMemory *memory, CMakeString str)
{
    char *result = (char *) c_make_memory_allocate(memory, str.count + 1);

    for (size_t i = 0; i < str.count; i += 1)
    {
        result[i] = str.data[i];
    }

    result[str.count] = 0;

    return result;
}

C_MAKE_DEF CMakePlatform
c_make_get_target_platform(void)
{
    return _c_make_context.target_platform;
}

C_MAKE_DEF CMakeArchitecture
c_make_get_target_architecture(void)
{
    return _c_make_context.target_architecture;
}

C_MAKE_DEF CMakeBuildType
c_make_get_build_type(void)
{
    return _c_make_context.build_type;
}

C_MAKE_DEF const char *
c_make_get_build_path(void)
{
    return _c_make_context.build_path;
}

C_MAKE_DEF const char *
c_make_get_source_path(void)
{
    return _c_make_context.source_path;
}

C_MAKE_DEF const char *
c_make_get_install_prefix(void)
{
    const char *result = 0;

    CMakeConfigValue value = c_make_config_get("install_prefix");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
        // TODO: something else for windows
        result = "/usr/local";
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_host_c_compiler(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("host_c_compiler");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
#if C_MAKE_PLATFORM_WINDOWS
#  ifdef __MINGW32__
        result = "x86_64-w64-mingw32-gcc";
#  else
        result = "cl";
#  endif
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX
        result = "cc";
#elif C_MAKE_PLATFORM_MACOS
        result = "clang";
#elif C_MAKE_PLATFORM_WEB
        result = "clang";
#endif
    }

    if (!c_make_has_slash_or_backslash(result))
    {
        result = c_make_find_program(result);
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_c_compiler(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_c_compiler");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
        result = c_make_get_host_c_compiler();
    }

    if (!c_make_has_slash_or_backslash(result))
    {
        result = c_make_find_program(result);
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_c_flags(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_c_flags");

    if (value.is_valid)
    {
        result = value.val;
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_host_cpp_compiler(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("host_cpp_compiler");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
#if C_MAKE_PLATFORM_WINDOWS
#  ifdef __MINGW32__
        result = "x86_64-w64-mingw32-g++";
#  else
        result = "cl";
#  endif
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX
        result = "c++";
#elif C_MAKE_PLATFORM_MACOS
        result = "clang++";
#elif C_MAKE_PLATFORM_WEB
        result = "clang++";
#endif
    }

    if (!c_make_has_slash_or_backslash(result))
    {
        result = c_make_find_program(result);
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_cpp_compiler(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_cpp_compiler");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
        result = c_make_get_host_cpp_compiler();
    }

    if (!c_make_has_slash_or_backslash(result))
    {
        result = c_make_find_program(result);
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_cpp_flags(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_cpp_flags");

    if (value.is_valid)
    {
        result = value.val;
    }

    return result;
}

C_MAKE_DEF void
c_make_config_set(const char *_key, const char *value)
{
    CMakeConfigEntry *entry = 0;
    CMakeString key = CMakeCString(_key);

    for (size_t i = 0; i < _c_make_context.config.count; i += 1)
    {
        CMakeConfigEntry *config_entry = _c_make_context.config.items + i;

        if (c_make_strings_are_equal(config_entry->key, key))
        {
            entry = config_entry;
            break;
        }
    }

    if (!entry)
    {
        if (_c_make_context.config.count == _c_make_context.config.allocated)
        {
            size_t old_count = _c_make_context.config.allocated;
            _c_make_context.config.allocated += 16;
            _c_make_context.config.items =
                (CMakeConfigEntry *) c_make_memory_reallocate(&_c_make_context.private_memory,
                                                              _c_make_context.config.items,
                                                              old_count * sizeof(*_c_make_context.config.items),
                                                              _c_make_context.config.allocated * sizeof(*_c_make_context.config.items));
        }

        entry = _c_make_context.config.items + _c_make_context.config.count;
        _c_make_context.config.count += 1;

        entry->key = c_make_copy_string(&_c_make_context.private_memory, key);
    }

    entry->value = c_make_copy_string(&_c_make_context.private_memory, CMakeCString(value));

    if (c_make_strings_are_equal(entry->key, CMakeStringLiteral("target_platform")))
    {
        if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("android")))
        {
            _c_make_context.target_platform = CMakePlatformAndroid;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("windows")))
        {
            _c_make_context.target_platform = CMakePlatformWindows;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("linux")))
        {
            _c_make_context.target_platform = CMakePlatformLinux;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("macos")))
        {
            _c_make_context.target_platform = CMakePlatformMacOs;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("web")))
        {
            _c_make_context.target_platform = CMakePlatformWeb;
        }
    }
    else if (c_make_strings_are_equal(entry->key, CMakeStringLiteral("target_architecture")))
    {
        if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("amd64")))
        {
            _c_make_context.target_architecture = CMakeArchitectureAmd64;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("aarch64")))
        {
            _c_make_context.target_architecture = CMakeArchitectureAarch64;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("wasm32")))
        {
            _c_make_context.target_architecture = CMakeArchitectureWasm32;
        }
    }
    else if (c_make_strings_are_equal(entry->key, CMakeStringLiteral("build_type")))
    {
        if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("debug")))
        {
            _c_make_context.build_type = CMakeBuildTypeDebug;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("reldebug")))
        {
            _c_make_context.build_type = CMakeBuildTypeRelDebug;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("release")))
        {
            _c_make_context.build_type = CMakeBuildTypeRelease;
        }
    }
}

C_MAKE_DEF CMakeConfigValue
c_make_config_get(const char *_key)
{
    CMakeConfigValue result = { 0 };
    CMakeString key = CMakeCString(_key);

    for (size_t i = 0; i < _c_make_context.config.count; i += 1)
    {
        CMakeConfigEntry *config_entry = _c_make_context.config.items + i;

        if (c_make_strings_are_equal(config_entry->key, key))
        {
            result.is_valid = true;
            result.val = config_entry->value.data;
            break;
        }
    }

    return result;
}

C_MAKE_DEF void
c_make_print_config(void)
{
    for (size_t i = 0; i < _c_make_context.config.count; i += 1)
    {
        CMakeConfigEntry *entry = _c_make_context.config.items + i;
        c_make_log(CMakeLogLevelRaw, "  + %" CMakeStringFmt " = \"%" CMakeStringFmt "\"\n",
                   CMakeStringArg(entry->key), CMakeStringArg(entry->value));
    }
}

C_MAKE_DEF bool
c_make_store_config(const char *file_name)
{
    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

    CMakeString config_string = CMakeStringLiteral("");

    for (size_t i = 0; i < _c_make_context.config.count; i += 1)
    {
        CMakeConfigEntry *entry = _c_make_context.config.items + i;

        config_string = c_make_string_concat(config_string, entry->key,
                                             CMakeStringLiteral(" = \""),
                                             entry->value, CMakeStringLiteral("\"\n"));
    }

    if (!c_make_write_entire_file(file_name, config_string))
    {
        c_make_log(CMakeLogLevelError, "could not write config file '%s'\n", file_name);
        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        return false;
    }

    c_make_memory_set_used(&_c_make_context.public_memory, public_used);

    return true;
}

C_MAKE_DEF bool
c_make_load_config(const char *file_name)
{
    CMakeString config_string = { 0 };

    if (!c_make_read_entire_file(file_name, &config_string))
    {
        c_make_log(CMakeLogLevelError, "could not read config file '%s'\n", file_name);
        return false;
    }

    while (config_string.count)
    {
        CMakeString line = c_make_string_split_left(&config_string, '\n');
        CMakeString key = c_make_string_trim(c_make_string_split_left(&line, '='));
        CMakeString value = c_make_string_trim(line);

        if (key.count && value.count)
        {
            if (value.data[0] == '"')
            {
                value.count -= 1;
                value.data += 1;

                if (value.count && (value.data[value.count - 1] == '"'))
                {
                    value.count -= 1;
                }

                size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                c_make_config_set(c_make_string_to_c_string(&_c_make_context.public_memory, key),
                                  c_make_string_to_c_string(&_c_make_context.public_memory, value));

                c_make_memory_set_used(&_c_make_context.public_memory, public_used);
            }
            else
            {
                // not a string
            }
        }
    }

    return true;
}

C_MAKE_DEF bool
c_make_needs_rebuild(const char *output_file, size_t input_file_count, const char **input_files)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, output_file);
    HANDLE file = CreateFile(utf16_file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    if (file == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return true;
        }

        return false;
    }

    FILETIME output_file_last_write_time;

    if (!GetFileTime(file, 0, 0, &output_file_last_write_time))
    {
        CloseHandle(file);
        return true;
    }

    CloseHandle(file);

    for (size_t i = 0; i < input_file_count; i += 1)
    {
        size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

        utf16_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, input_files[i]);
        file = CreateFile(utf16_file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

        c_make_memory_set_used(&_c_make_context.private_memory, private_used);

        if (file == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        FILETIME input_file_last_write_time;

        if (!GetFileTime(file, 0, 0, &input_file_last_write_time))
        {
            CloseHandle(file);
            return true;
        }

        CloseHandle(file);

        if (CompareFileTime(&output_file_last_write_time, &input_file_last_write_time) < 0)
        {
            return true;
        }
    }

    return false;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    struct stat stats;

    if (stat(output_file, &stats))
    {
        if (errno == ENOENT)
        {
            return true;
        }

        return false;
    }

    time_t output_file_last_write_time = stats.st_mtime;

    for (size_t i = 0; i < input_file_count; i += 1)
    {
        const char *input_file = input_files[i];

        if (!stat(input_file, &stats))
        {
            time_t input_file_last_write_time = stats.st_mtime;

            if (input_file_last_write_time > output_file_last_write_time)
            {
                return true;
            }
        }
    }

    return false;
#endif
}

C_MAKE_DEF bool
c_make_needs_rebuild_single_source(const char *output_file, const char *input_file)
{
    return c_make_needs_rebuild(output_file, 1, &input_file);
}

C_MAKE_DEF bool
c_make_file_exists(const char *file_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, file_name);
    DWORD file_attributes = GetFileAttributes(utf16_file_name);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return (file_attributes != INVALID_FILE_ATTRIBUTES);
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    bool result = false;
    struct stat stats;

    if (!stat(file_name, &stats))
    {
        result = S_ISREG(stats.st_mode) ? true : false;
    }

    return result;
#endif
}

C_MAKE_DEF bool
c_make_directory_exists(const char *directory_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_directory_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, directory_name);
    DWORD file_attributes = GetFileAttributes(utf16_directory_name);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return (file_attributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    bool result = false;
    struct stat stats;

    if (!stat(directory_name, &stats))
    {
        result = S_ISDIR(stats.st_mode) ? true : false;
    }

    return result;
#endif
}

C_MAKE_DEF bool
c_make_create_directory(const char *directory_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_directory_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, directory_name);

    if (!CreateDirectory(utf16_directory_name, 0))
    {
        c_make_memory_set_used(&_c_make_context.private_memory, private_used);

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            return true;
        }

        return false;
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    if (!mkdir(directory_name, 0775))
    {
        return true;
    }

    if (errno == EEXIST)
    {
        return true;
    }

    return false;
#endif
}

C_MAKE_DEF bool
c_make_read_entire_file(const char *file_name, CMakeString *content)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, file_name);
    HANDLE file = CreateFile(utf16_file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    LARGE_INTEGER file_size;

    if (!GetFileSizeEx(file, &file_size))
    {
        CloseHandle(file);
        return false;
    }

    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

    content->count = file_size.QuadPart;
    content->data = (char *) c_make_memory_allocate(&_c_make_context.public_memory, content->count + 1);

    size_t index = 0;

    while (index < content->count)
    {
        DWORD bytes_read = 0;
        if (!ReadFile(file, content->data + index, content->count - index, &bytes_read, 0))
        {
            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
            CloseHandle(file);
            return false;
        }

        index += bytes_read;
    }

    content->data[content->count] = 0;

    CloseHandle(file);
    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    int fd = open(file_name, O_RDONLY);

    if (fd >= 0)
    {
        struct stat stats;

        if (fstat(fd, &stats) < 0)
        {
            close(fd);
            return false;
        }

        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        content->count = stats.st_size;
        content->data = (char *) c_make_memory_allocate(&_c_make_context.public_memory, content->count + 1);

        size_t index = 0;

        while (index < content->count)
        {
            ssize_t read_bytes = read(fd, content->data + index, content->count - index);

            if (read_bytes < 0)
            {
                c_make_memory_set_used(&_c_make_context.public_memory, public_used);
                close(fd);
                return false;
            }

            index += read_bytes;
        }

        content->data[content->count] = 0;

        close(fd);
        return true;
    }
    else
    {
        return false;
    }
#endif
}

C_MAKE_DEF bool
c_make_write_entire_file(const char *file_name, CMakeString content)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, file_name);
    HANDLE file = CreateFile(utf16_file_name, GENERIC_WRITE, 0, 0,
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    while (content.count)
    {
        DWORD bytes_written = 0;
        if (!WriteFile(file, content.data, content.count, &bytes_written, 0))
        {
            CloseHandle(file);
            return false;
        }

        content.data += bytes_written;
        content.count -= bytes_written;
    }

    CloseHandle(file);
    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    int fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0664);

    if (fd >= 0)
    {
        while (content.count)
        {
            ssize_t written_bytes = write(fd, content.data, content.count);

            if (written_bytes < 0)
            {
                close(fd);
                return false;
            }

            content.data += written_bytes;
            content.count -= written_bytes;
        }

        close(fd);
        return true;
    }
    else
    {
        return false;
    }
#endif
}

C_MAKE_DEF bool
c_make_copy_file(const char *src_file_name, const char *dst_file_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_src_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, src_file_name);
    LPWSTR utf16_dst_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, dst_file_name);

    if (!CopyFile(utf16_src_file_name, utf16_dst_file_name, FALSE))
    {
        c_make_memory_set_used(&_c_make_context.private_memory, private_used);
        return false;
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    int src_fd = open(src_file_name, O_RDONLY);

    if (src_fd < 0)
    {
        c_make_log(CMakeLogLevelError, "could not open file '%s': %s\n", src_file_name, strerror(errno));
        return false;
    }

    struct stat stats;

    if (fstat(src_fd, &stats) < 0)
    {
        c_make_log(CMakeLogLevelError, "could not get file stats on '%s': %s\n", src_file_name, strerror(errno));
        close(src_fd);
        return false;
    }

    size_t src_file_size = stats.st_size;

    int dst_fd = open(dst_file_name, O_WRONLY | O_TRUNC | O_CREAT, stats.st_mode);

    if (dst_fd < 0)
    {
        c_make_log(CMakeLogLevelError, "could not create file '%s': %s\n", dst_file_name, strerror(errno));
        close(src_fd);
        return false;
    }

    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    void *copy_buffer = c_make_memory_allocate(&_c_make_context.private_memory, 4096);

    size_t index = 0;

    while (index < src_file_size)
    {
        size_t remaining_size = src_file_size - index;
        size_t size_to_read = 4096;

        if (remaining_size < size_to_read)
        {
            size_to_read = remaining_size;
        }

        ssize_t read_bytes = read(src_fd, copy_buffer, size_to_read);

        if ((read_bytes < 0) || ((size_t) read_bytes != size_to_read))
        {
            c_make_memory_set_used(&_c_make_context.private_memory, private_used);
            close(src_fd);
            close(dst_fd);
            return false;
        }

        write(dst_fd, copy_buffer, size_to_read);
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    close(dst_fd);
    close(src_fd);
    return true;
#endif
}

C_MAKE_DEF bool
c_make_rename_file(const char *old_file_name, const char *new_file_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_old_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, old_file_name);
    LPWSTR utf16_new_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, new_file_name);

    bool result =
        MoveFileEx(utf16_old_file_name, utf16_new_file_name, MOVEFILE_REPLACE_EXISTING) ? true : false;

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return result;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    if (rename(old_file_name, new_file_name))
    {
        return false;
    }

    return true;
#endif
}

C_MAKE_DEF bool
c_make_delete_file(const char *file_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_utf8_to_utf16(&_c_make_context.private_memory, file_name);

    bool result = DeleteFile(utf16_file_name) ? true : false;

    if (!result && (GetLastError() == ERROR_FILE_NOT_FOUND))
    {
        result = true;
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return result;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    if (unlink(file_name))
    {
        if (errno == ENOENT)
        {
            return true;
        }

        return false;
    }

    return true;
#endif
}

C_MAKE_DEF bool
c_make_has_slash_or_backslash(const char *path)
{
    if (path)
    {
        while (*path)
        {
            if ((*path == '/') || (*path == '\\'))
            {
                return true;
            }

            path += 1;
        }

        return false;
    }
    else
    {
        return false;
    }
}

C_MAKE_DEF const char *
c_make_find_program(const char *program_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    // TODO: search path
    return program_name;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    char *PATH = getenv("PATH");

    if (PATH)
    {
        CMakeString paths = CMakeCString(PATH);

        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);
        size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

        while (paths.count)
        {
            char *path = c_make_string_to_c_string(&_c_make_context.private_memory, c_make_string_split_left(&paths, ':'));
            char *full_path = c_make_c_string_path_concat(path, program_name);

            c_make_memory_set_used(&_c_make_context.private_memory, private_used);

            // TODO: is executable?
            if (c_make_file_exists(full_path))
            {
                return full_path;
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }
    }

    return 0;
#endif
}

C_MAKE_DEF CMakeString
c_make_string_concat_va(size_t count, ...)
{
    CMakeString result = { 0 };

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        CMakeString str = va_arg(args, CMakeString);
        result.count += str.count;
    }

    va_end(args);

    result.data = (char *) c_make_memory_allocate(&_c_make_context.public_memory, result.count);
    char *dst = result.data;

    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        CMakeString str = va_arg(args, CMakeString);

        for (size_t j = 0; j < str.count; j += 1)
        {
            *dst++ = str.data[j];
        }
    }

    va_end(args);

    return result;
}

C_MAKE_DEF char *
c_make_c_string_concat_va(size_t count, ...)
{
    size_t length = 1;

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        const char *str = va_arg(args, const char *);
        length += c_make_get_c_string_length(str);
    }

    va_end(args);

    char *result = (char *) c_make_memory_allocate(&_c_make_context.public_memory, length);
    char *dst = result;

    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        const char *str = va_arg(args, const char *);
        while (*str) *dst++ = *str++;
    }

    va_end(args);

    *dst = 0;

    return result;
}

C_MAKE_DEF char *
c_make_c_string_path_concat_va(size_t count, ...)
{
    size_t length = 0;

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        const char *str = va_arg(args, const char *);
        length += c_make_get_c_string_length(str) + 1;
    }

    va_end(args);

    char *result = (char *) c_make_memory_allocate(&_c_make_context.public_memory, length);
    char *dst = result;

    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        const char *str = va_arg(args, const char *);
        while (*str) *dst++ = *str++;

#if C_MAKE_PLATFORM_WINDOWS
        *dst++ = '\\';
#else
        *dst++ = '/';
#endif
    }

    va_end(args);

    *--dst = 0;

    return result;
}

C_MAKE_DEF CMakeProcessId
c_make_command_run(CMakeCommand command)
{
    if (command.count == 0)
    {
        return CMakeInvalidProcessId;
    }

    if (_c_make_context.verbose)
    {
        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        CMakeString command_string = c_make_command_to_string(command);
        c_make_log(CMakeLogLevelRaw, "%" CMakeStringFmt "\n", CMakeStringArg(command_string));

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
    }

#if C_MAKE_PLATFORM_WINDOWS
    STARTUPINFO start_info = { sizeof(STARTUPINFO) };
    start_info.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    start_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    start_info.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    start_info.dwFlags    = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION process_info = { 0 };

    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    CMakeString command_line = c_make_command_to_string(command);

    int wide_command_line_size = 2 * command_line.count;
    LPWSTR wide_command_line = (LPWSTR) c_make_memory_allocate(&_c_make_context.private_memory, wide_command_line_size);

    int wide_count = MultiByteToWideChar(CP_UTF8, 0, command_line.data, command_line.count, wide_command_line, wide_command_line_size);
    wide_command_line[wide_count] = 0;

    BOOL result = CreateProcess(0, wide_command_line, 0, 0, TRUE, 0, 0, 0, &start_info, &process_info);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);
    c_make_memory_set_used(&_c_make_context.public_memory, public_used);

    if (!result)
    {
        // TODO: log error
        fprintf(stderr, "Could not run: %lu\n", GetLastError());
        return CMakeInvalidProcessId;
    }

    CloseHandle(process_info.hThread);

    return process_info.hProcess;
#else
    char **command_line = (char **) c_make_memory_allocate(&_c_make_context.private_memory, (command.count + 1) * sizeof(char *));

    for (size_t i = 0; i < command.count; i += 1)
    {
        command_line[i] = (char *) command.items[i];
    }

    command_line[command.count] = 0;

    pid_t pid = fork();

    if (pid < 0)
    {
        // TODO: log error
        fprintf(stderr, "Could not fork\n");
        return CMakeInvalidProcessId;
    }

    if (pid == 0)
    {
        int ret = execvp(command_line[0], command_line);

        if (ret < 0)
        {
            // TODO: log error
            fprintf(stderr, "Could not execvp: %s\n", strerror(errno));
            exit(1);
        }

        /* unreachable */
        return CMakeInvalidProcessId;
    }

    return pid;
#endif
}

C_MAKE_DEF bool
c_make_process_wait(CMakeProcessId process_id)
{
    if (process_id == CMakeInvalidProcessId)
    {
        return false;
    }

    bool result = true;

#if C_MAKE_PLATFORM_WINDOWS
    DWORD wait_result = WaitForSingleObject(process_id, INFINITE);

    if (wait_result == WAIT_FAILED)
    {
        // TODO: log error
        return false;
    }

    DWORD exit_code = 0;

    if (!GetExitCodeProcess(process_id, &exit_code))
    {
        // TODO: log error
        result = false;
    }

    if (exit_code != 0)
    {
        // TODO: log that the process has exited with an error code
        // TODO: What to return here? false or true?
        result = false;
    }

    CloseHandle(process_id);
#else
    for (;;)
    {
        int status;

        if (waitpid(process_id, &status, 0) < 0)
        {
            // TODO: log error
            result = false;
            break;
        }

        if (WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);

            if (exit_code != 0)
            {
                // TODO: log that the process has exited with an error code
                // TODO: What to return here? false or true?
                result = false;
            }

            break;
        }

        if (WIFSIGNALED(status))
        {
            // TODO: log the signal that terminated the child
            result = false;
            break;
        }
    }
#endif

    return result;
}

C_MAKE_DEF bool
c_make_command_run_and_wait(CMakeCommand command)
{
    CMakeProcessId process_id = c_make_command_run(command);
    return c_make_process_wait(process_id);
}

#if !defined(C_MAKE_NO_ENTRY_POINT)

static void
print_help(const char *program_name)
{
    fprintf(stderr, "usage: %s <command> <build-directory> [--verbose]\n", program_name);
    fprintf(stderr, "\n");
}

int main(int argument_count, char **arguments)
{
    if (argument_count < 3)
    {
        print_help(arguments[0]);
        return -1;
    }

    CMakeString command = CMakeCString(arguments[1]);
    const char *build_directory = arguments[2];
    const char *config_file_name = c_make_c_string_path_concat(build_directory, "c_make.txt");

    _c_make_context.build_path = build_directory;
    _c_make_context.source_path = ".";

    for (int i = 3; i < argument_count; i += 1)
    {
        CMakeString argument = CMakeCString(arguments[i]);

        if (c_make_strings_are_equal(argument, CMakeStringLiteral("--verbose")))
        {
            _c_make_context.verbose = true;
        }
    }

    // TODO: use executable path
#if C_MAKE_PLATFORM_WINDOWS
    const char *c_make_executable_file = "c_make.exe";
#else
    const char *c_make_executable_file = "c_make";
#endif
#ifdef __cplusplus
    const char *c_make_source_file = "c_make.cpp";
#else
    const char *c_make_source_file = "c_make.c";
#endif

    const char *c_make_source_files[] = { c_make_source_file, __FILE__ };

    if (c_make_needs_rebuild(c_make_executable_file, CMakeArrayCount(c_make_source_files), c_make_source_files))
    {
        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        char *c_make_temp_file = c_make_c_string_concat(c_make_executable_file, ".orig");
        c_make_rename_file(c_make_executable_file, c_make_temp_file);

#ifdef __cplusplus
        const char *compiler = c_make_get_host_cpp_compiler();
#else
        const char *compiler = c_make_get_host_c_compiler();
#endif

        CMakeCommand command = { 0 };

        c_make_command_append(&command, compiler);
#ifdef C_MAKE_INCLUDE_PATH
        c_make_command_append(&command, "-I" CMakeStr(C_MAKE_INCLUDE_PATH));
        c_make_command_append(&command, "-DC_MAKE_INCLUDE_PATH=" CMakeStr(C_MAKE_INCLUDE_PATH));
#endif
#ifdef C_MAKE_COMPILER_FLAGS
        c_make_command_append_command_line(&command, CMakeStr(C_MAKE_COMPILER_FLAGS));
        c_make_command_append(&command, "-DC_MAKE_COMPILER_FLAGS=" CMakeStr(C_MAKE_COMPILER_FLAGS));
#endif

        char *exe_output_arg = 0;

        if (c_make_compiler_is_msvc(compiler))
        {
            exe_output_arg = c_make_c_string_concat("-Fe\"", c_make_executable_file, "\"");
            c_make_command_append(&command, "-nologo", exe_output_arg, c_make_source_file);
        }
        else
        {
            c_make_command_append(&command, "-o", c_make_executable_file, c_make_source_file);
        }

        c_make_log(CMakeLogLevelInfo, "rebuild c_make\n");

        if (!c_make_command_run_and_wait(command))
        {
            c_make_rename_file(c_make_temp_file, c_make_executable_file);
        }
        else
        {
            command.count = 0;
            c_make_command_append(&command, c_make_c_string_path_concat(".", c_make_executable_file));
            c_make_command_append_slice(&command, argument_count - 1, (const char **) (arguments + 1));
            c_make_command_run_and_wait(command);
        }

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);

        return 0;
    }
    else
    {
        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        c_make_delete_file(c_make_c_string_concat(c_make_executable_file, ".orig"));

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
    }

    _c_make_context.target_platform = c_make_get_host_platform();
    _c_make_context.target_architecture = c_make_get_host_architecture();
    _c_make_context.build_type = CMakeBuildTypeDebug;

    if (c_make_strings_are_equal(command, CMakeStringLiteral("setup")))
    {
        c_make_create_directory(build_directory);

        if (c_make_file_exists(config_file_name))
        {
            c_make_log(CMakeLogLevelError, "there is already a directory called '%s'\n", build_directory);
            return -1;
        }

        c_make_config_set("target_platform", c_make_get_platform_name(c_make_get_host_platform()));
        c_make_config_set("target_architecture", c_make_get_architecture_name(c_make_get_host_architecture()));
        c_make_config_set("build_type", "debug");

        char *CC = getenv("CC");

        if (CC)
        {
            size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

            CMakeString CC_str = CMakeCString(CC);
            const char *target_c_compiler = c_make_string_to_c_string(&_c_make_context.public_memory, c_make_string_split_left(&CC_str, ' '));

            if (!c_make_has_slash_or_backslash(target_c_compiler))
            {
                target_c_compiler = c_make_find_program(target_c_compiler);
            }

            if (target_c_compiler)
            {
                c_make_config_set("target_c_compiler", target_c_compiler);
            }

            CC_str = c_make_string_trim(CC_str);

            if (CC_str.count)
            {
                c_make_config_set("target_c_flags", c_make_string_to_c_string(&_c_make_context.public_memory, CC_str));
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }

        char *CXX = getenv("CXX");

        if (CXX)
        {
            size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

            CMakeString CXX_str = CMakeCString(CXX);
            const char *target_cpp_compiler = c_make_string_to_c_string(&_c_make_context.public_memory, c_make_string_split_left(&CXX_str, ' '));

            if (!c_make_has_slash_or_backslash(target_cpp_compiler))
            {
                target_cpp_compiler = c_make_find_program(target_cpp_compiler);
            }

            if (target_cpp_compiler)
            {
                c_make_config_set("target_cpp_compiler", target_cpp_compiler);
            }

            CXX_str = c_make_string_trim(CXX_str);

            if (CXX_str.count)
            {
                c_make_config_set("target_cpp_flags", c_make_string_to_c_string(&_c_make_context.public_memory, CXX_str));
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }

        for (int i = 3; i < argument_count; i += 1)
        {
            CMakeString argument = CMakeCString(arguments[i]);

            CMakeString key = c_make_string_trim(c_make_string_split_left(&argument, '='));
            CMakeString value = c_make_string_trim(argument);

            if (key.count && value.count)
            {
                if (value.data[0] == '"')
                {
                    value.count -= 1;
                    value.data += 1;
                }

                if (value.count && (value.data[value.count - 1] == '"'))
                {
                    value.count -= 1;
                }

                size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                c_make_config_set(c_make_string_to_c_string(&_c_make_context.public_memory, key),
                                  c_make_string_to_c_string(&_c_make_context.public_memory, value));

                c_make_memory_set_used(&_c_make_context.public_memory, public_used);
            }
        }

        _c_make_entry_(CMakeTargetSetup);

        if (!c_make_config_get("install_prefix").is_valid)
        {
            // TODO: set to something different for windows
            c_make_config_set("install_prefix", "/usr/local");
        }

        if (c_make_get_host_platform() != c_make_get_target_platform())
        {
            if (c_make_get_target_platform() == CMakePlatformWindows)
            {
                CMakeConfigValue target_c_compiler = c_make_config_get("target_c_compiler");

                if (!target_c_compiler.is_valid)
                {
                    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                    const char *c_compiler = c_make_find_program("x86_64-w64-mingw32-gcc");

                    if (c_compiler)
                    {
                        c_make_config_set("target_c_compiler", c_compiler);
                    }

                    c_make_memory_set_used(&_c_make_context.public_memory, public_used);
                }

                CMakeConfigValue target_cpp_compiler = c_make_config_get("target_cpp_compiler");

                if (!target_cpp_compiler.is_valid)
                {
                    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                    const char *cpp_compiler = c_make_find_program("x86_64-w64-mingw32-g++");

                    if (cpp_compiler)
                    {
                        c_make_config_set("target_cpp_compiler", cpp_compiler);
                    }

                    c_make_memory_set_used(&_c_make_context.public_memory, public_used);
                }
            }
        }

        c_make_log(CMakeLogLevelInfo, "store config:\n");
        c_make_print_config();

        if (!c_make_store_config(config_file_name))
        {
            return -1;
        }
    }
    else if (c_make_strings_are_equal(command, CMakeStringLiteral("build")) ||
             c_make_strings_are_equal(command, CMakeStringLiteral("install")))
    {
        if (!c_make_directory_exists(build_directory))
        {
            c_make_log(CMakeLogLevelError, "there is no build directory called '%s'\n", build_directory);
            return -1;
        }

        if (!c_make_file_exists(config_file_name))
        {
            c_make_log(CMakeLogLevelError, "the build directory '%s' was never setup\n", build_directory);
            return -1;
        }

        if (!c_make_load_config(config_file_name))
        {
            return -1;
        }

        if (_c_make_context.verbose)
        {
            c_make_log(CMakeLogLevelInfo, "load config:\n");
            c_make_print_config();
        }

        if (c_make_strings_are_equal(command, CMakeStringLiteral("build")))
        {
            _c_make_entry_(CMakeTargetBuild);
        }
        else
        {
            _c_make_entry_(CMakeTargetInstall);
        }
    }

    return 0;
}

#endif // !defined(C_MAKE_NO_ENTRY_POINT)

#endif // defined(C_MAKE_IMPLEMENTATION)
