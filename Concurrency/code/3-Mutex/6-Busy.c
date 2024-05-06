#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


pthread_spinlock_t splk;

void* threadfunc(void* unused)
{
    (void)unused;

    printf("Acquiring the spinlock... ");
    fflush(stdout);
    
    pthread_spin_lock(&splk);
    printf("acquired\n");
    sleep(4);
    pthread_spin_unlock(&splk);

    return NULL;
}



int main()
{
    int retcode = EXIT_FAILURE;

    pthread_t thread;

    if (pthread_spin_init(&splk, PTHREAD_PROCESS_PRIVATE))
        return retcode;

    pthread_spin_lock(&splk);

    if (pthread_create(&thread, NULL, threadfunc, NULL))
        goto spinlock_cleanup;

    sleep(12);
    pthread_spin_unlock(&splk);

    if (!pthread_join(thread, NULL))
        retcode = EXIT_SUCCESS;

spinlock_cleanup:
    if (pthread_spin_destroy(&splk))
        return retcode;

    return retcode;
}
