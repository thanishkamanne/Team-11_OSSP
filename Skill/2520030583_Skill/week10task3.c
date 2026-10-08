#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;

    printf("Starting background job...\n");

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        /* Child process */

        printf("Background process started. PID = %d\n", getpid());

        sleep(5);

        printf("Background process completed. PID = %d\n", getpid());

        exit(0);
    }
    else
    {
        /* Parent process */

        printf("Parent continues immediately.\n");
        printf("Background Job PID = %d\n", pid);

        /*
         * Parent does NOT wait here.
         * Therefore it can continue immediately.
         */

        printf("Shell is ready for the next command.\n");

        sleep(6);

        /* Collect the completed child */
        waitpid(pid, NULL, 0);

        printf("Background job has been collected.\n");
    }

    return 0;
}
