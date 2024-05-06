#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


pthread_t thread_2_handle;


void* thread_2_fn(void* unused)
{
    (void)unused;

    printf("Thread 2\n");

    return NULL;
}


void* thread_1_fn(void* unused)
{
    (void)unused;

    printf("Thread 1\n");

    if (pthread_create(&thread_2_handle, NULL, thread_2_fn, NULL)) {
        fprintf(stderr, "pthread_create failure.\n");
    }

    return NULL;
}



int main()
{
    pthread_t thread_1_handle;

    printf("Main thread\n");

    if (pthread_create(&thread_1_handle, NULL, thread_1_fn, NULL)) {
        fprintf(stderr, "pthread_create failure.\n");
    }

    // BEWARE: this is just a poor man example
    //         NEVER use sleep to synchronize!
    sleep(1);

    if (pthread_join(thread_1_handle, NULL)) {
        fprintf(stderr, "pthread_join failure.\n");
    }

    if (pthread_join(thread_2_handle, NULL)) {
        fprintf(stderr, "pthread_join failure.\n");
    }

    return EXIT_SUCCESS;
}
