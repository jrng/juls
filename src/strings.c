typedef struct
{
    s64 count;
    u8 *data;
} String;

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
