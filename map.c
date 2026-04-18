#include "cig.h"
#include <assert.h>
#include <stdio.h>

#define FLAG_FREE      (-1)
#define FLAG_TOMBSTONE (-2)


// TODO: when setting a key to a gravestone, then while the mapping
// item to the right is FLAG_FREE, you can set this one to FLAG_FREE,
// go one index less, and if that is also a FLAG_GRAVESTONE, you can
// continue the process.

#define ESC "\033["
#define RED ESC "31m"
#define DEFAULT ESC "0m"
#define COLORED(COLOR, TEXT) COLOR TEXT DEFAULT

static int internal_map_mod(int a, int b) {
    int r = a % b;
    if (r < 0) r += (b > 0 ? b : -b);
    return r;
}
#define mod(a, b) internal_map_mod(a, b)

void map_print_mapping_state(void *this, FILE *file) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	for ( unsigned int i = 0; i < header->mapping_capacity; i++ ) {
		int mapping = header->mapping_arr[i];

		if (mapping == FLAG_TOMBSTONE) {
			fprintf(file, "󰮢");
		} else if (mapping == FLAG_FREE) {
			fprintf(file, "");
		} else if (mapping < 0) {
			fprintf(file, COLORED(RED, ""));
		} else {
			fprintf(file, "");
		}
	}
	fprintf(file, "\n");
}

static inline unsigned int mapping_cap(unsigned int capacity) {
	return capacity * 2;
}

typedef struct internal_map_sizes {
	int bytes;
	int real_cap_of_items_arr;
} internal_map_sizes_t;

internal_map_sizes_t internal_map_sizes(unsigned int capacity, unsigned int item_size) {
	const size_t cap_of_index_arr =
		sizeof(int) * mapping_cap(capacity);
	const size_t INC = ALIGN_OF(int);
	const size_t SIZE = item_size * capacity;
	const size_t cap_of_items_arr = (SIZE + INC - 1) / INC;
	const size_t bytes =
		sizeof(map_header_t) + cap_of_items_arr + cap_of_index_arr;
	return (internal_map_sizes_t){
		.bytes=bytes,
		.real_cap_of_items_arr=cap_of_items_arr,
	};
}

void *map_create_func(
	map_create_func_args_t args
) {
	internal_map_sizes_t sizes = internal_map_sizes(
		args.initial_capacity,
		args.item_size
	);
	map_header_t *header =
		allocator_alloc_func(
			args.allocator,
			sizes.bytes,
			args.file,
			args.line
		);

	header->n_items = 0;
	header->capacity = args.initial_capacity;
	header->item_size = args.item_size;
	header->key_size = args.key_size;
	header->allocator = args.allocator;
	header->hash = args.hash;
	header->equals = args.equals;
	header->mapping_arr = (int*) &header->bytes[sizes.real_cap_of_items_arr];
	header->mapping_capacity = mapping_cap(args.initial_capacity);

	for ( unsigned int i = 0; i < header->mapping_capacity; i++ ) {
		header->mapping_arr[i] = FLAG_FREE;
	}

	return header->bytes;
}

static const void *internal_cig_key_ptr_from_pair_ptr(const void *this, const void *pair) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	return (const void *) &((const char *)pair)[header->key_offset];
}

// TODO: just make these internal probably
bool map_key_equals(void *this, const void *key1, const void *key2) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	if (header->equals != NULL) {
		return header->equals(key1, key2);
	}
	const char *key1_bytes = key1;
	const char *key2_bytes = key2;
	for ( unsigned int i = 0; i < header->key_size; i++ ) {
		if (key1_bytes[i] != key2_bytes[i]) {
			return false;
		}
	}
	return true;
}

unsigned int map_place(void *this, index_pair_t idx_pair) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	if (idx_pair.has_old_index) {
		header->mapping_arr[idx_pair.new_index] = header->mapping_arr[idx_pair.old_index];
		header->mapping_arr[idx_pair.old_index] = FLAG_TOMBSTONE;
		for ( long i = idx_pair.old_index; i != idx_pair.new_index; mod(i-1, header->mapping_capacity) ) {
			if (header->mapping_arr[i] == FLAG_TOMBSTONE) {
				header->mapping_arr[i] = FLAG_FREE;
			} else {
				break;
			}
		}
	} else {
		header->mapping_arr[idx_pair.new_index] = header->n_items;
		header->n_items++;
	}

	return header->mapping_arr[idx_pair.new_index];
}

index_pair_t map_pair_hash(void *this, void *pair) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	const void *key = internal_cig_key_ptr_from_pair_ptr(this, pair);

	// find the initial index based on hash
	uint32_t mapping_index;
	if (header->hash != NULL) {
		mapping_index = header->hash(key) % header->mapping_capacity;
	} else {
		mapping_index = fnv_1a(key, header->key_size) % header->mapping_capacity;
	}
	uint32_t tombstone_index;
	bool found_tombstone = false;

	// begin probing
	int existing_value_index = header->mapping_arr[mapping_index];
	while (
			existing_value_index != FLAG_FREE &&
			!map_key_equals(
				this,
				internal_cig_key_ptr_from_pair_ptr(
					this,
					&header->bytes[existing_value_index * header->item_size]
				),
				key
			)
		) {


		const bool is_tombstone = existing_value_index == FLAG_TOMBSTONE;
		if ((!found_tombstone) && is_tombstone) {
			tombstone_index = mapping_index;
			found_tombstone = true;
		}

		mapping_index = (mapping_index + 1) % header->mapping_capacity;
		existing_value_index = header->mapping_arr[mapping_index];
	}

	// Whatever is at index is either free, or the key is already in the map.
	//
	// If the key is already in the map, we instead want to use the earliest
	// tombstone index we found, if we found one.
	//
	// But this also means that we have to know to set the existing mapping
	// slot to a tombstone value at the time where we insert the key-value
	// pair.

	if (found_tombstone) {
		return (index_pair_t) {
			.has_old_index=true,
			.new_index=tombstone_index,
			.old_index=mapping_index,
		};
	} else {
		return (index_pair_t) {
			.has_old_index=false,
			.new_index=mapping_index,
		};
	}
}

void map_assure_growable_by_1(void **this, const char *file, int line) {
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, *this);
	unsigned int new_n_items = header->n_items + 1;
	if (new_n_items <= header->capacity) {
		return;
	}

	allocator_t allocator = header->allocator;
	unsigned int new_capacity = 1;
	if (header->capacity > 0) {
		new_capacity = header->capacity * 2;
	}
	internal_map_sizes_t new_size = internal_map_sizes(
		new_capacity,
		header->item_size
	);
	header = allocator_resize_func(allocator, header, new_size.bytes, file, line);
	header->mapping_arr = (int*) &header->bytes[new_size.real_cap_of_items_arr];
	header->mapping_capacity = mapping_cap(new_capacity);
	header->capacity=new_capacity;
	
	// first we have to clear the entire mapping array
	for ( unsigned int i = 0; i < header->mapping_capacity; i++ ) {
		header->mapping_arr[i] = FLAG_FREE;
	}

	// Then we have to re-hash and map all the items in the existing map.
	// The reason for re-hashing is that all hashes are moduloed by the
	// size of the mapping array, so the resulting hash will likely change.
	for ( unsigned int i = 0; i < header->n_items; i++ ) {
		void *pair = &this[i*header->item_size];
		// This is okay, because we absolutely know that the keys don't already
		// exist. (assuming all the other code for map is doing it's job)

		unsigned int mapping_index = map_pair_hash(this, pair).new_index;
		header->mapping_arr[mapping_index] = i;
	}
}

uint32_t fnv_1a(const uint8_t *bytes, const unsigned int size) {
	// Yanked from: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	const uint32_t PRIME = 0x01000193;
	const uint32_t OFFSET = 0x811c9dc5;
	uint32_t hash = OFFSET;
	for (unsigned int i = 0; i < size; i++) {
		uint32_t byte = (uint32_t)bytes[i];
		hash = hash ^ byte;
		hash = hash * PRIME;
	}
	return hash;
}

int map_len_func(void *this) {
	if (this == NULL) {
		return 0;
	}
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	return header->n_items;
}

int map_cap_func(void *this) {
	if (this == NULL) {
		return 0;
	}
	map_header_t *header = PTR_FROM_FIELD_PTR(map_header_t, bytes, this);
	return header->capacity;
}

#undef FLAG_FREE
#undef FLAG_TOMBSTONE

