#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdlib.h>

// Contains all operations an allocator can do. Similar interface to sdtlibs
// malloc, realloc and free.
typedef struct allocator_vtbl {
	void *(*alloc)(void *this, size_t bytes, const char *file, int line);
	void *(*resize)(void *this, void *old_ptr, size_t bytes, const char *file, int line);
	void (*free)(void *this, void *ptr, const char *file, int line);
} allocator_vtbl_t;

// An instance of an allocator.
typedef struct allocator {
	// pointer to the behind-the-scenes data that an allocator may store.
	void *const this;
	// pointer to the method implementations of an allocator.
	const allocator_vtbl_t *const vtbl;
} allocator_t;

void *allocator_alloc_func(allocator_t this, size_t bytes, const char *file, int line);
void *allocator_resize_func(allocator_t this, void *old_ptr, size_t bytes, const char *file, int line);
void allocator_free_func(allocator_t this, void *ptr, const char *file, int line);
#define allocator_alloc(this, bytes) allocator_alloc_func(this, bytes, __FILE__, __LINE__)
#define allocator_resize(this, old_ptr, bytes) allocator_resize_func(this, old_ptr, bytes, __FILE__, __LINE__)
#define allocator_free(this, ptr) allocator_free_func(this, ptr, __FILE__, __LINE__)

extern const allocator_t allocator_stdlib;

#ifdef ALLOCATOR_IMPLEMENTATION

void *allocator_alloc_func(allocator_t this, size_t bytes, const char *file, int line) {
	return this.vtbl->alloc(this.this, bytes, file, line);
}
void *allocator_resize_func(allocator_t this, void *old_ptr, size_t bytes, const char *file, int line) {
	return this.vtbl->resize(this.this, old_ptr, bytes, file, line);
}
void allocator_free_func(allocator_t this, void *ptr, const char *file, int line) {
	this.vtbl->free(this.this, ptr, file, line);
}

#include "std_allocator.c"

#endif // ALLOCATOR_IMPLEMENTATION
#endif // ALLOCATOR_H
