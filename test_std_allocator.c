#include <criterion/criterion.h>
#include "allocator.h"


Test(std_allocator, test) {
    allocator_t this = allocator_stdlib;
    void *ptr = allocator_alloc(this, 10);
    cr_assert(ptr != ((void *)0), "non null from malloc");
    void *new_ptr = allocator_resize(this, ptr, 1024*1024*500);
    cr_assert(new_ptr != ((void *)0), "non null from realloc");
    cr_assert(new_ptr != ptr, "realloc is not the same ptr as malloc");

    allocator_free(this, new_ptr);
}
