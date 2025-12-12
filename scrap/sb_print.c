#!/usr/bin/tcc -run

#define CIG_IMPL
#include "../cig.h"

int main(int argc, const char **argv) {
    with_borrow(allocator) {
        string_builder_t sb = sb_create(allocator);
        sb_add_string(&sb, "Age: ");
        sb_add_i64(&sb, 50);
        sb_add_string(&sb, "\n");
        sb_fprint(&sb, stdout);
    }
	return 0;
}
