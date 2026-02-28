#include "cig.h"
#include <assert.h>

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
	allocator_t allocator = header->allocator;
	const int new_size = 0; // TODO!!!
	header = allocator_resize_func(allocator, header, new_size, file, line);
	// TODO!!!
	assert(false && "TODO");

	return header->bytes;
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
