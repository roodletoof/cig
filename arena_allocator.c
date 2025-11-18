#include "cig.h"
#include <assert.h>
#include <stdbool.h>


void *arena_allocator_alloc_func(
    arena_allocator_t *this,
    size_t bytes,
    const char *file,
    int line
) {
    buffer_allocator_t ba = (buffer_allocator_t) {
        .size=dyn_array_length(this->bytes),
        .capacity=dyn_array_capacity(this->bytes),
        .data=this->bytes
    };
    void *allocated = buffer_allocator_alloc(&ba, bytes);
    if (allocated != NULL) {
        return allocated;
    }
    allocated = borrow_allocator_alloc_func(&this->borrow_allocator, bytes, file, line);
    if (allocated != NULL) {
        this->total_allocated += bytes;
        return allocated;
    }
    return NULL;
}

void arena_allocator_reset(arena_allocator_t *this, const char *file, int line) {
    borrow_allocator_free_all(&this->borrow_allocator);
    if (this->bytes == NULL) {
        this->bytes = dyn_array_create_non_crashing_func(
            (dyn_array_create_non_crashing_func_args_t) {
                .allocator=allocator_stdlib(),
                .itemsize=sizeof(uint8_t),
                .initial_capacity=this->total_allocated,
                .file=file,
                .line=line,
            }
        );
    }
    if (this->bytes == NULL) {
        return;
    }
    if (dyn_array_capacity(this->bytes) < this->total_allocated) {
        size_t needed_bytes = this->total_allocated - dyn_array_length(this->bytes);
        uint8_t *new_bytes = dyn_array_grow_non_crashing_func(
            this->bytes,
            needed_bytes,
            file,
            line
        );
        if (new_bytes == NULL) {
            dyn_array_shrink_func(
                this->bytes,
                dyn_array_length(this->bytes),
                file,
                line
            );
            return;
        } else {
            this->bytes = new_bytes;
        }
    }
    if (this->bytes == NULL) {
        return;
    }
    dyn_array_shrink_func(this->bytes, dyn_array_length(this->bytes), file, line);
    return;
}

void arena_allocator_destroy(arena_allocator_t *this) {
    borrow_allocator_free_all(&this->borrow_allocator);
    dyn_array_destroy(this->bytes);
}

static void *arena_allocator_alloc_impl(void *this, size_t bytes, const char *file, int line) {
    return arena_allocator_alloc_func(
        (arena_allocator_t*) this,
        bytes,
        file,
        line
    );
}
static void *arena_allocator_resize_impl(void *this, void *old_ptr, size_t bytes, const char *file, int line) {
    (void)this;
    (void)file;
    (void)line;
    assert( false && "TODO: This is fucky. I need to see whether the allocation is in the buffer, if not then it is one of the borrow allocator nodes. I need an actual implementation here.");
    return realloc(old_ptr, bytes);
    return arena_allocator_alloc_func(
        (arena_allocator_t*) this,
        bytes,
        file,
        line
    );
}
static void arena_allocator_free_impl(void *this, void *ptr, const char *file, int line) {
    (void)this;
    (void)file;
    (void)line;
    free(ptr);
}

static const allocator_vtbl_t stdlib_vtbl = {
    .alloc = arena_allocator_alloc_impl,
    .resize = arena_allocator_resize_impl,
    .free = arena_allocator_free_impl,
};

allocator_t arena_allocator_interface(arena_allocator_t *this) {
}

