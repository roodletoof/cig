#include "cig.h"
#include <assert.h>

#define FREE       (-1)
#define GRAVESTONE (-2)

static inline int mapping_cap(int capacity) {
	return capacity * 2;
}

typedef struct internal_map_sizes {
	int bytes;
	int cap_of_items_arr;
} internal_map_sizes_t;

internal_map_sizes_t internal_map_sizes(int capacity, int itemsize) {
	const size_t cap_of_index_arr =
		sizeof(int) * mapping_cap(capacity);
	const size_t INC = ALIGN_OF(int);
	const size_t SIZE = itemsize * capacity;
	const size_t cap_of_items_arr = (SIZE + INC - 1) / INC;
	const size_t bytes =
		sizeof(map_header_t) + cap_of_items_arr + cap_of_index_arr;
	return (internal_map_sizes_t){
		.bytes=bytes,
		.cap_of_items_arr=cap_of_items_arr,
	};
}

void *map_create_func(
	map_create_func_args_t args
) {
	internal_map_sizes_t sizes = internal_map_sizes(
		args.initial_capacity,
		args.itemsize
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
	header->itemsize = args.itemsize;
	header->keysize = args.keysize;
	header->allocator = args.allocator;
	header->hash = args.hash;
	header->equals = args.equals;
	header->mapping_arr = (int*) &header->bytes[sizes.cap_of_items_arr];
	header->mapping_capacity = mapping_cap(args.initial_capacity);

	for ( int i = 0; i < header->mapping_capacity; i++ ) {
		header->mapping_arr[i] = -1;
	}

	return header->bytes;
}

void *map_grow_func(void *this, const char *file, int line) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	int new_size = header->n_items + 1;

	if (header->capacity < new_size) {
		allocator_t allocator = header->allocator;
		int new_capacity = 1;
		if (header->capacity > 0) {
			new_capacity = header->capacity * 2;
		}
		internal_map_sizes_t new_size = internal_map_sizes(
			new_capacity,
			header->itemsize
		);
		header = allocator_resize_func(allocator, header, new_size.bytes, file, line);
		
		static_assert(0, "Overwrite the mapping arr with -1, then iterate"
		"through the items arr and find the hashes of the values to populate the"
		"mappings");
			
		static_assert(0, "make sure the mapping_arr field is set to the new and"
		"correct pointer!");
			
	}

	header->n_items = new_size;
	return header->bytes;
}

void map_shrink_func(void *this, const char *file, int line) {
	if (map_len(this) <= 0) {
		fprintf(
			stderr,
			"%s:%d: tried to shrink map of size 0.\n",
			file,
			line
		);
		exit(1);
	}
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	header->n_items--;
	static_assert(0, "Hash the key at the removed item to find it in the mapping array, and set it to the gravestone value.");
}

static inline uint32_t fnv_1a(const uint8_t *bytes, const int length) {
	// Yanked from: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	const uint32_t PRIME = 0x01000193;
	const uint32_t OFFSET = 0x811c9dc5;
	uint32_t hash = OFFSET;
	for (int i = 0; i < length; i++) {
		uint32_t byte = (uint32_t)bytes[i];
		hash = hash ^ byte;
		hash = hash * PRIME;
	}
	return hash;
}

unsigned int map_hash(void *this, void *value) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	if (header->hash) {
		return header->hash(value);
	}
	return (unsigned int) fnv_1a(value, header->keysize) % header->mapping_capacity;
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

#undef FREE
#undef GRAVESTONE

