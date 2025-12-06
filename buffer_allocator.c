#include "cig.h"

static void *buffer_allocator_alloc(buffer_allocator_t *this, size_t bytes) {
	size_t address_of_new_data = (size_t) &this->data[this->size];
	size_t offset = address_of_new_data % MAX_ALIGN;
	size_t new_size = this->size + offset + bytes;
	if (new_size > this->capacity) {
		return NULL;
	}
	this->size += offset;
	void *ptr = &this->data[this->size];
	this->size = new_size;
	return ptr;
}

static void *buffer_allocator_resize(buffer_allocator_t *this, void *old_ptr, size_t bytes) {
	void *new_ptr = buffer_allocator_alloc(this, bytes);
	if (new_ptr == NULL) {
		return NULL;
	}
	for (size_t i = 0; i < bytes; i++) {
		unsigned char *new_ptr_b = (unsigned char *)new_ptr;
		unsigned char *old_ptr_b = (unsigned char *)old_ptr;
		new_ptr_b[i] = old_ptr_b[i];
	}
	return new_ptr;
}

static void buffer_allocator_reset(buffer_allocator_t *this) {
	this->size = 0;
}

static void *buffer_alloc_impl(void *this, size_t bytes) {
	return buffer_allocator_alloc((buffer_allocator_t *)this, bytes);
}

static void *buffer_resize_impl(void *this, void *old_ptr, size_t bytes) {
	return buffer_allocator_resize((buffer_allocator_t *)this, old_ptr, bytes);
}

static void buffer_reset_impl(void *this) {
	buffer_allocator_reset((buffer_allocator_t *)this);
}

static const allocator_vtbl_t buffer_vtbl = {
	.alloc = buffer_alloc_impl,
	.resize = buffer_resize_impl,
	.reset = buffer_reset_impl,
};

allocator_t allocator_from_buffer(buffer_allocator_t *this) {
	return (allocator_t) {
		.this=this,
		.vtbl=&buffer_vtbl,
	};
}
