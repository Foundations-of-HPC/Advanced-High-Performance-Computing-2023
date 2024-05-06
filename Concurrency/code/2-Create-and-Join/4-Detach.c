#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


void* thread_function(void* unused)
{
    (void)unused;
    
    if (pthread_detach(pthread_self())) {
        fprintf(stderr, "pthread_detach failure.\n");
    }

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

    sleep(1);

    return EXIT_SUCCESS;
}
