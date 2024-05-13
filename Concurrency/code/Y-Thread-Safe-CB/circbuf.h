#pragma once

#include <stddef.h>
#include <stdbool.h>

struct circbuf {
    void* buffer;
    size_t el_size;
    size_t capacity;

    void* head;
    void* tail;
    size_t size;
};


void cb_init(struct circbuf* cb,
             void* buf,
             size_t el_size,
             size_t capacity);
void cb_unset(struct circbuf* cb);
size_t cb_size(const struct circbuf* cb); 
size_t cb_capacity(const struct circbuf* cb);
bool cb_push(struct circbuf* cb, const void* el);
bool cb_pop(struct circbuf* cb, void* el);

#ifdef DEBUG
ptrdiff_t cb_head_offset(const struct circbuf* cb);
ptrdiff_t cb_tail_offset(const struct circbuf* cb);
#endif //DEBUG
