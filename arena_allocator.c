#include "cig.h"
#include <stdbool.h>

static void arena_reset(arena_allocator_t *this) {
	if (0 < this->bytes_outside_data) {
		this->capacity += this->bytes_outside_data;
		this->bytes_outside_data = 0;
		allocator_reset(this->allocator);
		this->data = NULL;
	}
	if (this->data == NULL) {
		this->data = allocator_alloc(this->allocator, this->capacity);
	}
	this->size = 0;
}

static void *arena_alloc(arena_allocator_t *this, size_t bytes) {
	// Assume that the this->size index already points to an address that is
	// divisible by MAX_ALIGN. The following is a magic formula from the GPT
	// gods that rounds up bytes to a multiple of MAX_ALIGN.
	// TODO: write tests that assert that this formula is always correct.
	bytes = (bytes + MAX_ALIGN - 1) / MAX_ALIGN * MAX_ALIGN;
	size_t remaining = this->capacity - this->size;
	if (remaining < bytes) {
		this->bytes_outside_data += bytes;
		return allocator_alloc(this->allocator, bytes);
	}
	void *ptr = &this->data[this->size];
	this->size += bytes;
	return ptr;
}

static void *arena_resize(arena_allocator_t *this, void *old_ptr, size_t bytes) {
	bool is_old_ptr_in_data = this->data <= old_ptr && old_ptr < &this->data[this->size]
	// TODO: implement this
}

// typedef struct arena_allocator {
// 	allocator_t allocator;
// 	size_t size;
// 	size_t capacity;
// 	size_t bytes_outside_data;
// 	uint8_t *data;
// } arena_allocator_t;


allocator_t allocator_from_arena(arena_allocator_t *this) {
}
