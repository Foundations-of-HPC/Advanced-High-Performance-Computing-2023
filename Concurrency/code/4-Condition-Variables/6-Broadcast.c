#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define WTHREADS 10

pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
int order = 0;


void* threadfunc(void* vorder)
{
    int myorder = *((int*)vorder);

    pthread_mutex_lock(&mtx);
    
    while (order != myorder)
        pthread_cond_wait(&cv, &mtx);

    printf("Set %d\n", myorder);
    ++order;

    pthread_mutex_unlock(&mtx);


    pthread_cond_broadcast(&cv);

    return NULL;
}


int main()
{
    int retcode = EXIT_FAILURE;

    pthread_t threads[WTHREADS];
    int all_order[WTHREADS];

    int running_threads = 0;
    for (int i = 0; i < WTHREADS; ++i) {
        all_order[i] = WTHREADS - i;
        if (pthread_create(&threads[i], NULL, threadfunc, &all_order[i]))
            goto threads_cleanup;
        ++running_threads;
    }

    printf("About to start...\n");
    sleep(1);
    pthread_mutex_lock(&mtx);
    ++order;
    pthread_mutex_unlock(&mtx);

    pthread_cond_broadcast(&cv);


    retcode = EXIT_SUCCESS;

threads_cleanup:
    for (int i = 0; i < running_threads; ++i)
        pthread_join(threads[i], NULL);

    return retcode;
}
