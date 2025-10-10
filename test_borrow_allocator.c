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


// TODO: when I update the number of allocations it does not seem to change the
// number of allocated bytes...
Test(borrow_allocator, with) {
    with_borrow(foo) {
        allocator_alloc(foo, 1000);
        allocator_alloc(foo, 100);
        allocator_alloc(foo, 90);
        break;
    }
}
