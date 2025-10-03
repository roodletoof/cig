#include "allocator.h"

// TODO

static void *buffer_alloc(void *this, size_t bytes, const char *file, int line) {
    (void)this;
    (void)bytes;
    (void)file;
    (void)line;
    return 0;
}
static void *buffer_resize(void *this, void *old_ptr, size_t bytes, const char *file, int line) {
    (void)this;
    (void)old_ptr;
    (void)bytes;
    (void)file;
    (void)line;
    return 0;
}
static void buffer_free(void *this, void *ptr, const char *file, int line) {
    (void)this;
    (void)ptr;
    (void)file;
    (void)line;
}

static const allocator_vtbl_t buffer_vtbl = {
    .alloc = buffer_alloc,
    .resize = buffer_resize,
    .free = buffer_free,
};
