#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct thread_output;

struct thread_input {
    unsigned int numerator;
    unsigned int denominator;
    struct thread_output* output;
};

struct thread_output {
    unsigned int quotient;
    unsigned int reminder;
};


void* thread_function(void* arg)
{
    struct thread_input* input = (struct thread_input*)arg;

    input->output->quotient = input->numerator / input->denominator;
    input->output->reminder = input->numerator % input->denominator;

    return NULL;
}


int main()
{
    int exit_code = EXIT_FAILURE;

    struct thread_input* input = (struct thread_input*)malloc(sizeof(struct thread_input));
    if (!input) {
        fprintf(stderr, "malloc failure.\n");
        return exit_code;
    }

    input->output = (struct thread_output*)malloc(sizeof(struct thread_output));
    if (!input->output) {
        fprintf(stderr, "malloc failure.\n");
        goto cleanup_input;
    }

    input->numerator = 25;
    input->denominator = 7;

    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_function, (void*)input)) {
       fprintf(stderr, "pthread_create failure.\n");
       goto cleanup_output;
    }

    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "pthread_join failure.\n");
        goto cleanup_output;
    }

    printf("%d divided by %d is %d with reminder %d.\n",
           input->numerator,
           input->denominator,
           input->output->quotient,
           input->output->reminder);

    exit_code = EXIT_SUCCESS;

cleanup_output:
    free(input->output);

cleanup_input:
    free(input);

    return exit_code;
}
