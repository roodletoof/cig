#include <criterion/criterion.h>

#include <stdbool.h>

#include "cig.h"

Test(dynamic_arrays, append) {
    with_borrow(alloc) {
        int *numbers = dyn_array_create(alloc, int);
        dyn_array_append(numbers, 40);
        dyn_array_append(numbers, 41);
        dyn_array_append(numbers, 42);
        dyn_array_append(numbers, 43);
        dyn_array_append(numbers, 44);
        dyn_array_append(numbers, 45);
        dyn_array_append(numbers, 46);
        dyn_array_append(numbers, 47);
        dyn_array_append(numbers, 48);
        dyn_array_append(numbers, 49);
        dyn_array_append(numbers, 50);
        for (int i = 0; i < (int)dyn_array_length(numbers); i++) {
            cr_assert_eq(numbers[i], i+40);
        }
    }
}

Test(dynamic_arrays, pop) {
    with_borrow(alloc) {
        int *numbers = dyn_array_create(alloc, int);
        dyn_array_append(numbers, 40);
        dyn_array_append(numbers, 41);
        dyn_array_append(numbers, 42);
        dyn_array_append(numbers, 43);
        dyn_array_append(numbers, 44);
        dyn_array_append(numbers, 45);
        dyn_array_append(numbers, 46);
        dyn_array_append(numbers, 47);
        dyn_array_append(numbers, 48);
        dyn_array_append(numbers, 49);
        dyn_array_append(numbers, 50);
        int len = (int)dyn_array_length(numbers);
        for (int i = 0; i < len; i++) {
            int num = dyn_array_pop(numbers);
            cr_assert_eq(num, 50-i);
        }
    }
}
