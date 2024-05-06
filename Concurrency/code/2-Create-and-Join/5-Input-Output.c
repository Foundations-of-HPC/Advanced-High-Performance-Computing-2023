#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct thread_input {
    unsigned int numerator;
    unsigned int denominator;
};

struct thread_output {
    unsigned int quotient;
    unsigned int reminder;
};


void* thread_function(void* arg)
{
    struct thread_input* input = (struct thread_input*)arg;
    struct thread_output* output = (struct thread_output*)malloc(sizeof(struct thread_output));

    if (!output)
        return NULL;

    output->quotient = input->numerator / input->denominator;
    output->reminder = input->numerator % input->denominator;

    return output;
}


int main()
{
    int exit_code = EXIT_FAILURE;

    struct thread_input* input = (struct thread_input*)malloc(sizeof(struct thread_input));
    if (!input) {
        fprintf(stderr, "malloc failure.\n");
        return exit_code;
    }

    input->numerator = 25;
    input->denominator = 7;

    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_function, (void*)input)) {
       fprintf(stderr, "pthread_create failure.\n");
       goto cleanup_input;
    }

    struct thread_output* output;
    if (pthread_join(thread, (void**)&output)) {
        fprintf(stderr, "pthread_join failure.\n");
        goto cleanup_input;
    }

    if (!output) {
        fprintf(stderr, "thread malloc failure.\n");
        goto cleanup_input;
    }

    printf("%d divided by %d is %d with reminder %d.\n",
           input->numerator,
           input->denominator,
           output->quotient,
           output->reminder);

    exit_code = EXIT_SUCCESS;

    free(output);

cleanup_input:
    free(input);

    return exit_code;
}
