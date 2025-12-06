#include "cig.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

string_builder_t sb_create(allocator_t builder_allocator) {
	return (string_builder_t) {
		.allocator=builder_allocator,
		.head=NULL,
		.tail=NULL
	};
}

void sb_add_string(string_builder_t *this, const char *string) {
	string_builder_node_t *node = allocator_alloc(this->allocator, sizeof(string_builder_node_t));
	node->length = strlen(string);
	node->string = string;
	node->next = NULL;
	if (this->tail == NULL) {
		assert(this->head == NULL);
		this->head = node;
		this->tail = node;
	} else {
		this->tail->next = node;
		this->tail = node;
	}
}

// 21 bytes is the max needed ever for a 64 bit integer, even unsigned.
#define MAX_SIZE (21)

void sb_add_i64(string_builder_t *this, int64_t i64) {
	char *buffer = allocator_alloc(this->allocator, MAX_SIZE);
	snprintf(buffer, MAX_SIZE, "%"PRIi64, i64);
	sb_add_string(this, buffer);
}

void sb_add_i32(string_builder_t *this, int32_t i32) {
	char *buffer = allocator_alloc(this->allocator, MAX_SIZE);
	snprintf(buffer, MAX_SIZE, "%"PRIi32, i32);
	sb_add_string(this, buffer);
}

void sb_add_i16(string_builder_t *this, int16_t i16) {
	char *buffer = allocator_alloc(this->allocator, MAX_SIZE);
	snprintf(buffer, MAX_SIZE, "%"PRIi16, i16);
	sb_add_string(this, buffer);
}

void sb_add_i8(string_builder_t *this, int8_t i8) {
	char *buffer = allocator_alloc(this->allocator, MAX_SIZE);
	snprintf(buffer, MAX_SIZE, "%"PRIi8, i8);
	sb_add_string(this, buffer);
}

void sb_add_u64(string_builder_t *this, uint64_t u64) {
	char *buffer = allocator_alloc(this->allocator, MAX_SIZE);
	snprintf(buffer, MAX_SIZE, "%"PRIu64, u64);
	sb_add_string(this, buffer);
}

void sb_add_u32(string_builder_t *this, uint32_t u32) {
	char *buffer = allocator_alloc(this->allocator, MAX_SIZE);
	snprintf(buffer, MAX_SIZE, "%"PRIu32, u32);
	sb_add_string(this, buffer);
}

void sb_add_u16(string_builder_t *this, uint16_t u16) {
	char *buffer = allocator_alloc(this->allocator, MAX_SIZE);
	snprintf(buffer, MAX_SIZE, "%"PRIu16, u16);
	sb_add_string(this, buffer);
}

void sb_add_u8(string_builder_t *this, uint8_t u8) {
	char *buffer = allocator_alloc(this->allocator, MAX_SIZE);
	snprintf(buffer, MAX_SIZE, "%"PRIu8, u8);
	sb_add_string(this, buffer);
}

void sb_add_substring(string_builder_t *this, const char *string, size_t substring_length) {
	size_t string_length = strlen(string);
	size_t min_length = string_length < substring_length
		? string_length
		: substring_length;
	char *buffer = allocator_alloc(this->allocator, min_length + 1);
	for ( size_t i = 0; i < min_length; i++ ) {
		buffer[i] = string[i];
	}
	buffer[min_length-1] = '\0';
	sb_add_string(this, buffer);
}

const char *sb_build(
	string_builder_t *this,
	allocator_t output_allocator
) {
	size_t total_length = 0;
	for (
		string_builder_node_t *node = this->head;
		node != NULL;
		node = node->next
	) {
		total_length += node->length;
	}
	total_length++; // null character
	char *buffer = allocator_alloc(output_allocator, total_length * sizeof(char));
	size_t i = 0;
	for (
		string_builder_node_t *node = this->head;
		node != NULL;
		node = node->next
	) {
		for (const char *c = node->string; (*c) != '\0'; c++) {
			buffer[i] = *c;
			i++;
		}
	}
	buffer[total_length-1] = '\0';
	return buffer;
}
