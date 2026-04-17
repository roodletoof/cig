#define CIG_IMPL
#include "cig.h"

int main() {
	with_borrow(allocator) {
		int *ints;
		init_set(&ints, allocator, .initial_capacity=20);
		map_print_mapping_state(ints, stdout);
		set_add(ints, 10);
		map_print_mapping_state(ints, stdout);
	}
}
