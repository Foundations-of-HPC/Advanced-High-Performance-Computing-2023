#include "circbuf.h"

#include <string.h>


inline static void ptr_advance(const struct circbuf* cb,
                               void** ptr)
{
    char* ptr_c = (char*)(*ptr);
    char* const end_buffer = (char*)cb->buffer + cb->capacity * cb->el_size;
    ptr_c += cb->el_size;
    *ptr = ptr_c < end_buffer ? ptr_c : cb->buffer;
}


void cb_init(struct circbuf* cb, 
             void* buf,
             size_t el_size,
             size_t capacity)
{
    cb->buffer = buf;
    cb->el_size = el_size;
    cb->capacity = capacity;

    cb->head = buf;
    cb->tail = buf;
    cb->size = 0;
}

void cb_unset(struct circbuf* cb)
{
    cb_init(cb, NULL, 0, 0);
}

size_t cb_size(const struct circbuf* cb)
{
    return cb->size;
}

size_t cb_capacity(const struct circbuf* cb)
{
    return cb->capacity;
}

bool cb_push(struct circbuf* cb, const void* el)
{
    if (cb->size == cb->capacity)
        return false;

    memcpy(cb->head, el, cb->el_size);
    ptr_advance(cb, &cb->head);
    ++cb->size;
    return true;
}

bool cb_pop(struct circbuf* cb, void* el)
{
    if (!cb->size)
        return 0;

    memcpy(el, cb->tail, cb->el_size);
    ptr_advance(cb, &cb->tail);
    --cb->size;
    return true;
}


#ifdef DEBUG

ptrdiff_t cb_head_offset(const struct circbuf* cb)
{
    ptrdiff_t dptr = (char*)cb->head - (char*)cb->buffer;
    return dptr / cb->el_size;
}

ptrdiff_t cb_tail_offset(const struct circbuf* cb)
{
    ptrdiff_t dptr = (char*)cb->tail - (char*)cb->buffer;
    return dptr / cb->el_size;
}

#endif //DEBUG
