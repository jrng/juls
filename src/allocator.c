typedef struct
{
    u64 capacity;
    u64 occupied;
    u8 *memory;
} Allocator;

typedef struct
{
    u64 capacity;
    u64 occupied;
    u8 *memory;
} AllocatorFooter;

static inline u64
get_alignment_offset(Allocator *allocator, u64 alignment)
{
    u64 alignment_offset = 0;
    u64 next_pointer = (u64) (allocator->memory + allocator->occupied);
    u64 alignment_mask = alignment - 1;

    if (next_pointer & alignment_mask)
    {
        alignment_offset = alignment - (next_pointer & alignment_mask);
    }

    return alignment_offset;
}

#define alloc_type(allocator, type, alignment, clear) (type *) alloc(allocator, sizeof(type), alignment, clear)
#define alloc_array(allocator, type, count, alignment, clear) (type *) alloc(allocator, (count) * sizeof(type), alignment, clear)

static void *
alloc(Allocator *allocator, u64 size, u64 alignment, bool clear)
{
    u64 alignment_offset = get_alignment_offset(allocator, alignment);

    if ((allocator->occupied + alignment_offset + size) > allocator->capacity)
    {
        u64 allocate_size = 64 * 1024;
        u64 required_size = Align(size + sizeof(AllocatorFooter), 16 * 1024);

        if (required_size > allocate_size)
        {
            allocate_size = required_size;
        }

        u64 capacity = allocator->capacity;
        u64 occupied = allocator->occupied;
        u8 *memory = allocator->memory;

        allocator->capacity = allocate_size - sizeof(AllocatorFooter);
        allocator->occupied = 0;
        allocator->memory = (u8 *) allocate(allocate_size);

        AllocatorFooter *footer = (AllocatorFooter *) (allocator->memory + allocator->capacity);

        footer->capacity = capacity;
        footer->occupied = occupied;
        footer->memory = memory;

        alignment_offset = get_alignment_offset(allocator, alignment);
    }

    void *result = allocator->memory + allocator->occupied + alignment_offset;
    allocator->occupied += alignment_offset + size;

    if (clear)
    {
        u8 *dst = (u8 *) result;
        while (size--) *dst++ = 0;
    }

    return result;
}

static void *
realloc(Allocator *allocator, void *old_ptr, u64 old_size, u64 new_size, u64 alignment, bool clear)
{
    void *result = alloc(allocator, new_size, alignment, clear);

    if (old_ptr)
    {
        u64 size = old_size;

        u8 *dst = (u8 *) result;
        u8 *src = (u8 *) old_ptr;

        while (size--) *dst++ = *src++;
    }

    return result;
}

static void
free_all(Allocator *allocator)
{
    while (allocator->memory)
    {
        void *memory = allocator->memory;
        u64 size = allocator->capacity + sizeof(AllocatorFooter);

        AllocatorFooter *footer = (AllocatorFooter *) (allocator->memory + allocator->capacity);

        allocator->capacity = footer->capacity;
        allocator->occupied = footer->occupied;
        allocator->memory = footer->memory;

        deallocate(memory, size);
    }
}
