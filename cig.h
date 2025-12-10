#ifndef CIG_H
#define CIG_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef union any_align { char c; int i; long l; long long ll; float f; double d; void *p; long double ld; } any_align_t;
#define MAX_ALIGN ((size_t) sizeof(any_align_t))
#define KB (1024)
#define MB (KB * KB)
#define GB (KB * KB * KB)
#define OFFSET(STRUCT, FIELD) ((size_t) (&((STRUCT*) NULL)->FIELD))
#define PTR_FROM_FIELD_PTR(STRUCT, FIELD, PTR) ((STRUCT *) (((char *) PTR) - OFFSET(STRUCT, FIELD)))

// Contains all operations an allocator can do. Similar interface to sdtlibs
// malloc, realloc and free.
typedef struct allocator_vtbl {
	void *(*alloc)(void *this, size_t bytes);
	void *(*resize)(void *this, void *old_ptr, size_t bytes);
	void (*reset)(void *this);
} allocator_vtbl_t;

// An instance of an allocator.
typedef struct allocator {
	// pointer to the behind-the-scenes data that an allocator may store.
	void *this;
	// pointer to the method implementations of an allocator.
	const allocator_vtbl_t *vtbl;
} allocator_t;

void *allocator_alloc_func(allocator_t this, size_t bytes, const char *file, int line);
void *allocator_resize_func(allocator_t this, void *old_ptr, size_t bytes, const char *file, int line);
void allocator_reset(allocator_t this);
#define allocator_alloc(this, bytes) allocator_alloc_func(this, bytes, __FILE__, __LINE__)
#define allocator_resize(this, old_ptr, bytes) allocator_resize_func(this, old_ptr, bytes, __FILE__, __LINE__)

// forever_allocator ///////////////////////////////////////////////////////////
allocator_t forever_allocator(void);

// buffer_allocator ////////////////////////////////////////////////////////////
typedef struct buffer_allocator {
	size_t size, capacity;
	uint8_t *data;
} buffer_allocator_t;

// TODO: name stack value or something to signal what is happening here!
#define buffer_allocator_value(CAPACITY) \
  ((buffer_allocator_t){ \
	  .size = 0, .capacity = CAPACITY, .data = (uint8_t[CAPACITY]){0}})

allocator_t allocator_from_buffer(buffer_allocator_t *this);

// borrow_allocator ////////////////////////////////////////////////////////////
typedef struct linked_allocation_node {
	struct linked_allocation_node *next;
	struct linked_allocation_node *prev;
	any_align_t data[];
} linked_allocation_node_t;

typedef struct borrow_allocator {
	linked_allocation_node_t *head;
} borrow_allocator_t;

#define borrow_allocator_value() ((borrow_allocator_t){.head=NULL})
allocator_t allocator_from_borrow(borrow_allocator_t *this);
#define borrow_allocator_create() allocator_from_borrow(&borrow_allocator_value())

// Some text that can be used as an identifier (no, not by you), so that I can
// use a variable that won't collide with yours inside macros.
#define UNIQUE __macro_internal_34bba35b8b9b20a75f9881e3795630e25d36e620d9c9741e2e9141ba82ec6ef6__

// Can be used to mimic rust borrow semantics. with_borrow(foos_allocator) {
// foo(foos_allocator); }. If a function should return ownership assume that
// my_allocator was passed in by an outer function void *bar;
// with_borrow(foos_allocator) {bar = foo(foos_allocator, my_allocator); } Using
// the return a keyword in the statement following this macro will cause a
// guaranteed memory leak.
#define with_borrow(NAME)\
	for (allocator_t NAME = borrow_allocator_create(); NAME.this != NULL; NAME.this = (allocator_reset(NAME), NULL)) \
	for (int UNIQUE = 0; UNIQUE < 1; UNIQUE++) \

// arena allocator /////////////////////////////////////////////////////////////

typedef struct arena_allocator {
	allocator_t allocator;
	size_t size;
	size_t capacity;
	size_t bytes_outside_data;
	uint8_t *data;
} arena_allocator_t;

#define arena_allocator_value(BACKING_ALLOCATOR, INITIAL_CAPACITY) ((arena_allocator_t) { \
	.allocator=BACKING_ALLOCATOR, \
	.size=0, \
	.capacity=INITIAL_CAPACITY, \
	.data=NULL \
})

allocator_t allocator_from_arena(arena_allocator_t *this);
#define arena_allocator_create(BACKING_ALLOCATOR, INITIAL_CAPACITY) \
	allocator_from_arena(&arena_allocator_value(BACKING_ALLOCATOR, INITIAL_CAPACITY))

// dynamic arrays //////////////////////////////////////////////////////////////

typedef struct dyn_array_header {
	size_t n_items, capacity, itemsize;
	allocator_t allocator;
	union {
		// allocates a little more memory than needed, but who cares?
		uint8_t bytes[1];
		any_align_t _[1];
	};
} dyn_array_header_t;

typedef struct dyn_array_create_func_args {
	allocator_t allocator;
	size_t itemsize;
	size_t initial_capacity;
	const char *file;
	int line;
} dyn_array_create_func_args_t;
void *dyn_array_create_func(dyn_array_create_func_args_t args);

#define make_arr(ALLOCATOR, TYPE, ...) ((TYPE *)dyn_array_create_func((dyn_array_create_func_args_t){ \
	.allocator = ALLOCATOR, \
	.itemsize = sizeof(TYPE), \
	.file = __FILE__, \
	.line = __LINE__, \
	__VA_ARGS__ \
}))

// Always reassign the array. if multiple variables reference the same growing
// array, then you should be using pointer pointers.
void *dyn_array_grow_func(void *this, size_t n_new_items, const char *file, int line);
void dyn_array_shrink_func(void *this, size_t n_items_to_remove, const char *file, int line);

#define arr_grow(THIS, N_NEW_ITEMS) \
  dyn_array_grow_func(THIS, N_NEW_ITEMS, __FILE__, __LINE__)

#define arr_shrink(THIS, N_ITEMS_TO_REMOVE) \
	dyn_array_shrink_func(THIS, N_ITEMS_TO_REMOVE, __FILE__, __LINE__)

#define arr_reset(THIS)\
  do {\
	dyn_array_header_t *header = PTR_FROM_FIELD_PTR(dyn_array_header_t, bytes, THIS);\
	header->n_items = 0;\
  } while (0)

size_t arr_len(void *this);
size_t arr_cap(void *this);

#define arr_append(THIS, ...) do { \
	THIS = arr_grow(THIS, 1); \
	THIS[arr_len(THIS)-1] = __VA_ARGS__; \
} while(0)


#define arr_pop(THIS) (arr_shrink(THIS, 1), THIS[arr_len(THIS)])


/* Compile-time assert that works in expression contexts */
#define STATIC_ASSERT(expr) ((void)sizeof(char[(expr) ? 1 : -1]))


bool dyn_array_contains_func(void *this, uint8_t *value);

/*
    arr_contains(THIS, TYPE, VALUE)

    - SIZE CHECK: compile-time check that sizeof(*THIS) == sizeof(TYPE)
    - TYPE is the type of the value to search for
    - VALUE is stored in a compound literal of TYPE
    - Pure C99, no extensions needed
    - Callers with typeof support can make wrapper macros
*/
#define arr_contains(THIS, ...)\
	(\
		STATIC_ASSERT(sizeof(*(THIS)) == sizeof(*(__VA_ARGS__))),\
		dyn_array_contains_func((THIS), (uint8_t*)(__VA_ARGS__))\
	)

// CLI /////////////////////////////////////////////////////////////////////////

#define CLI_UNIQUE1 __macro_internal_34bba35b8b9b20a75f9881e3795630e25d36e620d9c9741e2e9141ba82ec6ef7__
#define CLI_UNIQUE2 __macro_internal_34bba35b8b9b20a75f9881e3795630e25d36e620d9c9741e2e9141ba82ec6ef8__

typedef struct args {
	int count;
	const char **values;
} args_t;

args_t cli_make_args(int argc, const char **argv); // no crash
bool cli_command(args_t *args, const char *command_name); // no crash
bool cli_bool(args_t args, const char *flag_name); // no crash
bool cli_opt_str_func(args_t args, const char *flag_name, const char **output, const char *file, int line);
const char *cli_req_str_func(args_t args, const char *flag_name, const char *file, int line);
bool cli_opt_int_func(args_t args, const char *flag_name, int *output, const char *file, int line);
int cli_req_int_func(args_t args, const char *flag_name, const char *file, int line);

#define cli_opt_str(ARGS, FLAG_NAME, OUTPUT) cli_opt_str_func(ARGS, FLAG_NAME, OUTPUT, __FILE__, __LINE__)
#define cli_req_str(ARGS, FLAG_NAME) cli_req_str_func(ARGS, FLAG_NAME, __FILE__, __LINE__)
#define cli_opt_int(ARGS, FLAG_NAME, OUTPUT) cli_opt_int_func(ARGS, FLAG_NAME, OUTPUT, __FILE__, __LINE__)
#define cli_req_int(ARGS, FLAG_NAME) cli_req_int_func(ARGS, FLAG_NAME, __FILE__, __LINE__)

#define with_opt_str(ARGS, FLAG_NAME, VAR) \
	for (const char *VAR = NULL; VAR == NULL;) \
	for (bool CLI_UNIQUE1 = true; CLI_UNIQUE1;) \
	for ( \
		bool CLI_UNIQUE2 = (CLI_UNIQUE1 = cli_opt_str(ARGS, FLAG_NAME, &VAR), true); \
		CLI_UNIQUE2; \
		CLI_UNIQUE2 = (VAR = ((const char *) 1), CLI_UNIQUE1 = false, false) \
	) \
	if (CLI_UNIQUE1)

#define with_opt_int(ARGS, FLAG_NAME, VAR) \
	for (int VAR = 0; VAR == 0;) \
	for (bool CLI_UNIQUE1 = true; CLI_UNIQUE1;) \
	for ( \
		bool CLI_UNIQUE2 = (CLI_UNIQUE1 = cli_opt_int(ARGS, FLAG_NAME, &VAR), true); \
		CLI_UNIQUE2; \
		CLI_UNIQUE2 = (VAR = 1, CLI_UNIQUE1 = false, false) \
	) \
	if (CLI_UNIQUE1)

// scanner /////////////////////////////////////////////////////////////////////

typedef struct error_node {
	char *msg;
	struct error_node *next;
} error_node_t;

typedef union scan_value {
	int digit;
	uint8_t u8;
	int8_t i8;
	uint16_t u16;
	int16_t i16;
	uint32_t u32;
	int32_t i32;
	uint64_t u64;
	int64_t i64;
	float f32;
	double f64;
	char *identifier;
	char *string_literal;
} scan_value_t;

typedef struct scanner {
	const char *name; // name of the buffer
	const char *start; // pointer to the full buffer
	const char *cur; // current pointer
	scan_value_t value;
	error_node_t *errors; // singly linked list of error strings
	allocator_t allocator;
} scanner_t;

scanner_t make_scanner(const char *name, const char *buffer, allocator_t allocator);
void scanner_recover(scanner_t *s);
void scanner_error(scanner_t *s, const char *message);
void scanner_error_and_recover(scanner_t *s, const char *message);
bool scan_eof(scanner_t *s);
bool scan_literal(scanner_t *s, const char *lit);
int scan_repeat_literal(scanner_t *s, const char *lit);
bool scan_whitespace(scanner_t *s);
bool scan_digit(scanner_t *s);
bool scan_i64(scanner_t *s);
bool scan_i32(scanner_t *s);
bool scan_i16(scanner_t *s);
bool scan_i8(scanner_t *s);
bool scan_u64(scanner_t *s);
bool scan_u32(scanner_t *s);
bool scan_u16(scanner_t *s);
bool scan_u8(scanner_t *s);
bool scan_f64(scanner_t *s);
bool scan_f32(scanner_t *s);
// Scan as much of an identifier as possible, you are responsible to scan for
// valid characters after the identifier is scanned.
bool scan_identifier(scanner_t *s);
bool scan_string_literal(scanner_t *s);
bool scanner_print_errors(scanner_t *s, FILE *fp);

// string builder //////////////////////////////////////////////////////////////
typedef struct string_builder_node {
	struct string_builder_node *next;
	size_t length;
	const char *string;
} string_builder_node_t;

typedef struct string_builder {
	string_builder_node_t *head;
	string_builder_node_t *tail;
	allocator_t allocator;
} string_builder_t;

string_builder_t sb_create(allocator_t builder_allocator);
// assumes the provided string will live until sb_build is called.
void sb_add_string(string_builder_t *this, const char *string);
void sb_add_i64(string_builder_t *this, int64_t i64);
void sb_add_i32(string_builder_t *this, int32_t i32);
void sb_add_i16(string_builder_t *this, int16_t i16);
void sb_add_i8(string_builder_t *this, int8_t i8);
void sb_add_u64(string_builder_t *this, uint64_t u64);
void sb_add_u32(string_builder_t *this, uint32_t u32);
void sb_add_u16(string_builder_t *this, uint16_t u16);
void sb_add_u8(string_builder_t *this, uint8_t u8);
void sb_add_substring(string_builder_t *this, const char *string, size_t substring_length);
const char *sb_build(string_builder_t *this, allocator_t output_allocator);

#ifdef CIG_IMPL

void *allocator_alloc_func(allocator_t this, size_t bytes, const char *file, int line) {
	void *ptr = this.vtbl->alloc(this.this, bytes);
	if (ptr == NULL) {
		fprintf(stderr, "%s:%d: alloc returned NULL\n", file, line);
		exit(1);
	}
	return ptr;
}
void *allocator_resize_func(allocator_t this, void *old_ptr, size_t bytes, const char *file, int line) {
	void *ptr = this.vtbl->resize(this.this, old_ptr, bytes);
	if (ptr == NULL) {
		fprintf(stderr, "%s:%d: alloc returned NULL\n", file, line);
		exit(1);
	}
	return ptr;
}
void allocator_reset(allocator_t this) {
	this.vtbl->reset(this.this);
}

#include "forever_allocator.c"
#include "buffer_allocator.c"
#include "borrow_allocator.c"
#include "arena_allocator.c"
#include "dyn_array.c"
#include "cli.c"
#include "scanner.c"
#include "string_builder.c"

#endif // CIG_IMPL
#endif // CIG_H
