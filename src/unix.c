#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

static inline void *
allocate(u64 size)
{
    void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (result == MAP_FAILED) ? 0 : result;
}

static inline void
deallocate(void *ptr, u64 size)
{
    munmap(ptr, size);
}

static File *
open_file(Allocator *allocator, String filename, u32 mode)
{
    File *result = 0;
    int access_mode = 0;

    if ((mode & FILE_MODE_READ) && (mode & FILE_MODE_WRITE))
    {
        access_mode = O_RDWR;
    }
    else if (mode & FILE_MODE_READ)
    {
        access_mode = O_RDONLY;
    }
    else if (mode & FILE_MODE_WRITE)
    {
        access_mode = O_WRONLY;
    }

    s32 fd = open(to_c_string(allocator, filename), access_mode);

    if (fd >= 0)
    {
        result = (File *) ((u64) fd + 1);
    }

    return result;
}

static File *
create_file(Allocator *allocator, String filename, u32 permissions)
{
    File *result = 0;
    mode_t mode = 0;

    if (permissions & FILE_PERMISSION_READABLE)
    {
        mode |= 0444;
    }

    if (permissions & FILE_PERMISSION_WRITEABLE)
    {
        mode |= 0220;
    }

    if (permissions & FILE_PERMISSION_EXECUTABLE)
    {
        mode |= 0111;
    }

    s32 fd = creat(to_c_string(allocator, filename), mode);

    if (fd >= 0)
    {
        result = (File *) ((u64) fd + 1);
    }

    return result;
}

static u64
get_file_size(File *file)
{
    assert(file);
    assert((u64) file <= 0x80000000);

    u64 size = 0;

    struct stat file_stat;

    if (!fstat(*(s32 *) &file - 1, &file_stat))
    {
        size = file_stat.st_size;
    }

    return size;
}

static void
read_file(File *file, void *buffer, u64 offset, u64 size)
{
    assert(file);
    assert((u64) file <= 0x80000000);

    lseek(*(s32 *) &file - 1, offset, SEEK_SET);
    read(*(s32 *) &file - 1, buffer, size);
}

static void
write_file(File *file, void *buffer, u64 offset, u64 size)
{
    assert(file);
    assert((u64) file <= 0x80000000);

    lseek(*(s32 *) &file - 1, offset, SEEK_SET);
    write(*(s32 *) &file - 1, buffer, size);
}

static void
close_file(File *file)
{
    assert(file);
    assert((u64) file <= 0x80000000);
    close(*(s32 *) &file - 1);
}
