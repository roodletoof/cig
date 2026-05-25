#include <criterion/criterion.h>
#include "cig.h"

#define KEYS_LEN 100

Test(map_hash_key, foo) {
	map_header_t header = (map_header_t){0};
	long long key_hashes[KEYS_LEN];

	for (long long i = 0; i < KEYS_LEN; i++) {
		header.key_size = sizeof(i);
		key_hashes[i] = map_hash_key(&header, (const uint8_t*)(&i));
		// TODO
	}
	
	map_hash_key(&header)
}
