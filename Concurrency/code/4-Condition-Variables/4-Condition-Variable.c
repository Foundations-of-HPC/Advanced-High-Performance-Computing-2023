/*
 * Adapted from: M. Kerrisk, The Linux Programming Interface.
 *
 * https://github.com/prm239/kerrisk/tree/master
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define DEFAULT_TOTAL 20


int avail;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;


void* producer(void* tot)
{
    const int total = atoi((char*)tot);
    printf("[PRODUCER]: %d\n", total);

    for (int i = 0; i < total; ++i) {
        sleep(1);

        pthread_mutex_lock(&mtx);
        ++avail;
        pthread_mutex_unlock(&mtx);
    }

    pthread_cond_signal(&cv);

    printf("[PRODUCER]: end.\n");

    return NULL;
}


void* consumer(void* tot)
{
    int total = *(int *)tot;
    printf("[CONSUMER]: %d\n", total);
    
    while (total) {
        pthread_mutex_lock(&mtx);
        
        while (!avail)
            pthread_cond_wait(&cv, &mtx);

        while (avail) {
            --avail;
            --total;
        }

        pthread_mutex_unlock(&mtx);
    }

    printf("[CONSUMER]: end.\n");

    return NULL;
}


int main(int argc, char* argv[])
{
    int retcode = EXIT_FAILURE;

    int to_consume = 0;
    int producers = argc > 1 ? argc - 1 : 0;
    int started_producers = 0;

    pthread_t* producers_th = (pthread_t*)malloc(sizeof(pthread_t) * producers);
    if (!producers_th)
        return retcode;

    pthread_t consumer_th;

    for (int i = 0; i < producers; ++i) {
        to_consume += atoi(argv[i + 1]);
        if (pthread_create(&producers_th[i], NULL, producer, argv[i + 1]))
            goto producers_cleanup;
        ++started_producers;
    }

    if (pthread_create(&consumer_th, NULL, consumer, &to_consume))
        goto producers_cleanup;

    retcode = EXIT_SUCCESS;

    pthread_join(consumer_th, NULL);

producers_cleanup:
    for (int i = 0; i < started_producers; ++i)
        pthread_join(producers_th[i], NULL);

    free(producers_th);

    return retcode;
}

