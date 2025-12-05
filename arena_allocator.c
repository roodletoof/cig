#include "cig.h"
#include <stdbool.h>
#include <assert.h>

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
	uint8_t *old_ptr_b = (uint8_t *) old_ptr;

	bool is_old_ptr_in_data =
		this->data <= old_ptr_b &&
		old_ptr_b < &this->data[this->capacity];

	if (is_old_ptr_in_data) {
		assert(
			old_ptr_b < &this->data[this->size] &&
			"You cannot resize a pointer you got from before this allocator was reset.\n"
			"You should not store pointer gotten from this allocator across resets.\n"
		);
	}

	if (!is_old_ptr_in_data) {
		// just a guess for how many extra bytes were allocated.
		this->bytes_outside_data += bytes / 2;
		return allocator_resize(this->allocator, old_ptr, bytes);
	}

	uint8_t *new_ptr_b = arena_alloc(this, bytes);
	if (new_ptr_b == NULL) {
		return NULL;
	}

	// sizeof not necessary but ill do it anyways.
	size_t old_index = ((size_t)old_ptr - (size_t)this->data) / sizeof(*this->data);
	size_t bytes_til_buffer_end = old_index + this->size;
	size_t max_bytes_to_copy = bytes < bytes_til_buffer_end
		? bytes
		: bytes_til_buffer_end;
	for (size_t i = old_index; i < old_index + max_bytes_to_copy; i++) {
		new_ptr_b[i] = this->data[i];
	}
	return new_ptr_b;
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
