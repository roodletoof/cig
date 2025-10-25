#include "cig.h"
#include <stdio.h>

void *dyn_array_create_func(dyn_array_create_func_args_t args) {
    void *bytes = dyn_array_create_non_crashing_func(
        (dyn_array_create_non_crashing_func_args_t) {
            .allocator=args.allocator,
            .itemsize=args.itemsize,
            .initial_capacity=args.initial_capacity,
        }
    );
    if (bytes == NULL) {
        fprintf(
            stderr,
            "%s:%d: allocator returned NULL\n",
            args.file,
            args.line
        );
        exit(1);
    }
    return bytes;
}

void *dyn_array_grow_func(void *this, size_t n_new_items, const char *file, int line) {
    void *bytes = dyn_array_grow_non_crashing_func(this, n_new_items);
    if (bytes == NULL) {
        fpritnf(
            stderr,
            "%s:%d: allocator returned NULL\n",
            file,
            line
        );
    }
    return bytes;
}

void *dyn_array_create_non_crashing_func(dyn_array_create_non_crashing_func_args_t args) {
    dyn_array_header_t *header = allocator_alloc(
        args.allocator,
        sizeof(dyn_array_header_t) + args.itemsize * args.initial_capacity
    );
    if (header == NULL) { return NULL; }
    header->size = 0;
    header->capacity = args.initial_capacity;
    header->itemsize = args.itemsize;
    header->allocator = args.allocator;
    return &header->bytes;
}

void *dyn_array_grow_non_crashing_func(void *this, size_t n_new_items) {
    dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
    size_t new_size = header->size + n_new_items;

    if (header->capacity < new_size) {
        size_t cap_times_2 = header->capacity * 2;
        size_t new_capacity =
            cap_times_2 > new_size
            ? cap_times_2
            : new_size;
        header = allocator_resize(
            header->allocator,
            header,
            sizeof(dyn_array_header_t) + header->itemsize * new_capacity
        );
        if (header == NULL) { return NULL; }
        header->capacity = new_capacity;
    }
    return &header->bytes
}

size_t dyn_array_length(void *this) {
    dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
}

size_t dyn_array_capacity(void *this); {
    dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
}


