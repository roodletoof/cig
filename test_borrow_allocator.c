#include <criterion/criterion.h>
#include "cig.h"

#define EXPECT 5
Test(borrow_allocator, test) {
    allocator_t balloc = borrow_allocator(&borrow_allocator_create());
    int *ptr = allocator_alloc(balloc, sizeof(int));
    *ptr = EXPECT;
    cr_assert_eq(*ptr, EXPECT);
    allocator_reset(balloc);
}

#define IS_SIZE 900
Test(borrow_allocator, with) {
    with_borrow(foo) {
        with_borrow(bar) {
        } 
        int *is = allocator_alloc(foo, sizeof(int) * IS_SIZE);
        for (int i = 0; i < IS_SIZE; i++) is[i] = i;
        is = allocator_alloc(foo, sizeof(int) * IS_SIZE);
        for (int i = 0; i < IS_SIZE; i++) is[i] = i;
        is = allocator_alloc(foo, sizeof(int) * IS_SIZE);
        for (int i = 0; i < IS_SIZE; i++) is[i] = i;
        break;
    }
}
