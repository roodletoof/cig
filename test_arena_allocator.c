#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include "cig.h"

Test(arena_allocator, test) {
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 4 * MB);
		for ( size_t i = 0; i < 1000; i++ ) {
			allocator_reset(arena);
			allocator_alloc(arena, 4 * MB);
		}

		borrow_allocator_t *concrete = (borrow_allocator_t*)backing.this;
		size_t n_allocations = 0;
		for (linked_allocation_node_t *node = concrete->head; node != NULL; node = node->next) {
			n_allocations++;
		}
		cr_assert_eq(n_allocations, 1);
	}

	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 0);
		for ( size_t i = 0; i < 1000; i++ ) {
			allocator_reset(arena);
			allocator_alloc(arena, 1 * MB);
			allocator_alloc(arena, 1 * MB);
			allocator_alloc(arena, 1 * MB);
			allocator_alloc(arena, 1 * MB);
		}

		borrow_allocator_t *concrete = (borrow_allocator_t*)backing.this;
		size_t n_allocations = 0;
		for (linked_allocation_node_t *node = concrete->head; node != NULL; node = node->next) {
			n_allocations++;
		}
		cr_assert_eq(n_allocations, 1);
	}

	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 0);
		for ( size_t i = 0; i < 1000; i++ ) {
			allocator_reset(arena);
			allocator_alloc(arena, 1 * MB);
			allocator_alloc(arena, 1 * MB);
			allocator_alloc(arena, 1 * MB);
			allocator_alloc(arena, 1 * MB);
		}

		borrow_allocator_t *concrete_borrow = (borrow_allocator_t*)backing.this;
		arena_allocator_t *concrete_arena = (arena_allocator_t*)arena.this;
		size_t n_allocations = 0;
		for (linked_allocation_node_t *node = concrete_borrow->head; node != NULL; node = node->next) {
			n_allocations++;
		}
		cr_assert_eq(n_allocations, 1);
		cr_assert_eq(concrete_arena->capacity, 4 * MB);
	}

	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 0);
		for ( size_t i = 0; i < 1000; i++ ) {
			allocator_reset(arena);
			allocator_alloc(arena, 1);
			allocator_alloc(arena, 1);
			allocator_alloc(arena, 1);
			allocator_alloc(arena, 1);
		}

		borrow_allocator_t *concrete_borrow = (borrow_allocator_t*)backing.this;
		arena_allocator_t *concrete_arena = (arena_allocator_t*)arena.this;
		size_t n_allocations = 0;
		for (linked_allocation_node_t *node = concrete_borrow->head; node != NULL; node = node->next) {
			n_allocations++;
		}
		cr_assert_eq(n_allocations, 1);
		const size_t expected_capacity = 4 * MAX_ALIGN;
		cr_assert_eq(concrete_arena->capacity, expected_capacity);
	}
}
