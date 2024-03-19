typedef struct
{
    s64 count;
    u8 *data;
} String;

typedef struct
{
    s32 count;
    s32 allocated;
    String *items;
} StringArray;

static inline String
make_string(s64 count, void *data)
{
    String result;
    result.count = count;
    result.data = data;
    return result;
}

#define S(str) ((String) { sizeof(str) - 1, (u8 *) (str) })
#define C(str) make_string(get_c_string_length(str), (str))

static inline s64
get_c_string_length(char const *str)
{
    s64 count = 0;

    while (*str++)
    {
        count += 1;
    }

    return count;
}

static inline bool
strings_are_equal(String a, String b)
{
    if (a.count != b.count)
    {
        return false;
    }

    for (s64 i = 0; i < a.count; i += 1)
    {
        if (a.data[i] != b.data[i])
        {
            return false;
        }
    }

    return true;
}

static char *
to_c_string(Allocator *allocator, String str)
{
    char *result = alloc_array(allocator, char, str.count + 2, 8, false);

    for (s64 i = 0; i < str.count; i += 1)
    {
        result[i] = str.data[i];
    }

    result[str.count + 0] = 0;
    result[str.count + 1] = 0;

    return result;
}

static String
concat(Allocator *allocator, String a, String b)
{
    String result;

    result.count = a.count + b.count;
    result.data = alloc_array(allocator, u8, result.count, 8, false);

    s64 index = 0;

    for (s64 i = 0; i < a.count; i += 1, index += 1)
    {
        result.data[index] = a.data[i];
    }

    for (s64 i = 0; i < b.count; i += 1, index += 1)
    {
        result.data[index] = b.data[i];
    }

    return result;
}

static String
path_concat(Allocator *allocator, String a, String b)
{
    if (a.count && ((a.data[a.count - 1] == '/') || (a.data[a.count - 1] == '\\')))
    {
        a.count -= 1;
    }

    if (b.count && ((b.data[0] == '/') || (b.data[0] == '\\')))
    {
        b.count -= 1;
        b.data  += 1;
    }

    String result;

    result.count = a.count + b.count + 1;
    result.data = alloc_array(allocator, u8, result.count, 8, false);

    s64 index = 0;

    for (s64 i = 0; i < a.count; i += 1, index += 1)
    {
        result.data[index] = a.data[i];
    }

#if JULS_PLATFORM_WINDOWS
    result.data[index++] = '\\';
#else
    result.data[index++] = '/';
#endif

    for (s64 i = 0; i < b.count; i += 1, index += 1)
    {
        result.data[index] = b.data[i];
    }

    return result;
}

static String
get_base_path(String path)
{
    while ((path.count > 0) && (path.data[path.count - 1] != '/') &&
           (path.data[path.count - 1] != '\\'))
    {
        path.count -= 1;
    }

    if ((path.count > 0) &&
        ((path.data[path.count - 1] == '/') || (path.data[path.count - 1] == '\\')))
    {
        path.count -= 1;
    }

    return path;
}
