#include "cig.h"

static void *forever_alloc(void *this, size_t bytes) {
	(void)this;
	return malloc(bytes);
}
static void *forever_resize(void *this, void *old_ptr, size_t bytes) {
	(void)this;
	return realloc(old_ptr, bytes);
}
static void forever_no_op(void *this) {
	(void)this;
}

static const allocator_vtbl_t forever_vtbl = {
	.alloc = forever_alloc,
	.resize = forever_resize,
	.reset = forever_no_op,
};

allocator_t forever_allocator() {
	return (allocator_t) {
		.this=NULL,
		.vtbl=&forever_vtbl,
	};
}

