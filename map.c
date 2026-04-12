#include "cig.h"
#include <assert.h>

#define FLAG_FREE       (-1)
#define FLAG_GRAVESTONE (-2)

static inline unsigned int mapping_cap(unsigned int capacity) {
	return capacity * 2;
}

typedef struct internal_map_sizes {
	int bytes;
	int real_cap_of_items_arr;
} internal_map_sizes_t;

internal_map_sizes_t internal_map_sizes(unsigned int capacity, unsigned int item_size) {
	const size_t cap_of_index_arr =
		sizeof(int) * mapping_cap(capacity);
	const size_t INC = ALIGN_OF(int);
	const size_t SIZE = item_size * capacity;
	const size_t cap_of_items_arr = (SIZE + INC - 1) / INC;
	const size_t bytes =
		sizeof(map_header_t) + cap_of_items_arr + cap_of_index_arr;
	return (internal_map_sizes_t){
		.bytes=bytes,
		.real_cap_of_items_arr=cap_of_items_arr,
	};
}

void *map_create_func(
	map_create_func_args_t args
) {
	internal_map_sizes_t sizes = internal_map_sizes(
		args.initial_capacity,
		args.item_size
	);
	map_header_t *header =
		allocator_alloc_func(
			args.allocator,
			sizes.bytes,
			args.file,
			args.line
		);

	header->n_items = 0;
	header->capacity = args.initial_capacity;
	header->item_size = args.item_size;
	header->key_size = args.key_size;
	header->allocator = args.allocator;
	header->hash = args.hash;
	header->equals = args.equals;
	header->mapping_arr = (int*) &header->bytes[sizes.real_cap_of_items_arr];
	header->mapping_capacity = mapping_cap(args.initial_capacity);

	for ( int i = 0; i < header->mapping_capacity; i++ ) {
		header->mapping_arr[i] = FLAG_FREE;
	}

	return header->bytes;
}

// Checks that the map can handle one more item. The maps capacity grows if not,
// usually by doubling the capacity.
void map_assure_growable_by_1(void **this, const char *file, int line) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, *this);
	int new_n_items = header->n_items + 1;
	if (new_n_items <= header->mapping_capacity) {
		return;
	}

	allocator_t allocator = header->allocator;
	unsigned int new_capacity = 1;
	if (header->capacity > 0) {
		new_capacity = header->capacity * 2;
	}
	internal_map_sizes_t new_size = internal_map_sizes(
		new_capacity,
		header->item_size
	);
	header = allocator_resize_func(allocator, header, new_size.bytes, file, line);
	header->mapping_arr = (int*) &header->bytes[new_size.real_cap_of_items_arr];
	header->mapping_capacity = mapping_cap(new_capacity);
	header->capacity=new_capacity;
	
	// first we have to clear the entire mapping array
	for ( unsigned int i = 0; i < header->mapping_capacity; i++ ) {
		header->mapping_arr[i] = FLAG_FREE;
	}

	// Then we have to re-hash and map all the items in the existing map.
	// The reason for re-hashing is that all hashes are moduloed by the
	// size of the mapping array, so the resulting hash will likely change.
	for ( unsigned int i = 0; i < header->n_items; i++ ) {
		void *pair = &this[i*header->item_size];
		// TODO: make the map_pair_hash more private? would be nice to just
		// pass in the header, and I don't really think it is needed
		// directly by the user of the library.
		unsigned int mapping_index = map_pair_hash(this, pair);
		header->mapping_arr[mapping_index] = i;
	}
}

uint32_t fnv_1a(const uint8_t *bytes, const unsigned int size) {
	// Yanked from: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	const uint32_t PRIME = 0x01000193;
	const uint32_t OFFSET = 0x811c9dc5;
	uint32_t hash = OFFSET;
	for (int i = 0; i < size; i++) {
		uint32_t byte = (uint32_t)bytes[i];
		hash = hash ^ byte;
		hash = hash * PRIME;
	}
	return hash;
}

static void *internal_cig_key_ptr_from_pair_ptr(void *this, void *pair) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	return &pair[header->key_offset];
}

// TODO: just make these internal probably
bool map_key_equals(void *this, const void *key1, const void *key2) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	if (header->equals != NULL) {
		return header->equals(key1, key2);
	}
	const char *key1_bytes = key1;
	const char *key2_bytes = key2;
	for ( int i = 0; i < header->key_size; i++ ) {
		if (key1_bytes[i] != key2_bytes[i]) {
			return false;
		}
	}
	return true;
}

unsigned int map_pair_hash(void *this, void *pair) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	void *key = internal_cig_key_ptr_from_pair_ptr(this, pair);

	// find the initial index based on hash
	uint32_t index;
	if (header->hash != NULL) {
		index = header->hash(key) % header->mapping_capacity;
	} else {
		index = fnv_1a(key, header->key_size) % header->mapping_capacity;
	}

	int existing_index = header->mapping_arr[index];
	while (!(existing_index == FLAG_FREE || existing_index == FLAG_GRAVESTONE)) {
		index = fnv_1a(
			(const uint8_t *) &index,
			sizeof(index)
		) % header->mapping_capacity;
		int existing_index = header->mapping_arr[index];
	}
	return (unsigned int) index;
}

int map_len(void *this) {
	if (this == NULL) {
		return 0;
	}
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	return header->n_items;
}

int map_cap(void *this) {
	if (this == NULL) {
		return 0;
	}
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	return header->capacity;
}

#undef FLAG_FREE
#undef FLAG_GRAVESTONE

