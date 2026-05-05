#include <criterion/criterion.h>
#include "cig.h"
#include <stdio.h>

#define FLAG_FREE      (-1)
#define FLAG_TOMBSTONE (-2)

static int count_filled(void *map) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, map);
	int count = 0;
	for (unsigned int i = 0; i < header->mapping_capacity; i++) {
		if (header->mapping_arr[i] >= 0) {
			count++;
		}
	}
	return count;
}

Test(map, test) {
	with_borrow(allocator) {
		int *s;
		set_init(&s, allocator);
		int count = count_filled((void*)s);
		cr_assert_eq(count, 0, "%d != 0", count);

		for (int i = 0; i < 10; i++) {
			set_add(&s, 1);
			count = count_filled((void*)s);
			cr_assert_eq(count, 1, "%d != 0", count);
			map_print_mapping_state((void*)s, stdout);
			fflush(stdout);
		}
	}
}
