#include <criterion/criterion.h>
#include <stdbool.h>

enum animal {
    dog,
    cat,
};

typedef struct args {
    enum animal animal;
    bool is_true;
} args_t;

#define ARGS_DEFAULT .animal=cat, .is_true=true
#define ARGS(...) ((args_t){ ARGS_DEFAULT, __VA_ARGS__ })

Test(macro_magic, default_values) {
    args_t a1 = ARGS();
    args_t a2 = ARGS(.animal=dog);
    args_t a3 = ARGS(.animal=dog, .is_true=false);
    cr_assert_eq(a1.animal, cat);
    cr_assert_eq(a2.animal, dog);
    cr_assert_eq(a1.is_true, true);
    cr_assert_eq(a2.is_true, true);
    cr_assert_eq(a3.is_true, false);
    malloc(100);
    // TODO: make this shite crash the tests...
}
