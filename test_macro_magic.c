#include <criterion/criterion.h>

enum animal {
    dog,
    cat,
};

typedef struct args {
    enum animal animal;
} args_t;

#define ARGS_DEFAULT .animal=cat
#define ARGS(...) ((args_t){ ARGS_DEFAULT, __VA_ARGS__ })

Test(macro_magic, default_values) {
    args_t a1 = ARGS();
    args_t a2 = ARGS(.animal=dog);
    cr_assert_eq(a1.animal, cat);
    cr_assert_eq(a2.animal, dog);
}
