#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>


struct resource {
    double value;
    pthread_mutex_t mtx;
};

struct resource r1 = {.value = 0.0,
                      .mtx = PTHREAD_MUTEX_INITIALIZER};

struct resource r2 = {.value = 0.0,
                      .mtx = PTHREAD_MUTEX_INITIALIZER};


void* threadswap(void* resources)
{
    struct resource* first = ((struct resource**)resources)[0];
    struct resource* second = ((struct resource**)resources)[1];

    for (;;) {
        pthread_mutex_lock(&first->mtx);
        usleep(10000); // just to simulate some load
        if (pthread_mutex_trylock(&second->mtx)) {
            pthread_mutex_unlock(&first->mtx);
            sched_yield();
        } else {
            break;
        }
    }

    double tmp = first->value;
    first->value = second->value;
    second->value = tmp;

    pthread_mutex_unlock(&second->mtx);
    pthread_mutex_unlock(&first->mtx);

    return NULL;
}


int main()
{
    int retcode = EXIT_FAILURE;

    pthread_mutex_lock(&r1.mtx);
    r1.value = 37.0;
    pthread_mutex_unlock(&r1.mtx);

    pthread_mutex_lock(&r2.mtx);
    r2.value = -4.0;
    pthread_mutex_unlock(&r2.mtx);

    pthread_t thread1, thread2;
    struct resource* resources_1[] = {&r1, &r2};
    struct resource* resources_2[] = {&r2, &r1};

    if (pthread_create(&thread1, NULL, threadswap, resources_1))
        return retcode;

    if (pthread_create(&thread2, NULL, threadswap, resources_2))
        goto thread1_cleanup;

    if (pthread_join(thread2, NULL))
        goto thread1_cleanup;

    retcode = EXIT_SUCCESS;

thread1_cleanup:
    if (pthread_join(thread1, NULL))
        return EXIT_FAILURE;

    if (retcode == EXIT_FAILURE)
        return retcode;

    pthread_mutex_lock(&r1.mtx);
    printf("r1.value = %lf\n", r1.value);
    pthread_mutex_unlock(&r1.mtx);

    pthread_mutex_lock(&r2.mtx);
    printf("r2.value = %lf\n", r2.value);
    pthread_mutex_unlock(&r2.mtx);
    
    return EXIT_SUCCESS;
}
