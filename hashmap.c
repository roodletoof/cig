#include "cig.h"

static inline int mapping_cap(int capacity) {
	return capacity * 2;
}

void *hashmap_create_func(
	hashmap_create_func_args_t args
) {

  size_t size_of_index_arr = sizeof(int) * mapping_cap(args.initial_capacity);
  size_t INC = ALIGN_OF(int);
  size_t SIZE = args.itemsize * args.initial_capacity;
  size_t size_of_items_arr = (SIZE + INC - 1) / INC;
  size_t bytes =
      sizeof(hashmap_header_t) + size_of_items_arr + size_of_index_arr;

  hashmap_header_t *header =
      allocator_alloc_func(args.allocator, bytes, args.file, args.line);

  header->n_items = 0;
  header->capacity = args.initial_capacity;
  header->itemsize = args.itemsize;
  header->keysize = args.keysize;
  header->allocator = args.allocator;
  header->hash = args.hash;
  header->equals = args.equals;

  return header->bytes;
}

int *hashmap_get_mapping_arr(hashmap_header_t *this) {
	// TODO
}
