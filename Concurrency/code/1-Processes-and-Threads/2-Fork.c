#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


void huge_function()
{
    sleep(600);
}

int main()
{
    pid_t pid = fork();
    switch (pid) {
    case -1:
        fprintf(stderr, "fork failure.\n");
        return EXIT_FAILURE;
        break;

    case 0:
        printf("CHILD\n\tPID: %ld, PPID: %ld\n", (long)getpid(), (long)getppid());
        huge_function();
        break;

    default:
        printf("PARENT\n\tPID: %ld, PPID: %ld\n", (long)getpid(), (long)getppid());
        huge_function();
        break;
    }

    return EXIT_SUCCESS;
}