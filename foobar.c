#define CIG_IMPL
#include "cig.h"

int main(void) {
	with_borrow(allocator) {
		int *ints;
		set_init(&ints, allocator, .initial_capacity=20);
		map_print_mapping_state(ints, stdout);
		for ( int i = 0; i < 20; i++ ) {
			set_add(&ints, i);
			map_print_mapping_state(ints, stdout);
		}
	}
}
