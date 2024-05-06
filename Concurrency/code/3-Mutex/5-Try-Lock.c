#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;


void* threadfunc(void* unused)
{
    (void)unused;
    while (pthread_mutex_trylock(&mtx)) {
        printf("mutex is locked, I will try again...\n");
        sleep(1);
    }
    printf("finally I locked it!\n");

    sleep(1);
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

    sleep(7);
    pthread_mutex_unlock(&mtx);
    
    if (pthread_join(thread, NULL))
        return retcode;

    retcode = EXIT_SUCCESS;

    return retcode;
}
