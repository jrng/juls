#define TokenType __TokenType

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef TokenType

static inline void *
allocate(u64 size)
{
    return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

static inline void
deallocate(void *ptr, uint64_t size)
{
    (void) size;
    VirtualFree(ptr, 0, MEM_RELEASE);
}

static File *
open_file(Allocator *allocator, String filename, u32 mode)
{
    File *result = 0;

    DWORD flags = FILE_ATTRIBUTE_NORMAL;
    DWORD share_mode = 0;
    DWORD access_mode = 0;

    if (mode & FILE_MODE_READ)
    {
        share_mode = FILE_SHARE_READ;
        access_mode = GENERIC_READ;
    }

    if (mode & FILE_MODE_WRITE)
    {
        flags = FILE_FLAG_WRITE_THROUGH;
        access_mode |= GENERIC_WRITE;
    }

    int size = MultiByteToWideChar(CP_UTF8, 0, (const char *) filename.data, filename.count, 0, 0);

    if (size > 0)
    {
        wchar_t *filename_wide = alloc_array(allocator, wchar_t, size, 8, false);
        MultiByteToWideChar(CP_UTF8, 0, (const char *) filename.data, filename.count, filename_wide, size);

        HANDLE file = CreateFile(filename_wide, access_mode, share_mode, 0, OPEN_EXISTING, flags, 0);

        if (file != INVALID_HANDLE_VALUE)
        {
            result = (File *) file;
        }
    }

    return result;
}

static File *
create_file(Allocator *allocator, String filename, u32 permissions)
{
    File *result = 0;

    int size = MultiByteToWideChar(CP_UTF8, 0, (const char *) filename.data, filename.count, 0, 0);

    if (size > 0)
    {
        wchar_t *filename_wide = alloc_array(allocator, wchar_t, size, 8, false);
        MultiByteToWideChar(CP_UTF8, 0, (const char *) filename.data, filename.count, filename_wide, size);

        HANDLE file = CreateFile(filename_wide, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
                                 FILE_FLAG_WRITE_THROUGH, 0);

        if (file != INVALID_HANDLE_VALUE)
        {
            result = (File *) file;
        }
    }

    return result;
}

static u64
get_file_size(File *file)
{
    assert(file);

    s64 result = 0;
    BY_HANDLE_FILE_INFORMATION file_info = { 0 };

    if (GetFileInformationByHandle((HANDLE) file, &file_info))
    {
        result = ((s64) file_info.nFileSizeHigh << 32) | (s64) file_info.nFileSizeLow;
    }

    return result;
}

static void
read_file(File *file, void *buffer, u64 offset, u64 size)
{
    assert(file);

    DWORD bytes_read = 0;
    ReadFile((HANDLE) file, buffer, size, &bytes_read, 0);
}

static void
write_file(File *file, void *buffer, u64 offset, u64 size)
{
    assert(file);

    DWORD bytes_written = 0;
    WriteFile((HANDLE) file, buffer, size, &bytes_written, 0);
}

static void
close_file(File *file)
{
    assert(file);
    CloseHandle((HANDLE) file);
}
