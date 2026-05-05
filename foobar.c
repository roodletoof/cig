#define CIG_IMPL
#include "cig.h"

int main(void) {
	with_borrow(allocator) {
		int *s;
		set_init(&s, allocator);
		map_print_mapping_state((void*)s, stdout);
		fflush(stdout);

		for (int i = 0; i < 10; i++) {
			set_add(&s, 1);
			map_print_mapping_state((void*)s, stdout);
			fflush(stdout);
		}
	}
}
