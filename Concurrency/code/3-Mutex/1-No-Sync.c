#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "timing.h"

#define DEFAULT_LEN 10000
#define DEFAULT_THREADS 2

int len;
int counter = 0;


void* threadfunc(void* unused)
{
    (void)unused;

    for (int i = 0; i < len; ++i)
        ++counter;

    return NULL;
}


int main(int argc, char* argv[])
{
    int retcode = EXIT_FAILURE;
    double dt = 0.0;

    int nthread = DEFAULT_THREADS;
    len = DEFAULT_LEN;
    if (argc > 1)
        len = atoi(argv[1]);

    if (argc > 2)
        nthread = atoi(argv[2]);

    pthread_t* threads = (pthread_t*)malloc(nthread * sizeof(pthread_t));
    if (!threads)
        return retcode;

    dt -= cputime_ms();

    for (int i = 0; i < nthread; ++i)
    {
        if (pthread_create(&threads[i], NULL, threadfunc, NULL))
            goto cleanup;
    }

    for (int i = 0; i < nthread; ++i)
    {
        if (pthread_join(threads[i], NULL))
            goto cleanup;
    }
   
    dt += cputime_ms(); 

    retcode = EXIT_SUCCESS;

    printf("counter = %d; expected = %d\n", counter, len * nthread);
    printf("elapsed time = %lfms\n", dt);

cleanup:
    free (threads);

    return retcode;
}
