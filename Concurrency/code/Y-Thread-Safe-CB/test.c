#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "tscircbuf.h"

#define DEFAULT_LEN 1000
#define DEFAULT_PRODUCERS 4
#define DEFAULT_CONSUMERS 4

#define TRUE (void*)true
#define FALSE (void*)false


void print_test_result(size_t n, const char* name, bool passed)
{
    printf("TEST %2lu: %-50s [%.6s]\n",
           n,
           name,
           passed ? "PASSED" : "FAILED");
}


void* producer_1(void* vtscb)
{
    struct tscircbuf* tscb = (struct tscircbuf*)vtscb;
    double el = 7.0;

    sleep(1);
    if (!tscb_try_push(tscb, &el))
        return FALSE;

    return TRUE;
}


void* consumer_1(void* vtscb)
{
    struct tscircbuf* tscb = (struct tscircbuf*)vtscb;
    double el;

    if (tscb_size(tscb))  // BEWARE: fragile! sleep-safe!
        return FALSE;

    if (!tscb_wait_and_pop(tscb, &el))
        return FALSE;
    
    if (el != 7.0)
        return FALSE;

    if (tscb_size(tscb))
        return FALSE;

    return TRUE;
}


void* producer_2(void* vtscb)
{
    struct tscircbuf* tscb = (struct tscircbuf*)vtscb;
    double el = 0.0;

    for (size_t i = 0; i < DEFAULT_LEN + 1; ++i) {
        if (!tscb_wait_and_push(tscb, &el)) {
            return FALSE;
        }
        ++el;
    }

    return TRUE;
}


void* consumer_2(void* vtscb)
{
    struct tscircbuf* tscb = (struct tscircbuf*)vtscb;
    double el;

    sleep(1);

    for (size_t i = 0; i < DEFAULT_LEN; ++i) {
        if (!tscb_try_pop(tscb, &el))
            return FALSE;

        if (el != (double)i)
            return FALSE;
    }

    sleep(1);

    if (!tscb_try_pop(tscb, &el))
        return FALSE;

    if (el != (double)DEFAULT_LEN)
        return FALSE;

    if (tscb_size(tscb))
        return FALSE;

    return TRUE;
}


void* producer_3(void* vtscb)
{
    struct tscircbuf* tscb = (struct tscircbuf*)vtscb;
    double el = 0.0;
    
    for (size_t i = 0; i < DEFAULT_LEN / DEFAULT_PRODUCERS; ++i) {
        if (!tscb_try_push(tscb, &el)) {
            printf("producer_3 fail: %ld\n", i);
            return FALSE;
        }
    }

    return TRUE;
}


void* consumer_3(void* vtscb)
{
    struct tscircbuf* tscb = (struct tscircbuf*)vtscb;
    double el;

    for (size_t i = 0; i < DEFAULT_LEN / DEFAULT_CONSUMERS; ++i) {
        if (!tscb_wait_and_pop(tscb, &el)) {
            printf("consumer_3 fail: %ld\n", i);
            return FALSE;
        }
    }

    return TRUE;
}


void* producer_4(void* vtscb)
{
    struct tscircbuf* tscb = (struct tscircbuf*)vtscb;

    sleep(5);
    tscb_abort_wait(tscb);

    return TRUE;
}


void* consumer_4(void* vtscb)
{
    struct tscircbuf* tscb = (struct tscircbuf*)vtscb;
    double el;

    tscb_wait_and_pop(tscb, &el);

    return TRUE;
}


int main(int argc, char* argv[])
{
    int retcode = EXIT_FAILURE;

    int len = DEFAULT_LEN;
    if (argc == 2)
        len = atoi(argv[1]);
    

    double* buffer = (double*)malloc(len * sizeof(double));
    if (!buffer) {
        fprintf(stderr, "Impossible to allocate buffer.\n");
        return retcode;
    }

    struct tscircbuf tscb;
    if (!tscb_init(&tscb, buffer, sizeof(double), len))
        goto free_buffer;


    pthread_t producers[DEFAULT_PRODUCERS], consumers[DEFAULT_CONSUMERS];
    
    pthread_create(&producers[0], NULL, producer_1, (void*)&tscb);
    pthread_create(&consumers[0], NULL, consumer_1, (void*)&tscb);

    void *ret_consumer, *ret_producer;
    pthread_join(consumers[0], &ret_consumer);
    pthread_join(producers[0], &ret_producer);

    bool passed = (bool)ret_producer & (bool)ret_consumer;
    print_test_result(1, "producer: try, consumer: wait", passed);


    pthread_create(&producers[0], NULL, producer_2, (void*)&tscb);
    pthread_create(&consumers[0], NULL, consumer_2, (void*)&tscb);

    pthread_join(consumers[0], &ret_consumer);
    pthread_join(producers[0], &ret_producer);

    passed = (bool)ret_producer & (bool)ret_consumer;
    print_test_result(2, "producer: wait, consumer: try", passed);

    
    for (size_t i = 0; i < DEFAULT_PRODUCERS; ++i)
        pthread_create(&producers[i], NULL, producer_3, (void*)&tscb);

    passed = true;
    for (size_t i = 0; i < DEFAULT_PRODUCERS; ++i) {
        pthread_join(producers[i], &ret_producer);
        passed &= (bool)ret_producer;
    }

    size_t expected_size = DEFAULT_LEN / DEFAULT_PRODUCERS * DEFAULT_PRODUCERS;
    passed &= (tscb_size(&tscb) == expected_size);

    for (size_t i = 0; i < DEFAULT_CONSUMERS; ++i) {
        pthread_create(&consumers[i], NULL, consumer_3, (void*)&tscb);
    }

    for (size_t i = 0; i < DEFAULT_CONSUMERS; ++i) {
        pthread_join(consumers[i], &ret_consumer);
        passed &= (bool)ret_consumer;
    }

    expected_size -= DEFAULT_LEN / DEFAULT_CONSUMERS * DEFAULT_CONSUMERS;
    size_t actual_size = tscb_size(&tscb);
    passed &= (actual_size == expected_size);

    print_test_result(3, "multiple producers: try, multiple consumers: wait", passed);

    for (size_t i = 0; i < actual_size; ++i) {
        double el;
        if (!tscb_try_pop(&tscb, &el))
            break;
    }

    passed = tscb_size(&tscb) == 0;

    print_test_result(4, "empty buffer", passed);

    
    pthread_create(&producers[0], NULL, producer_4, (void*)&tscb);
    pthread_create(&consumers[0], NULL, consumer_4, (void*)&tscb);
    pthread_create(&consumers[1], NULL, consumer_4, (void*)&tscb);

    pthread_join(consumers[1], &ret_consumer);
    passed = (bool)ret_consumer;
    pthread_join(consumers[0], &ret_consumer);
    passed &= (bool)ret_consumer;
    pthread_join(producers[0], &ret_producer);
    passed &= (bool)ret_producer;

    print_test_result(5, "abort wait", passed);


    tscb_unset(&tscb);

free_buffer:
    free(buffer);

    return retcode;
}
