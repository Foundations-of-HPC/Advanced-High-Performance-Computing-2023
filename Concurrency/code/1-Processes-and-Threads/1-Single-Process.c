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
    printf("PID: %ld, PPID: %ld\n", (long)getpid(), (long)getppid());
    huge_function();

    return EXIT_SUCCESS;
}
