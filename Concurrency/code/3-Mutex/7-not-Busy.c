#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* threadfunc(void* unused)
{
    (void)unused;

    printf("Acquiring the mutex... ");
    fflush(stdout);
    
    pthread_mutex_lock(&mtx);
    printf("acquired\n");
    sleep(4);
    pthread_mutex_unlock(&mtx);

    return NULL;
}



int main()
{
    int retcode = EXIT_FAILURE;

    pthread_t thread;

    pthread_mutex_lock(&mtx);

    if (pthread_create(&thread, NULL, threadfunc, NULL))
        return retcode;

    sleep(12);
    pthread_mutex_unlock(&mtx);

    if (!pthread_join(thread, NULL))
        retcode = EXIT_SUCCESS;

    return retcode;
}
