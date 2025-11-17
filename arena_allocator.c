#include "cig.h"


void *arena_allocator_alloc_func(
    arena_allocator_t *this,
    size_t bytes,
    const char *file,
    int line
) {
    allocator_t std_alloc = allocator_stdlib();
    buffer_allocator_t ba = (buffer_allocator_t) {
        .size=dyn_array_length(this->bytes),
        .capacity=dyn_array_capacity(this->bytes),
        .data=this->bytes
    };
}

void arena_allocator_reset(arena_allocator_t *this) {
}

void arena_allocator_destroy(arena_allocator_t *this) {
}

allocator_t arena_allocator_interface(arena_allocator_t *this) {
}

