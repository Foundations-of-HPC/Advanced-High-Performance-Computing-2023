#pragma once

#include "circbuf.h"
#include <pthread.h>


struct tscircbuf {
    struct circbuf cb;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    bool abort;
};

bool tscb_init(struct tscircbuf* tscb,
               void* buf,
               size_t el_size,
               size_t capacity);
bool tscb_unset(struct tscircbuf* tscb);
size_t tscb_size(struct tscircbuf* tscb);
size_t tscb_capacity(const struct tscircbuf* tscb);
bool tscb_try_push(struct tscircbuf* tscb, const void* el);
bool tscb_wait_and_push(struct tscircbuf* tscb, const void* el);
bool tscb_try_pop(struct tscircbuf* tscb, void* el);
bool tscb_wait_and_pop(struct tscircbuf* tscb, void* el);
void tscb_abort_wait(struct tscircbuf* tscb);
void tscb_reset_abort(struct tscircbuf* tscb);
