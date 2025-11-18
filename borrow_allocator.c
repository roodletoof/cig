#include "cig.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

void *borrow_allocator_alloc_func(borrow_allocator_t *this, size_t bytes, const char *file, int line) {
    linked_allocation_node_t *node = malloc(sizeof(*this->head) + bytes);
    if (node == NULL) { return NULL; }
    node->file = file;
    node->line = line;
    node->prev = NULL;
    node->next = this->head;
    if (this->head != NULL) {
        assert(this->head->prev == NULL);
        this->head->prev = node;
    }
    this->head = node;
    return &node->data;
}

void *borrow_allocator_resize_func(borrow_allocator_t *this, void *old_ptr, size_t bytes, const char *file, int line) {
    linked_allocation_node_t *old_node = PTR_FROM_FIELD_PTR(linked_allocation_node_t, data, old_ptr);
    linked_allocation_node_t *new_node = realloc(old_node, sizeof(*old_node) + bytes);
    if (new_node == NULL) { return NULL; }
    new_node->file = file;
    new_node->line = line;
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

static void *borrow_alloc(void *this, size_t bytes, const char *file, int line) {
    return borrow_allocator_alloc_func(this, bytes, file, line);
}

static void *borrow_resize(void *this, void *old_ptr, size_t bytes, const char *file, int line) {
    return borrow_allocator_resize_func(this, old_ptr, bytes, file, line);
}

static void borrow_free(void *this, void *ptr, const char *file, int line) {
    (void)file;
    (void)line;
    borrow_allocator_free(this, ptr);
}

static const allocator_vtbl_t borrow_vtbl = {
    .alloc = borrow_alloc,
    .resize = borrow_resize,
    .free = borrow_free,
};

allocator_t borrow_allocator_interface(borrow_allocator_t *this) {
    return (allocator_t) {
        .this=this,
        .vtbl=&borrow_vtbl,
    };
}

void borrow_allocator_assert_all_freed(borrow_allocator_t *this) {
    bool should_crash = false;
    for (linked_allocation_node_t *curr = this->head; curr != NULL; curr = curr->next) {
        fprintf(stderr, "%s:%d: allocation not freed\n", curr->file, curr->line);
    }
    if (should_crash) {
        exit(1);
    }
}
