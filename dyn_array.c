#include "cig.h"
#include <stdio.h>

typedef struct dyn_array_create_non_crashing_func_args {
	allocator_t allocator;
	size_t itemsize;
	size_t initial_capacity;
	const char *file;
	size_t line;
} dyn_array_create_non_crashing_func_args_t;


static inline void *todo_remove_create_func(dyn_array_create_non_crashing_func_args_t args);
void *dyn_array_create_func(dyn_array_create_func_args_t args) {
	void *bytes = todo_remove_create_func(
		(dyn_array_create_non_crashing_func_args_t) {
			.allocator=args.allocator,
			.itemsize=args.itemsize,
			.initial_capacity=args.initial_capacity,
			.file=args.file,
			.line=args.line
		}
	);
	return bytes;
}

static inline void *todo_remove_grow_func(void *this, size_t n_new_items, const char *file, int line);
void *dyn_array_grow_func(void *this, size_t n_new_items, const char *file, int line) {
	void *bytes = todo_remove_grow_func(this, n_new_items, file, line);
	return bytes;
}

// These functions are from when there were non-crashing versions of these
// allocating functions.
static inline void *todo_remove_create_func(dyn_array_create_non_crashing_func_args_t args) {
	dyn_array_header_t *header = allocator_alloc_func(
		args.allocator,
		sizeof(dyn_array_header_t) + args.itemsize * args.initial_capacity,
		args.file,
		args.line
	);
	header->n_items = 0;
	header->capacity = args.initial_capacity;
	header->itemsize = args.itemsize;
	header->allocator = args.allocator;
	return &header->bytes;
}

static inline void *todo_remove_grow_func(void *this, size_t n_new_items, const char *file, int line) {
	dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
	size_t new_size = header->n_items + n_new_items;

	if (header->capacity < new_size) {
		size_t cap_times_2 = header->capacity * 2;
		size_t new_capacity =
			cap_times_2 < new_size
			? new_size
			: cap_times_2;
		header = allocator_resize_func(
			header->allocator,
			header,
			sizeof(dyn_array_header_t) + header->itemsize * new_capacity,
			file,
			line
		);
		header->capacity = new_capacity;
	}
	header->n_items = new_size;
	return &header->bytes;
}

void dyn_array_shrink_func(void *this, size_t n_items_to_remove, const char *file, int line) {
	if (arr_len(this) < n_items_to_remove) {
		fprintf(
			stderr,
			"%s:%d: tried to remove more items than contained in dynamic array.\n",
			file,
			line
		);
		exit(1);
	}
	dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
	header->n_items -= n_items_to_remove;
}

size_t arr_len(void *this) {
	if (this == NULL) { return 0; }
	dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
	return header->n_items;

}

size_t arr_cap(void *this) {
	if (this == NULL) { return 0; }
	dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
	return header->capacity;
}

bool dyn_array_contains_func(void *this, uint8_t *value) {
	dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
	for (size_t i = 0; i < header->n_items; i++) {
		bool is_equal = true;
		for (size_t off = 0; off < header->itemsize; off++) {
			if (header->bytes[i*header->itemsize+off] != value[off]) {
				is_equal = false;
				break;
			}
		}
		if (is_equal) {
			return true;
		}
	}
	return false;
}
