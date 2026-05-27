#include "cig.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#define FLAG_FREE      (-1)
#define FLAG_TOMBSTONE (-2)


// TODO: when setting a key to a gravestone, then while the mapping
// item to the right is FLAG_FREE, you can set this one to FLAG_FREE,
// go one index less, and if that is also a FLAG_GRAVESTONE, you can
// continue the process.

#define ESC "\033["
#define RED ESC "31m"
#define DEFAULT ESC "0m"
#define COLORED(COLOR, TEXT) COLOR TEXT DEFAULT

uint32_t map_mod_index(const map_header_t *header, uint32_t i) {
    return i % header->mapping_capacity;
}

void map_print_mapping_state(void *this, FILE *file) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	for ( unsigned int i = 0; i < header->mapping_capacity; i++ ) {
		int mapping = header->mapping_arr[i];

		if (mapping == FLAG_TOMBSTONE) {
			fprintf(file, "󰮢");
		} else if (mapping == FLAG_FREE) {
			fprintf(file, "");
		} else if (mapping < 0) {
			fprintf(file, COLORED(RED, ""));
		} else {
			fprintf(file, "");
		}
	}
	fprintf(file, "\n");
}

static inline unsigned int mapping_cap(unsigned int capacity) {
	return capacity * 2;
}

typedef struct __map_sizes {
	size_t header_bytes;
	size_t mapping_arr_bytes;
} __map_sizes_t;

__map_sizes_t __map_sizes(size_t capacity, size_t item_size) {
	const size_t header_bytes = sizeof(map_header_t) + item_size * capacity;
	map_header_t foo;
	// TODO: this is shit. I am defining that the mapping arr is 2x the size of
	// the items arr in multiple places. Figure this out at some point.
	const size_t mapping_arr_bytes = sizeof(*foo.mapping_arr) * capacity * 2;
	return (__map_sizes_t){
		.header_bytes=header_bytes,
		.mapping_arr_bytes=mapping_arr_bytes,
	};
}

void *map_create_func(
	map_create_func_args_t args
) {
	__map_sizes_t sizes = __map_sizes(
		args.initial_capacity,
		args.item_size
	);
	map_header_t *header = allocator_alloc_func(
		args.allocator,
		sizes.header_bytes,
		args.file,
		args.line
	);

	header->mapping_arr = allocator_alloc_func(
		args.allocator,
		sizes.mapping_arr_bytes,
		args.file,
		args.line
	);
	header->mapping_capacity = args.initial_capacity * 2;

	header->n_items = 0;
	header->capacity = args.initial_capacity;
	header->item_size = args.item_size;
	header->key_size = args.key_size;
	header->allocator = args.allocator;
	header->hash = args.hash;
	header->equals = args.equals;

	for ( unsigned int i = 0; i < header->mapping_capacity; i++ ) {
		header->mapping_arr[i] = FLAG_FREE;
	}

	return header->bytes;
}

static const uint8_t *__cig_key_ptr_from_pair_ptr(const map_header_t *this, const uint8_t *pair) {
	return (const uint8_t *) (&pair[this->key_offset]);
}

bool map_key_equals(map_header_t *header, const uint8_t *key1_bytes, const uint8_t *key2_bytes) {
	if (header->equals != NULL) {
		return header->equals(key1_bytes, key2_bytes);
	}
	for ( unsigned int i = 0; i < header->key_size; i++ ) {
		if (key1_bytes[i] != key2_bytes[i]) {
			return false;
		}
	}
	return true;
}

unsigned int map_place(void *this, index_pair_t idx) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	if (idx.has_old) {
		header->mapping_arr[idx.new_i_into_mapping] = header->mapping_arr[idx.old_i_into_mapping];
		header->mapping_arr[idx.old_i_into_mapping] = FLAG_TOMBSTONE;
		for ( long i = idx.old_i_into_mapping; i != idx.new_i_into_mapping; map_mod_index(header, i-1) ) {
			if (header->mapping_arr[i] == FLAG_TOMBSTONE) {
				header->mapping_arr[i] = FLAG_FREE;
			} else {
				break;
			}
		}
	} else {
		header->mapping_arr[idx.new_i_into_mapping] = header->n_items;
		header->n_items++;
	}

	return header->mapping_arr[idx.new_i_into_mapping];
}

uint32_t map_hash_key(const map_header_t *header, const uint8_t *key) {
	if (header->hash != NULL) {
		return map_mod_index(header, header->hash(key));
	} else {
		return map_mod_index(header, fnv_1a(key, header->key_size));
	}
}

typedef struct pair {
	const uint8_t *pair;
	const size_t pair_len;
	const uint8_t *key;
	const size_t key_len;
} pair_t;

static pair_t __map_get_pair_at(const map_header_t *this, size_t index) {
	const uint8_t *pair = (uint8_t*)(&this->bytes[index * this->item_size]);
	const size_t pair_len = this->item_size;
	const uint8_t *key = __cig_key_ptr_from_pair_ptr(this, pair);
	const size_t key_len = this->key_size;
	return (pair_t) {
		.pair=pair,
		.pair_len=pair_len,
		.key=key,
		.key_len=key_len,
	};
}

index_pair_t map_pair_hash(void *this, const uint8_t *pair) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	const uint8_t *new_key = __cig_key_ptr_from_pair_ptr(header, pair);

	// find the initial index based on hash
	uint32_t i_into_mapping = map_hash_key(header, new_key);

	int existing_i_into_bytes = header->mapping_arr[i_into_mapping];
	if (existing_i_into_bytes == FLAG_FREE) {
		return (index_pair_t) {
			.has_old=false,
			.new_i_into_mapping=i_into_mapping,
		};
	}
	// TODO THIS!!
}

// TODO: search all occurances of **this. The whole point is to reassign to the
// variable!

void map_assure_growable_by_1(void **this, const char *file, int line) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, *this);
	unsigned int new_n_items = header->n_items + 1;
	if (new_n_items <= header->capacity) {
		return;
	}

	allocator_t allocator = header->allocator;
	unsigned int new_capacity = 1;
	if (header->capacity > 0) {
		new_capacity = header->capacity * 2;
	}
	__map_sizes_t new_size = __map_sizes(
		new_capacity,
		header->item_size
	);

	header = allocator_resize_func(allocator, header, new_size.header_bytes, file, line);
	header->mapping_arr = (int*) allocator_resize_func(allocator, header->mapping_arr, new_size.mapping_arr_bytes, file, line);
	header->mapping_capacity = new_capacity * 2;
	header->capacity=new_capacity;
	
	// first we have to clear the entire mapping array
	for ( unsigned int i = 0; i < header->mapping_capacity; i++ ) {
		header->mapping_arr[i] = FLAG_FREE;
	}

	// Then we have to re-hash and map all the items in the existing map.
	// The reason for re-hashing is that all hashes are moduloed by the
	// size of the mapping array, so the resulting hash will likely change.
	for ( unsigned int i = 0; i < header->n_items; i++ ) {
		void *pair = &(((uint8_t *)(*this))[i*header->item_size]);
		// This is okay, because we absolutely know that the keys don't already
		// exist. (assuming all the other code for map is doing it's job)

		// TODO: when the header is retrieved within the map_pair_hash function the address is completely wrong. Scrambles the parameters and fucks everyghing!!!
		// TODO!!!
		// TODO!!!
		// TODO!!!
		// TODO!!!
		unsigned int mapping_index = map_pair_hash(*this, pair).new_i_into_mapping;
		header->mapping_arr[mapping_index] = i;
	}

	// TODO!!!! FUCKING reasign to *this IDIOT!
}

uint32_t fnv_1a(const uint8_t *bytes, const unsigned int size) {
	// Yanked from: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	const uint32_t PRIME = 0x01000193;
	const uint32_t OFFSET = 0x811c9dc5;
	uint32_t hash = OFFSET;
	for (unsigned int i = 0; i < size; i++) {
		uint32_t byte = (uint32_t)bytes[i];
		hash = hash ^ byte;
		hash = hash * PRIME;
	}
	return hash;
}

int map_len_func(void *this) {
	if (this == NULL) {
		return 0;
	}
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	return header->n_items;
}

int map_cap_func(void *this) {
	if (this == NULL) {
		return 0;
	}
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	return header->capacity;
}

#undef FLAG_FREE
#undef FLAG_TOMBSTONE
#undef ESC
#undef RED
#undef DEFAULT
#undef COLORED

