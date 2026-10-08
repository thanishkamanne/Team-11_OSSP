#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
    pid_t pid;
    int status;

    printf("Creating a background job...\n");

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        /* Child process */

        printf("Child process started. PID = %d\n", getpid());

        printf("Child is working...\n");

        sleep(5);

        printf("Child process completed.\n");

        exit(0);
    }
    else
    {
        /* Parent process */

        printf("Background Job Created.\n");
        printf("Job PID = %d\n", pid);

        printf("\nParent is doing other work...\n");

        sleep(2);

        printf("\nSwitching job to foreground...\n");

        /*
         * Wait for the selected foreground job
         */
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            printf("Foreground job completed successfully.\n");
        }
        else
        {
            printf("Foreground job did not exit normally.\n");
        }

        printf("Job state updated to Completed.\n");
    }

    return 0;
}
