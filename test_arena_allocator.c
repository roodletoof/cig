#include <criterion/criterion.h>
#include "cig.h"

Test(arena_allocator, repeated_allocations) {
    arena_allocator_t aalloc = arena_allocator_create();

    for ( int i = 0; i < 10; i++ ) {
        with_arena(&aalloc, allocator) {
            allocator_alloc(allocator, 10);
        }
    }

    cr_assert_eq(aalloc.total_allocated, 10);
    cr_assert_eq(borrow_allocator_count_allocations(&aalloc.borrow_allocator), 0);
    cr_assert_eq(dyn_array_capacity(aalloc.bytes), 10);

    arena_allocator_destroy(&aalloc);
    cr_assert_eq(aalloc.total_allocated, 0);
}

// TODO somehow test reallocations
