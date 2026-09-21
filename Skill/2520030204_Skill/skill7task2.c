#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    int status;

    printf("====================================\n");
    printf("       CHILD PROCESS LAUNCHER\n");
    printf("====================================\n");

    printf("Parent Process ID: %d\n", getpid());

    // Create child process
    pid = fork();

    if (pid < 0)
    {
        // Fork failed
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        // Child process
        printf("\n--- Child Process ---\n");
        printf("Child Process ID: %d\n", getpid());
        printf("Parent Process ID: %d\n", getppid());

        printf("\nExecuting 'ls' command...\n");

        // Execute ls with arguments
        execlp("ls", "ls", "-l", NULL);

        // This executes only if exec fails
        perror("Execution failed");
        exit(1);
    }
    else
    {
        // Parent process
        printf("\n--- Parent Process ---\n");
        printf("Parent is waiting for child process...\n");

        // Wait for child to finish
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            printf("\nChild process completed successfully.\n");
            printf("Exit Status: %d\n", WEXITSTATUS(status));
        }
        else
        {
            printf("\nChild process terminated abnormally.\n");
        }

        printf("Parent process completed.\n");
    }

    return 0;
}
