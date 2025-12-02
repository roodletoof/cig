#include "cig.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

void *borrow_allocator_alloc_func(borrow_allocator_t *this, size_t bytes) {
    linked_allocation_node_t *node = malloc(sizeof(*this->head) + bytes);
    if (node == NULL) { return NULL; }
    node->prev = NULL;
    node->next = this->head;
    if (this->head != NULL) {
        assert(this->head->prev == NULL);
        this->head->prev = node;
    }
    this->head = node;
    return &node->data;
}

void *borrow_allocator_resize_func(borrow_allocator_t *this, void *old_ptr, size_t bytes) {
    linked_allocation_node_t *old_node = PTR_FROM_FIELD_PTR(linked_allocation_node_t, data, old_ptr);
    linked_allocation_node_t *new_node = realloc(old_node, sizeof(*old_node) + bytes);
    if (new_node == NULL) { return NULL; }
    if (this->head == old_node) {
        assert(new_node->prev == NULL);
        this->head = new_node;
    }
    if (new_node->prev != NULL) { new_node->prev->next = new_node; }
    if (new_node->next != NULL) { new_node->next->prev = new_node; }
    return &new_node->data;
}

void borrow_allocator_free(borrow_allocator_t *this, void *old_ptr) {
    linked_allocation_node_t *node = PTR_FROM_FIELD_PTR(linked_allocation_node_t, data, old_ptr);
    if (node->prev != NULL) { node->prev->next = node->next; }
    if (node->next != NULL) { node->next->prev = node->prev; }
    if (this->head == node) {
        assert(node->prev == NULL);
        if (node->next != NULL) { assert(node->next->prev == NULL); }
        this->head = node->next;
    }
    free(node);
}

void borrow_allocator_free_all(borrow_allocator_t *this) {
    if (this->head == NULL) {
        return;
    }
    assert(this->head->prev == NULL);
    void *prev = NULL;
    for (linked_allocation_node_t *node = this->head; node != NULL; node = node->next) {
        free(prev);
        prev = node;
    }
    free(prev);
    this->head = NULL;
}

size_t borrow_allocator_count_allocations(borrow_allocator_t *this) {
    size_t output = 0;
    for (linked_allocation_node_t *node = this->head; node != NULL; node = node->next) {
        output++;
    }
    return output;
}

static void *borrow_alloc_impl(void *this, size_t bytes) {
    return borrow_allocator_alloc_func(this, bytes);
}

static void *borrow_resize_impl(void *this, void *old_ptr, size_t bytes) {
    return borrow_allocator_resize_func(this, old_ptr, bytes);
}

static void borrow_reset(void *this) {
    borrow_allocator_free_all(this);
}

static const allocator_vtbl_t borrow_vtbl = {
    .alloc = borrow_alloc_impl,
    .resize = borrow_resize_impl,
    .reset = borrow_reset,
};

allocator_t borrow_allocator(borrow_allocator_t *this) {
    return (allocator_t) {
        .this=this,
        .vtbl=&borrow_vtbl,
    };
}
