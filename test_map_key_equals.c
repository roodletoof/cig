#include <criterion/criterion.h>
#include "cig.h"

Test(map_key_equals, ints) {
	{ // should be equal
		int key1 = 20;
		int key2 = 20;
		map_header_t header = (map_header_t) { 0 };
		header.key_size = sizeof(key1);
		cr_assert(
			map_key_equals(
				&header,
				(const uint8_t*)&key1,
				(const uint8_t*)&key2
			),
			"key %d is not equal to key %d",
			key1,
			key2
		);
	}
	{ // should not be equal
		int key1 = 20;
		int key2 = 21;
		map_header_t header = (map_header_t) { 0 };
		header.key_size = sizeof(key1);
		cr_assert(
			!map_key_equals(
				&header,
				(const uint8_t*)&key1,
				(const uint8_t*)&key2
			),
			"key %d is equal to key %d",
			key1,
			key2
		);
	}
}

static bool inverse_int_equals(const void *key1, const void *key2) {
	int key1_i = *((int*)key1);
	int key2_i = *((int*)key2);
	return key1_i != key2_i;
}

Test(map_key_equals, custom) {
	{ // should be equal
		int key1 = 20;
		int key2 = 21;
		map_header_t header = (map_header_t) { 0 };
		header.key_size = sizeof(key1);
		header.equals = inverse_int_equals;
		cr_assert(
			map_key_equals(
				&header,
				(const uint8_t*)&key1,
				(const uint8_t*)&key2
			),
			"key %d is equal to key %d",
			key1,
			key2
		);
	}
	{ // should not be equal
		int key1 = 20;
		int key2 = 20;
		map_header_t header = (map_header_t) { 0 };
		header.key_size = sizeof(key1);
		header.equals = inverse_int_equals;
		cr_assert(
			!map_key_equals(
				&header,
				(const uint8_t*)&key1,
				(const uint8_t*)&key2
			),
			"key %d is not equal to key %d",
			key1,
			key2
		);
	}
}
