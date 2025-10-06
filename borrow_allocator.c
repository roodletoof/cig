#include "allocator.h"


void *borrow_allcoator_alloc(borrow_allocator_t *this, size_t bytes) {
    linked_allocation_node_t *node = malloc(sizeof(*this->head) + bytes);
    node->prev = NULL;
    node->next = this->head;
    if (this->head != NULL) {
        // TODO: correct?
        this->head.prev = node;
    }
    this->head = node;
}
void *borrow_allcoator_resize(borrow_allocator_t *this, void *old_ptr, size_t bytes) {
}
void borrow_allcoator_free(borrow_allocator_t *this, void *old_ptr) {
}
