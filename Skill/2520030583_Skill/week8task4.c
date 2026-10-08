#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 4

int main()
{
    // Commands in the pipeline
    char *commands[MAX_COMMANDS][4] =
    {
        {"ls", NULL},
        {"grep", ".c", NULL},
        {"wc", "-l", NULL},
        {"cat", NULL}
    };

    // Number of commands
    int num_commands = 3;

    // Pipe descriptors
    int pipes[MAX_COMMANDS - 1][2];

    // Process IDs
    pid_t pids[MAX_COMMANDS];

    // Create required pipes
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return 1;
        }
    }

    // Create processes
    for (int i = 0; i < num_commands; i++)
    {
        pids[i] = fork();

        if (pids[i] == -1)
        {
            perror("fork");
            return 1;
        }

        if (pids[i] == 0)
        {
            // If not the first command,
            // take input from previous pipe
            if (i > 0)
            {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // If not the last command,
            // send output to next pipe
            if (i < num_commands - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipe descriptors
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute command
            execvp(commands[i][0], commands[i]);

            perror("execvp");
            exit(1);
        }
    }

    // Parent closes all pipe descriptors
    for (int i = 0; i < num_commands - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes
    for (int i = 0; i < num_commands; i++)
    {
        waitpid(pids[i], NULL, 0);
    }

    printf("Multiple pipeline execution completed.\n");

    return 0;
}
