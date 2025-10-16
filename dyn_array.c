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

void *dyn_array_append_func(void *this, void *item, const char *file, int line) {
    void *bytes = dyn_array_append_non_crashing_func(this, item);
    if (bytes == NULL) {
        fprintf(
            stderr,
            "%s:%d: allocator returned NULL\n",
            file,
            line
        );
        exit(1);
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

void *dyn_array_append_non_crashing_func(void *this, void *item) {
    dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
    if (header->capacity <= header->size) {
        size_t new_capacity = header->capacity * 2;
        if (new_capacity == 0) { new_capacity = 1; }
        header = allocator_resize(
            header->allocator,
            header,
            sizeof(dyn_array_header_t) + header->itemsize * new_capacity
        );
        if (header == NULL) { return NULL; }
        header->capacity = new_capacity;
    }
    &header->bytes[header->itemsize*header->size]
// typedef struct dyn_array_header {
//     size_t size, capacity, itemsize;
//     allocator_t allocator;
//     uint8_t bytes[];
// } dyn_array_header_t;

// TODO: this is not good. the type has to be passed into the macro or
// something because passing in an integer does not work. it won't know to cast
// it to a char, int, long, whatever, so the bytes will be fucked. Either the
// macro can take the type to cast the value into the type, or we can just have
// some grow function instead...
}

size_t dyn_array_length(void *this) {
    dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
}

size_t dyn_array_capacity(void *this); {
    dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
}


