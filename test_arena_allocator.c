#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <string.h>
#include "cig.h"

Test(arena_allocator, basic_allocation) {
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

Test(arena_allocator, resize_preserves_data) {
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 1 * KB);
		allocator_reset(arena);
		
		// Allocate a buffer and fill it with data
		int *data = allocator_alloc(arena, sizeof(int) * 10);
		for (int i = 0; i < 10; i++) {
			data[i] = i * 100;
		}
		
		// Resize to larger size - should preserve existing data
		int *resized = allocator_resize(arena, data, sizeof(int) * 20);
		
		// Verify old data is preserved
		for (int i = 0; i < 10; i++) {
			cr_assert_eq(resized[i], i * 100, "Expected resized[%d] = %d, got %d", i, i * 100, resized[i]);
		}
		
		// Fill the new space
		for (int i = 10; i < 20; i++) {
			resized[i] = i * 100;
		}
		
		// Verify all data
		for (int i = 0; i < 20; i++) {
			cr_assert_eq(resized[i], i * 100);
		}
	}
}

Test(arena_allocator, resize_smaller_preserves_partial_data) {
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 1 * KB);
		allocator_reset(arena);
		
		// Allocate a buffer and fill it with data
		int *data = allocator_alloc(arena, sizeof(int) * 20);
		for (int i = 0; i < 20; i++) {
			data[i] = i * 7;
		}
		
		// Resize to smaller size - should preserve first N elements
		int *resized = allocator_resize(arena, data, sizeof(int) * 10);
		
		// Verify first 10 elements are preserved
		for (int i = 0; i < 10; i++) {
			cr_assert_eq(resized[i], i * 7, "Expected resized[%d] = %d, got %d", i, i * 7, resized[i]);
		}
	}
}

Test(arena_allocator, resize_with_zero_capacity) {
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 1 * KB);
		allocator_reset(arena);
		
		// Simulate dyn_array with capacity 0 behavior
		typedef struct {
			size_t size;
			size_t capacity;
			size_t itemsize;
			allocator_t allocator;
			uint8_t bytes[];
		} test_header_t;
		
		// Allocate just the header (capacity = 0)
		test_header_t *header = allocator_alloc(arena, sizeof(test_header_t));
		header->size = 0;
		header->capacity = 0;
		header->itemsize = sizeof(int);
		
		// Resize to add space for 1 element
		header = allocator_resize(arena, header, sizeof(test_header_t) + sizeof(int) * 1);
		cr_assert_neq(header, NULL);
		header->capacity = 1;
		
		// Add an element
		int *data = (int*)header->bytes;
		data[0] = 42;
		header->size = 1;
		
		// Resize again to capacity 2
		header = allocator_resize(arena, header, sizeof(test_header_t) + sizeof(int) * 2);
		cr_assert_neq(header, NULL);
		header->capacity = 2;
		
		// Verify first element is preserved
		data = (int*)header->bytes;
		cr_assert_eq(data[0], 42);
		
		// Add second element
		data[1] = 99;
		header->size = 2;
		
		// Verify both elements
		cr_assert_eq(data[0], 42);
		cr_assert_eq(data[1], 99);
	}
}

Test(arena_allocator, dyn_array_resize_pattern) {
	// Test the actual pattern used by dynamic arrays
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 10 * KB);
		allocator_reset(arena);
		
		int *arr = make_arr(int, arena);
		
		// Add 100 elements, forcing multiple resizes
		for (int i = 0; i < 100; i++) {
			arr_append(arr, i * 3);
		}
		
		// Verify all elements
		cr_assert_eq(arr_len(arr), 100);
		for (int i = 0; i < 100; i++) {
			cr_assert_eq(arr[i], i * 3, "Expected arr[%d] = %d, got %d", i, i * 3, arr[i]);
		}
	}
}

Test(arena_allocator, multiple_arrays_resize) {
	// Test multiple dynamic arrays resizing independently
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 20 * KB);
		allocator_reset(arena);
		
		int *arr1 = make_arr(int, arena);
		int *arr2 = make_arr(int, arena);
		
		// Add to arr1
		for (int i = 0; i < 50; i++) {
			arr_append(arr1, i);
		}
		
		// Add to arr2
		for (int i = 0; i < 30; i++) {
			arr_append(arr2, i * 2);
		}
		
		// Add more to arr1
		for (int i = 50; i < 100; i++) {
			arr_append(arr1, i);
		}
		
		// Verify arr1
		cr_assert_eq(arr_len(arr1), 100);
		for (int i = 0; i < 100; i++) {
			cr_assert_eq(arr1[i], i);
		}
		
		// Verify arr2
		cr_assert_eq(arr_len(arr2), 30);
		for (int i = 0; i < 30; i++) {
			cr_assert_eq(arr2[i], i * 2);
		}
	}
}

Test(arena_allocator, resize_with_string_data) {
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 1 * KB);
		allocator_reset(arena);
		
		// Allocate and fill with string data
		char *str = allocator_alloc(arena, 10);
		strcpy(str, "Hello");
		
		// Resize to larger
		str = allocator_resize(arena, str, 20);
		
		// Verify string is preserved
		cr_assert_str_eq(str, "Hello");
		
		// Append more
		strcat(str, " World!");
		cr_assert_str_eq(str, "Hello World!");
	}
}

Test(arena_allocator, nested_structures_with_resize) {
	// Test resizing with nested pointer structures (like 2D arrays)
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 10 * KB);
		allocator_reset(arena);
		
		// Create array of int pointers (like dyn_array of dyn_arrays)
		int **rows = make_arr(int*, arena);
		
		// Add 5 rows
		for (int i = 0; i < 5; i++) {
			int *row = make_arr(int, arena);
			for (int j = 0; j < 10; j++) {
				arr_append(row, i * 10 + j);
			}
			arr_append(rows, row);
		}
		
		// Verify data
		cr_assert_eq(arr_len(rows), 5);
		for (int i = 0; i < 5; i++) {
			cr_assert_eq(arr_len(rows[i]), 10);
			for (int j = 0; j < 10; j++) {
				cr_assert_eq(rows[i][j], i * 10 + j);
			}
		}
	}
}

Test(arena_allocator, large_resize_chain) {
	// Test many consecutive resizes
	with_borrow(backing) {
		allocator_t arena = arena_allocator_create(backing, 50 * KB);
		allocator_reset(arena);
		
		int *data = allocator_alloc(arena, sizeof(int) * 1);
		data[0] = 123;
		
		// Resize many times, each time doubling
		for (int iteration = 0; iteration < 10; iteration++) {
			size_t old_size = 1 << iteration;
			size_t new_size = 1 << (iteration + 1);
			
			data = allocator_resize(arena, data, sizeof(int) * new_size);
			
			// Verify first element is still preserved
			cr_assert_eq(data[0], 123);
			
			// Fill new elements
			for (size_t i = old_size; i < new_size; i++) {
				data[i] = (int)i;
			}
		}
		
		// Final size should be 1024
		cr_assert_eq(data[0], 123);
		for (int i = 1; i < 1024; i++) {
			cr_assert_eq(data[i], i);
		}
	}
}
