#include "cig.h"

static inline int mapping_cap(int capacity) {
	return capacity * 2;
}

void *map_create_func(
	map_create_func_args_t args
) {

	const size_t cap_of_index_arr =
		sizeof(int) * mapping_cap(args.initial_capacity);
	const size_t INC = ALIGN_OF(int);
	const size_t SIZE = args.itemsize * args.initial_capacity;
	const size_t cap_of_items_arr = (SIZE + INC - 1) / INC;
	const size_t bytes =
		sizeof(map_header_t) + cap_of_items_arr + cap_of_index_arr;

	map_header_t *header =
		allocator_alloc_func(
			args.allocator,
			bytes,
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
	header->mapping_arr = (int*) &header->bytes[cap_of_items_arr];
	header->mapping_capacity = mapping_cap(args.initial_capacity);

	for ( int i = 0; i < header->mapping_capacity; i++ ) {
		header->mapping_arr[i] = -1;
	}

	return header->bytes;
}

void map_grow(void *this) {
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
