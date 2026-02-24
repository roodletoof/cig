#include "cig.h"

void *hashmap_create_func(
	hashmap_create_func_args_t args
) {
	size_t size_of_index_arr = sizeof(int) * args.initial_capacity * 2;
	size_t INC = sizeof(any_align_t);
	size_t SIZE = args.itemsize * args.initial_capacity;
	size_t size_of_items_arr = (SIZE + INC - 1) / INC;
	size_t bytes = sizeof(hashmap_header_t) + size_of_items_arr + size_of_index_arr;

	hashmap_header_t *header = allocator_alloc_func(
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

	return header->bytes;
}
