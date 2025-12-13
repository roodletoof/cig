#include "cig.h"
#include <stdio.h>

void *dyn_array_create_func(dyn_array_create_func_args_t args) {
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

void *dyn_array_grow_func(void *this, size_t n_new_items, const char *file, int line) {
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

bool dyn_array_contains_cmp_func(void *this, uint8_t *value, dyn_array_eq_fn eq) {
	dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
	size_t itemsize = header->itemsize;
	
	for (size_t i = 0; i < header->n_items; i++) {
		void *element = &header->bytes[i * itemsize];
		if (eq(element, value)) {
			return true;
		}
	}
	return false;
}

void dyn_array_insert_sorted_func(void **this_ptr, const void *value, dyn_array_cmp_fn cmp, const char *file, int line) {
	void *this = *this_ptr;
	dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
	
	// First append the value
	this = dyn_array_grow_func(this, 1, file, line);
	*this_ptr = this;
	header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, this);
	
	size_t itemsize = header->itemsize;
	size_t n_items = header->n_items;
	
	// Copy value to the end
	memcpy(&header->bytes[(n_items - 1) * itemsize], value, itemsize);
	
	// Bubble it into the correct position from the back
	size_t pos = n_items - 1;
	while (pos > 0) {
		void *a = &header->bytes[(pos - 1) * itemsize];
		void *b = &header->bytes[pos * itemsize];
		
		if (cmp(a, b) <= 0) {
			break; // Already in correct position
		}
		
		// Swap using a temporary buffer
		uint8_t temp[itemsize];
		memcpy(temp, a, itemsize);
		memcpy(a, b, itemsize);
		memcpy(b, temp, itemsize);
		
		pos--;
	}
}
