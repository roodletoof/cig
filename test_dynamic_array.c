#include <criterion/criterion.h>
#include <stdbool.h>
#include "cig.h"

static bool int_eq(const void *a, const void *b) {
	return *(const int*)a == *(const int*)b;
}

Test(dynamic_arrays, append) {
	with_borrow(alloc) {
		int *numbers = make_arr(int, alloc);
		arr_append(numbers, 40);
		arr_append(numbers, 41);
		arr_append(numbers, 42);
		arr_append(numbers, 43);
		arr_append(numbers, 44);
		arr_append(numbers, 45);
		arr_append(numbers, 46);
		arr_append(numbers, 47);
		arr_append(numbers, 48);
		arr_append(numbers, 49);
		arr_append(numbers, 50);
		for (int i = 0; i < (int)arr_len(numbers); i++) {
			cr_assert_eq(numbers[i], i+40);
		}
	}
}

Test(dynamic_arrays, pop) {
	with_borrow(alloc) {
		int *numbers = make_arr(int, alloc);
		arr_append(numbers, 40);
		arr_append(numbers, 41);
		arr_append(numbers, 42);
		arr_append(numbers, 43);
		arr_append(numbers, 44);
		arr_append(numbers, 45);
		arr_append(numbers, 46);
		arr_append(numbers, 47);
		arr_append(numbers, 48);
		arr_append(numbers, 49);
		arr_append(numbers, 50);
		cr_assert_eq(arr_len(numbers), 11);
		// NOTE: you can stack for loops to have scoped variables you can abuse
		// in macros. e.g.
		// for (TYPE UNIQUE = (int)arr_len(numbers); UNIQUE != 0; UNIQUE = 0;)
		//	 for (int i = 0, i < UNIQUE; i++)
		int len = (int)arr_len(numbers);
		for (int i = 0; i < len; i++) {
			int num = arr_pop(numbers);
			cr_assert_eq(num, 50-i);
		}
	}
}

Test(dynamic_arrays, contains) {
	with_borrow(alloc) {
		int *numbers = make_arr(int, alloc);
		arr_append(numbers, 20);
		cr_expect(arr_contains(numbers, (int){20}));
		arr_reset(numbers);

		for ( size_t y = 0; y < 1000; y++ ) {
			for ( size_t i = 0; i < 100; i++ ) {
				if (!arr_contains(numbers, (int){i})) {
					arr_append(numbers, i);
				}
			}
		}
		cr_assert_eq(arr_len(numbers), 100);
	}
}



Test(dynamic_arrays, contains_found) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc, .initial_capacity = 5);
		arr_append(arr, 10);
		arr_append(arr, 20);
		arr_append(arr, 30);
		
		cr_assert(arr_contains(arr, (int){10}));
		cr_assert(arr_contains(arr, (int){20}));
		cr_assert(arr_contains(arr, (int){30}));
	}
}

Test(dynamic_arrays, contains_not_found) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc, .initial_capacity = 5);
		arr_append(arr, 10);
		arr_append(arr, 20);
		arr_append(arr, 30);
		
		cr_assert_not(arr_contains(arr, (int){5}));
		cr_assert_not(arr_contains(arr, (int){15}));
		cr_assert_not(arr_contains(arr, (int){99}));
	}
}

Test(dynamic_arrays, contains_empty) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc, .initial_capacity = 5);
		
		cr_assert_not(arr_contains(arr, (int){10}));
	}
}


Test(dynamic_arrays, contains_cmp_found) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc, .initial_capacity = 5);
		arr_append(arr, 10);
		arr_append(arr, 20);
		arr_append(arr, 30);
		
		cr_assert(arr_contains_cmp(arr, int_eq, (int){10}));
		cr_assert(arr_contains_cmp(arr, int_eq, (int){20}));
		cr_assert(arr_contains_cmp(arr, int_eq, (int){30}));
	}
}

Test(dynamic_arrays, contains_cmp_not_found) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc, .initial_capacity = 5);
		arr_append(arr, 10);
		arr_append(arr, 20);
		arr_append(arr, 30);
		
		cr_assert_not(arr_contains_cmp(arr, int_eq, (int){5}));
		cr_assert_not(arr_contains_cmp(arr, int_eq, (int){15}));
		cr_assert_not(arr_contains_cmp(arr, int_eq, (int){99}));
	}
}

Test(dynamic_arrays, contains_cmp_empty) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc, .initial_capacity = 5);
		
		cr_assert_not(arr_contains_cmp(arr, int_eq, (int){10}));
	}
}

// Test using arr_contains_cmp for a simple key-value map
typedef struct {
	int key;
	const char *value;
} kv_pair_t;

static bool kv_eq_by_key(const void *a, const void *b) {
	const kv_pair_t *pa = a;
	const kv_pair_t *pb = b;
	return pa->key == pb->key;
}

Test(dynamic_arrays, contains_cmp_key_value_map) {
	with_borrow(alloc) {
		kv_pair_t *map = make_arr(kv_pair_t, alloc, .initial_capacity = 10);
		
		// Add some key-value pairs
		arr_append(map, ((kv_pair_t){.key = 1, .value = "one"}));
		arr_append(map, ((kv_pair_t){.key = 5, .value = "five"}));
		arr_append(map, ((kv_pair_t){.key = 10, .value = "ten"}));
		
		// Check if keys exist
		cr_assert(arr_contains_cmp(map, kv_eq_by_key, (kv_pair_t){.key = 1}));
		cr_assert(arr_contains_cmp(map, kv_eq_by_key, (kv_pair_t){.key = 5}));
		cr_assert(arr_contains_cmp(map, kv_eq_by_key, (kv_pair_t){.key = 10}));
		
		// Check for non-existent keys
		cr_assert_not(arr_contains_cmp(map, kv_eq_by_key, (kv_pair_t){.key = 2}));
		cr_assert_not(arr_contains_cmp(map, kv_eq_by_key, (kv_pair_t){.key = 99}));
	}
}

Test(dynamic_arrays, get_valid_indices) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc);
		arr_append(arr, 10);
		arr_append(arr, 20);
		arr_append(arr, 30);
		arr_append(arr, 40);
		
		cr_assert_eq(arr_get(arr, 0), 10);
		cr_assert_eq(arr_get(arr, 1), 20);
		cr_assert_eq(arr_get(arr, 2), 30);
		cr_assert_eq(arr_get(arr, 3), 40);
	}
}

Test(dynamic_arrays, set_valid_indices) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc);
		arr_append(arr, 10);
		arr_append(arr, 20);
		arr_append(arr, 30);
		
		arr_set(arr, 0, 100);
		arr_set(arr, 1, 200);
		arr_set(arr, 2, 300);
		
		cr_assert_eq(arr_get(arr, 0), 100);
		cr_assert_eq(arr_get(arr, 1), 200);
		cr_assert_eq(arr_get(arr, 2), 300);
	}
}

Test(dynamic_arrays, get_out_of_bounds, .exit_code = 1) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc);
		arr_append(arr, 10);
		arr_append(arr, 20);
		dyn_array_bounds_check_func( arr, 2, __FILE__, __LINE__, false);
	}
}

Test(dynamic_arrays, set_out_of_bounds, .exit_code = 1) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc);
		arr_append(arr, 10);
		arr_append(arr, 20);
		
		dyn_array_bounds_check_func( arr, 2, __FILE__, __LINE__, false);
	}
}

Test(dynamic_arrays, get_empty_array, .exit_code = 1) {
	with_borrow(alloc) {
		int *arr = make_arr(int, alloc);
		dyn_array_bounds_check_func( arr, 0, __FILE__, __LINE__, false);
	}
}
