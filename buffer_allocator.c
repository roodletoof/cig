#include "allocator.h"

void *buffer_allocator_alloc(buffer_allocator_t *this, size_t bytes) {
    size_t new_size = this->size + bytes;
    if (new_size > this->capacity) {
        return NULL;
    }
    void *ptr = &this->data[this->size]
    this->size = new_size;
    return ptr;
}

void *buffer_allocator_resize(buffer_allocator_t *this, void *old_ptr, size_t bytes) {
    // TODO: check if the pointer is even in the buffer?
    void *new_ptr = buffer_allocator_alloc(this, bytes);
    if (new_ptr == NULL) {
        return NULL;
    }
    // TODO: copy over data
}

void buffer_allocator_reset(buffer_allocator_t *this) {
    this->size = 0;
}

static void *buffer_alloc(void *this, size_t bytes, const char *file, int line) {
    (void)file;
    (void)line;
    return buffer_allocator_alloc((buffer_allocator_t *)this, bytes);
}

static void *buffer_resize(void *this, void *old_ptr, size_t bytes, const char *file, int line) {
    (void)old_ptr;
    (void)file;
    (void)line;

    // TODO: do the concrete implementation first!
    void *new_ptr = buffer_allocator_alloc((buffer_allocator_t *)this, bytes);
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

allocator_t buffer_allocator_interface(buffer_allocator_t *this) {
}
