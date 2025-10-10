#include <criterion/criterion.h>
#include "allocator.h"

#define EXPECT 5
Test(borrow_allocator, test) {
    borrow_allocator_t balloc = borrow_allocator_create();
    int *ptr = borrow_allocator_alloc(&balloc, sizeof(int));
    *ptr = EXPECT;
    cr_assert_eq(*ptr, EXPECT);
    cr_assert_eq(1, borrow_allocator_count_allocations(&balloc));
    borrow_allocator_reset(&balloc);
    cr_assert_eq(0, borrow_allocator_count_allocations(&balloc));
    borrow_allocator_assert_all_freed(&balloc);
}

#define IS_SIZE 900
Test(borrow_allocator, with) {
    with_borrow(foo) {
        int *is = allocator_alloc(foo, sizeof(int) * IS_SIZE);
        for (int i = 0; i < IS_SIZE; i++) is[i] = i;
        is = allocator_alloc(foo, sizeof(int) * IS_SIZE);
        for (int i = 0; i < IS_SIZE; i++) is[i] = i;
        is = allocator_alloc(foo, sizeof(int) * IS_SIZE);
        for (int i = 0; i < IS_SIZE; i++) is[i] = i;
        break;
    }
}
