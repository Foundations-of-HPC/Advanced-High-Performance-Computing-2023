#include "tscircbuf.h"


bool tscb_init(struct tscircbuf* tscb,
               void* buf,
               size_t el_size,
               size_t capacity)
{
    cb_init(&tscb->cb, buf, el_size, capacity);
    
    if (pthread_mutex_init(&tscb->mtx, NULL))
        return false;

    if (pthread_cond_init(&tscb->cv, NULL))
        return false;

    tscb->abort = false;

    return true;
}


bool tscb_unset(struct tscircbuf* tscb)
{
    if (pthread_cond_destroy(&tscb->cv))
        return false;

    if (pthread_mutex_destroy(&tscb->mtx))
        return false;

    tscb->abort = true;

    cb_unset(&tscb->cb);

    return true;
}


size_t tscb_size(struct tscircbuf* tscb)
{
    pthread_mutex_lock(&tscb->mtx);
    size_t size = cb_size(&tscb->cb);
    pthread_mutex_unlock(&tscb->mtx);

    return size;
}


size_t tscb_capacity(const struct tscircbuf* tscb)
{
    return cb_capacity(&tscb->cb);
}


bool tscb_try_push(struct tscircbuf* tscb, const void* el)
{
    pthread_mutex_lock(&tscb->mtx);

    if (cb_size(&tscb->cb) == cb_capacity(&tscb->cb)) {
        pthread_mutex_unlock(&tscb->mtx);
        return false;
    }

    bool retval = cb_push(&tscb->cb, el);
    pthread_mutex_unlock(&tscb->mtx);
    pthread_cond_broadcast(&tscb->cv);

    return retval;
}


bool tscb_wait_and_push(struct tscircbuf* tscb, const void* el)
{
    pthread_mutex_lock(&tscb->mtx);
    while (cb_size(&tscb->cb) == cb_capacity(&tscb->cb) && !tscb->abort)
        pthread_cond_wait(&tscb->cv, &tscb->mtx);

    if (tscb->abort) {
        pthread_mutex_unlock(&tscb->mtx);
        return false;
    }

    bool retval = cb_push(&tscb->cb, el);
    pthread_mutex_unlock(&tscb->mtx);
    pthread_cond_broadcast(&tscb->cv);

    return retval;
}


bool tscb_try_pop(struct tscircbuf* tscb, void* el)
{
    pthread_mutex_lock(&tscb->mtx);

    if (cb_size(&tscb->cb) == 0) {
        pthread_mutex_unlock(&tscb->mtx);
        return false;
    }

    bool retval = cb_pop(&tscb->cb, el);
    pthread_mutex_unlock(&tscb->mtx);
    pthread_cond_broadcast(&tscb->cv);

    return retval;
}


bool tscb_wait_and_pop(struct tscircbuf* tscb, void* el)
{
    pthread_mutex_lock(&tscb->mtx);
    while (cb_size(&tscb->cb) == 0 && !tscb->abort)
        pthread_cond_wait(&tscb->cv, &tscb->mtx);

    if (tscb->abort) {
        pthread_mutex_unlock(&tscb->mtx);
        return false;
    }

    bool retval = cb_pop(&tscb->cb, el);
    pthread_mutex_unlock(&tscb->mtx);
    pthread_cond_broadcast(&tscb->cv);

    return retval;
}


void tscb_abort_wait(struct tscircbuf* tscb)
{
    pthread_mutex_lock(&tscb->mtx);
    tscb->abort = true;
    pthread_mutex_unlock(&tscb->mtx);

    pthread_cond_broadcast(&tscb->cv);
}


void tscb_reset_abort(struct tscircbuf* tscb)
{
    pthread_mutex_lock(&tscb->mtx);
    tscb->abort = false;
    pthread_mutex_unlock(&tscb->mtx);

    pthread_cond_broadcast(&tscb->cv);
}

