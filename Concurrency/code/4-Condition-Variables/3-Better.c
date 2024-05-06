#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#define DEFAULT_MAIN_SLEEP 3
#define DEFAULT_THREAD_SLEEP 2


pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
bool set = false;

void* threadfunc(void* vsleep_s)
{
    int sleep_s = *((int *)vsleep_s);

    printf("Inside threadfunc:\n");
    sleep(sleep_s);
    pthread_mutex_lock(&mtx);

    printf("Going to sleep... ");
    fflush(stdout);

    while (!set)
        pthread_cond_wait(&cv, &mtx);

    printf("Waken up!\n");

    pthread_mutex_unlock(&mtx);

    return NULL;
}


int main(int argc, char* argv[])
{
    int retcode = EXIT_FAILURE;

    int main_sleep = DEFAULT_MAIN_SLEEP;
    int thread_sleep = DEFAULT_THREAD_SLEEP;

    if (argc > 1)
        main_sleep = atoi(argv[1]);
    if (argc > 2)
        thread_sleep = atoi(argv[2]);


    pthread_t thread;
    
    if (pthread_create(&thread, NULL, threadfunc, &thread_sleep))
        return retcode;

    sleep(main_sleep);
    pthread_mutex_lock(&mtx);
    set = true;
    pthread_mutex_unlock(&mtx);

    pthread_cond_signal(&cv);

    pthread_join(thread, NULL);

    retcode = EXIT_SUCCESS;


    return retcode;
}
