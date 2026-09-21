#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    int status;

    printf("========================================\n");
    printf(" PROCESS SYNCHRONIZATION USING WAITPID\n");
    printf("========================================\n");

    printf("Parent Process ID: %d\n", getpid());

    // Create child process
    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    // Child process
    if (pid == 0)
    {
        printf("\n--- Child Process ---\n");
        printf("Child PID: %d\n", getpid());
        printf("Child is working...\n");

        // Simulate some work
        sleep(3);

        printf("Child work completed.\n");
        printf("Child is exiting...\n");

        exit(0);
    }

    // Parent process
    else
    {
        printf("\n--- Parent Process ---\n");
        printf("Parent is waiting for child PID: %d\n", pid);

        // Wait specifically for the child
        pid_t result = waitpid(pid, &status, 0);

        if (result == -1)
        {
            perror("waitpid failed");
            return 1;
        }

        // Check how child terminated
        if (WIFEXITED(status))
        {
            printf("\nChild terminated normally.\n");
            printf("Child Exit Status: %d\n",
                   WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("\nChild terminated by a signal.\n");
            printf("Signal Number: %d\n",
                   WTERMSIG(status));
        }
        else
        {
            printf("\nChild terminated abnormally.\n");
        }

        printf("\nParent process continues...\n");
        printf("Parent Process Completed.\n");
    }

    return 0;
}
