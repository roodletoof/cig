#include "allocator.h"

static void *stdlib_alloc(void *this, size_t bytes, const char *file, int line) {
    (void)this;
    (void)file;
    (void)line;
    return malloc(bytes);
}
static void *stdlib_resize(void *this, void *old_ptr, size_t bytes, const char *file, int line) {
    (void)this;
    (void)file;
    (void)line;
    return realloc(old_ptr, bytes);
}
static void stdlib_free(void *this, void *ptr, const char *file, int line) {
    (void)this;
    (void)file;
    (void)line;
    free(ptr);
}

static const allocator_vtbl_t stdlib_vtbl = {
    .alloc = stdlib_alloc,
    .resize = stdlib_resize,
    .free = stdlib_free,
};

const allocator_t allocator_stdlib = (allocator_t) {
    .this=NULL,
    .vtbl=&stdlib_vtbl,
};

