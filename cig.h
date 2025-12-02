#ifndef CIG_H
#define CIG_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef union any_align { char c; int i; long l; long long ll; float f; double d; void *p; } any_align_t;
#define MAX_ALIGN ((size_t) sizeof(any_align_t))
#define KB (1024)
#define MB (KB * KB)
#define GB (KB * KB * KB)
#define OFFSET(STRUCT, FIELD) ((size_t) (&((STRUCT*) NULL)->FIELD))
#define PTR_FROM_FIELD_PTR(STRUCT, FIELD, PTR) ((STRUCT *) (((char *) PTR) - OFFSET(STRUCT, FIELD)))


// Contains all operations an allocator can do. Similar interface to sdtlibs
// malloc, realloc and free.
typedef struct allocator_vtbl {
    void *(*alloc)(void *this, size_t bytes);
    void *(*resize)(void *this, void *old_ptr, size_t bytes);
    void (*reset)(void *this);
} allocator_vtbl_t;

// An instance of an allocator.
typedef struct allocator {
    // pointer to the behind-the-scenes data that an allocator may store.
    void *this;
    // pointer to the method implementations of an allocator.
    const allocator_vtbl_t *vtbl;
} allocator_t;

void *allocator_alloc_func(allocator_t this, size_t bytes, const char *file, int line);
void *allocator_resize_func(allocator_t this, void *old_ptr, size_t bytes, const char *file, int line);
void allocator_reset(allocator_t this);
#define allocator_alloc(this, bytes) allocator_alloc_func(this, bytes, __FILE__, __LINE__)
#define allocator_resize(this, old_ptr, bytes) allocator_resize_func(this, old_ptr, bytes, __FILE__, __LINE__)

// std_allocator ///////////////////////////////////////////////////////////////
allocator_t forever_allocator();
// buffer_allocator ////////////////////////////////////////////////////////////
typedef struct buffer_allocator {
    size_t size, capacity;
    uint8_t *data;
} buffer_allocator_t;

#define buffer_allocator_create(CAPACITY) \
  ((buffer_allocator_t){ \
      .size = 0, .capacity = CAPACITY, .data = (uint8_t[CAPACITY]){0}})

allocator_t buffer_allocator(buffer_allocator_t *this);

// borrow_allocator ////////////////////////////////////////////////////////////
typedef struct linked_allocation_node {
    struct linked_allocation_node *next;
    struct linked_allocation_node *prev;
    any_align_t data[];
} linked_allocation_node_t;

typedef struct borrow_allocator {
    linked_allocation_node_t *head;
} borrow_allocator_t;

#define borrow_allocator_create() ((borrow_allocator_t){.head=NULL})

allocator_t borrow_allocator(borrow_allocator_t *this);

// Some text that can be used as an identifier (no, not by you), so that I can
// use a variable that won't collide with yours inside macros.
#define UNIQUE __macro_internal_34bba35b8b9b20a75f9881e3795630e25d36e620d9c9741e2e9141ba82ec6ef6__

// Can be used to mimic rust borrow semantics. with_borrow(foos_allocator) {
// foo(foos_allocator); }. If a function should return ownership assume that
// my_allocator was passed in by an outer function void *bar;
// with_borrow(foos_allocator) {bar = foo(foos_allocator, my_allocator); } Using
// the return a keyword in the statement following this macro will cause a
// guaranteed memory leak.
// TODO: simplify!
#define with_borrow(NAME)                                                      \
  for (allocator_t NAME =                                                      \
           borrow_allocator(&borrow_allocator_create());             \
       !((borrow_allocator_t *)NAME.this)->head;                               \
       ((borrow_allocator_t *)NAME.this)->head =                               \
           (allocator_reset(NAME),      \
            (linked_allocation_node_t *)1))                                    \
    for (int UNIQUE = 0; UNIQUE < 1; UNIQUE++)

// dynamic arrays //////////////////////////////////////////////////////////////

typedef struct dyn_array_header {
    size_t size, capacity, itemsize;
    allocator_t allocator;
    uint8_t bytes[];
} dyn_array_header_t;

typedef struct dyn_array_create_func_args {
    allocator_t allocator;
    size_t itemsize;
    size_t initial_capacity;
    const char *file;
    int line;
} dyn_array_create_func_args_t;
void *dyn_array_create_func(dyn_array_create_func_args_t args);
#define dyn_array_create(ALLOCATOR, TYPE, ...)                                 \
  ((TYPE *)dyn_array_create_func(                                              \
      (dyn_array_create_func_args_t){.allocator = ALLOCATOR,                   \
                                     .itemsize = sizeof(TYPE),                 \
                                     .file = __FILE__,                         \
                                     .line = __LINE__,                         \
                                     __VA_ARGS__}))
// Always reassign the array. if multiple variables reference the same growing
// array, then you should be using pointer pointers.
void *dyn_array_grow_func(void *this, size_t n_new_items, const char *file, int line);
void dyn_array_shrink_func(void *this, size_t n_items_to_remove, const char *file, int line);

#define dyn_array_grow(THIS, N_NEW_ITEMS)                                      \
  dyn_array_grow_func(THIS, N_NEW_ITEMS, __FILE__, __LINE__)

#define dyn_array_shrink(THIS, N_ITEMS_TO_REMOVE)                              \
  dyn_array_shrink_func(THIS, N_ITEMS_TO_REMOVE, __FILE__, __LINE__)

#define dyn_array_reset(THIS)                                                  \
  do {                                                                         \
    dyn_array_shrink(THIS, dyn_array_length(THIS));                            \
  } while (0)

typedef struct dyn_array_create_non_crashing_func_args {
    allocator_t allocator;
    size_t itemsize;
    size_t initial_capacity;
    const char *file;
    size_t line;
} dyn_array_create_non_crashing_func_args_t;
void *dyn_array_create_non_crashing_func(dyn_array_create_non_crashing_func_args_t args);
// This version returns a NULL pointer instead of crashing if the allocator return NULL.
// It is up to you to check that the pointer returned isn't NULL
#define dyn_array_create_non_crashing(ALLOCATOR, TYPE, ...)                    \
  ((TYPE *)dyn_array_create_non_crashing_func(                                 \
      (dyn_array_create_non_crashing_func_args_t){.allocator = ALLOCATOR,      \
                                                  .itemsize = sizeof(TYPE),    \
                                                  .file = __FILE__,            \
                                                  .line = __LINE__,            \
                                                  __VA_ARGS__}))
// TODO: remove the non-crashing versions. they crash...
// This version returns a NULL pointer instead of crashing if the allocator return NULL.
// It is up to you to check that the pointer returned isn't NULL
// Always reassign the array. if multiple variables reference the same growing
// array, then you should be using pointer pointers.
void *dyn_array_grow_non_crashing_func(void *this, size_t n_new_items, const char *file, int line);
#define dyn_array_grow_non_crashing(THIS, N_NEW_ITEMS) dyn_array_grow_non_crashing_func(THIS, N_NEW_ITEMS)

size_t dyn_array_length(void *this);
size_t dyn_array_capacity(void *this);

#define dyn_array_append(THIS, VAL) do { \
    THIS = dyn_array_grow(THIS, 1); \
    THIS[dyn_array_length(THIS)-1] = VAL; \
} while(0)

#define dyn_array_pop(THIS) (dyn_array_shrink(THIS, 1), THIS[dyn_array_length(THIS)])

#ifdef CIG_IMPL

void *allocator_alloc_func(allocator_t this, size_t bytes, const char *file, int line) {
    void *ptr = this.vtbl->alloc(this.this, bytes);
    if (ptr == NULL) {
        fprintf(stderr, "%s:%d: alloc returned NULL\n", file, line);
        exit(1);
    }
    return ptr;
}
void *allocator_resize_func(allocator_t this, void *old_ptr, size_t bytes, const char *file, int line) {
    void *ptr = this.vtbl->resize(this.this, old_ptr, bytes);
    if (ptr == NULL) {
        fprintf(stderr, "%s:%d: alloc returned NULL\n", file, line);
        exit(1);
    }
    return ptr;
}
void allocator_reset(allocator_t this) {
    this.vtbl->reset(this.this);
}

#include "std_allocator.c"
#include "buffer_allocator.c"
#include "borrow_allocator.c"
#include "dyn_array.c"

#endif // CIG_IMPL
#endif // CIG_H
