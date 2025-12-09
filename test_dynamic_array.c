#include <criterion/criterion.h>

#include <stdbool.h>

#include "cig.h"

Test(dynamic_arrays, append) {
	with_borrow(alloc) {
		int *numbers = make_arr(alloc, int);
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
		int *numbers = make_arr(alloc, int);
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
