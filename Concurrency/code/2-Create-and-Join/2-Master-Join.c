#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


void* thread_function(void* unused)
{
    (void)unused;

    printf("Child thread\n");

    return NULL;
}


int main()
{
    printf("Main thread\n");

    pthread_t thread;

    if (pthread_create(&thread, NULL, thread_function, NULL)) {
        fprintf(stderr, "pthread_create failure.\n");
        return EXIT_FAILURE;
    }

    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "pthread_joint failure.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
