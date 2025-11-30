#include <criterion/criterion.h>
#include "cig.h"
#include <stdio.h>

Test(arena_allocator, repeated_allocations) {
    arena_allocator_t aalloc = arena_allocator_create();

    for ( int i = 0; i < 10; i++ ) with_arena(&aalloc, allocator) {
        allocator_alloc(allocator, 10);
    }

    cr_assert_eq(aalloc.to_allocate, 10);
    cr_assert_eq(borrow_allocator_count_allocations(&aalloc.borrow_allocator), 0);
    cr_assert_eq(dyn_array_capacity(aalloc.bytes), 10);

    arena_allocator_destroy(&aalloc);
    cr_assert_eq(aalloc.to_allocate, 0);
}

Test(arena_allocator, alignment) {
    arena_allocator_t aalloc = arena_allocator_create();

    for ( int i = 0; i < 10; i++ ) with_arena(&aalloc, allocator) {
        allocator_alloc(allocator, 1000);
    }

	with_arena(&aalloc, allocator) {
		size_t prev_addr = 0;
		prev_addr = ~prev_addr;
		for (int i = 0; i < 1001; i++) {
			size_t addr = (size_t)allocator_alloc(allocator, 1);
			cr_assert_neq(addr, prev_addr);
			cr_assert_eq(addr % MAX_ALIGN, 0);
		}
	}
	arena_allocator_reset(&aalloc);
	fprintf(stderr, "%zu\n", aalloc.to_allocate);
    cr_assert_eq(aalloc.to_allocate, MAX_ALIGN *  1000 + 1);
}

// TODO somehow test reallocations
