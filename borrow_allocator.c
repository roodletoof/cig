#include "allocator.h"

void *borrow_allcoator_alloc(borrow_allocator_t *this, size_t bytes) {
    linked_allocation_node_t *node = malloc(sizeof(*this->head) + bytes);
    if (node == NULL) { return NULL; }
    node->prev = NULL;
    node->next = this->head;
    if (this->head != NULL) {
        this->head->prev = node;
    }
    this->head = node;
}

// WARNING: it is important to consider the case where the old_node is the head
// node.

void *borrow_allcoator_resize(borrow_allocator_t *this, void *old_ptr, size_t bytes) {
    linked_allocation_node_t *old_node = PTR_FROM_FIELD_PTR(linked_allocation_node_t, data, old_ptr);
    // TODO: update the links
}
void borrow_allcoator_free(borrow_allocator_t *this, void *old_ptr) {
    linked_allocation_node_t *old_node = PTR_FROM_FIELD_PTR(linked_allocation_node_t, data, old_ptr);
    // TODO: update the links
    free(old_node);
}
