#define CIG_IMPL
#include "cig.h"
#include <stdio.h>
#include <assert.h>

int main() {
	// test 1///////////////////////////////////////////////////////////////////////
	arena_allocator_t aalloc = arena_allocator_create();

	for ( int i = 0; i < 10; i++ ) with_arena(&aalloc, allocator) {
		allocator_alloc(allocator, 10);
	}

	assert(aalloc.to_allocate == 10);
	assert(borrow_allocator_count_allocations(&aalloc.borrow_allocator) == 0);
	assert(dyn_array_capacity(aalloc.bytes) == 10);

	arena_allocator_destroy(&aalloc);
	assert(aalloc.to_allocate == 0);
	arena_allocator_destroy(&aalloc);

	// test 2 //////////////////////////////////////////////////////////////////////
	aalloc = arena_allocator_create();

	for ( int i = 0; i < 10; i++ ) with_arena(&aalloc, allocator) {
		allocator_alloc(allocator, 1000);
	}

	with_arena(&aalloc, allocator) {
		size_t prev_addr = 0;
		prev_addr = ~prev_addr;
		for (int i = 0; i < 1001; i++) {
			size_t addr = (size_t)allocator_alloc(allocator, 1);
			assert(addr != prev_addr);
			assert(addr % MAX_ALIGN == 0);
		}
	}
	arena_allocator_reset(&aalloc);
	fprintf(stderr, "%zu\n", aalloc.to_allocate);
	//assert(aalloc.to_allocate == MAX_ALIGN *  1000 + 1);
	// TODO: WHAT is happening here?
	arena_allocator_destroy(&aalloc);
}
