#include <criterion/criterion.h>
#include "cig.h"

static void test_buffer_alloc(buffer_allocator_t impl) {
    allocator_t inter = buffer_allocator(&impl);
    const int n_ints = 100;
    int *ints = allocator_alloc(inter, sizeof(int) * n_ints);
    for (int i = 0; i < n_ints; i++) {
        ints[i] = i;
    }
    for (int i = 0; i < n_ints; i++) {
        cr_assert_eq(ints[i], i);
    }
    cr_assert_eq(impl.size, sizeof(int)*n_ints);
    ints = allocator_alloc(inter, sizeof(int) * n_ints);
    for (int i = 0; i < n_ints; i++) {
        ints[i] = i;
    }
    for (int i = 0; i < n_ints; i++) {
        cr_assert_eq(ints[i], i);
    }
    cr_assert_eq(impl.size, sizeof(int)*2*n_ints);
    size_t double_ints = ((size_t) n_ints) * 2;
    void *should_be_addr = allocator_resize(inter, ints, double_ints);
    cr_assert_neq(should_be_addr, NULL);
    allocator_reset(inter);
}

Test(buffer_allocator, test) {
    buffer_allocator_t impl = buffer_allocator_create(MB);
    test_buffer_alloc(impl);
    cr_assert_eq(impl.size, 0);
    test_buffer_alloc(impl);
}
