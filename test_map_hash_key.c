#include <stdbool.h>

#include <criterion/criterion.h>
#include "cig.h"

typedef long long T;
#define CAPACITY 50

Test(map_hash_key, foo) {
	map_header_t header = (map_header_t){0};
	header.key_size = sizeof(T);
	header.mapping_capacity = CAPACITY;

	bool hashed[CAPACITY] = {0};

	for (T i = 0; i < CAPACITY; i++) {
		uint32_t index = map_hash_key(&header, (const uint8_t*)(&i));
		cr_assert(index < CAPACITY);
		hashed[index] = true;
	}

	int n_unique = 0;

	for (int i = 0; i < CAPACITY; i++) {
		if (hashed[i]) {
			n_unique++;
		}
	}

	cr_assert_gt(n_unique, 1);
	cr_assert_lt(n_unique, CAPACITY);

	printf("Expect high number below 50. Got %d unique hashes.\nThis means %d collisions\n", n_unique, (int)(CAPACITY - n_unique));
}

// TODO: test hash with custom hash function.
