#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int pipefd[2];
    pid_t pid1, pid2;

    // Create pipe
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create first child process for ls -l
    pid1 = fork();

    if (pid1 == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0)
    {
        // Child 1

        // Close unused read end
        close(pipefd[0]);

        // Redirect stdout to pipe
        dup2(pipefd[1], STDOUT_FILENO);

        // Close original write end
        close(pipefd[1]);

        // Execute ls -l
        execlp("ls", "ls", "-l", (char *)NULL);

        perror("execlp");
        exit(EXIT_FAILURE);
    }

    // Create second child process for grep ".c"
    pid2 = fork();

    if (pid2 == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0)
    {
        // Child 2

        // Close unused write end
        close(pipefd[1]);

        // Redirect stdin to pipe
        dup2(pipefd[0], STDIN_FILENO);

        // Close original read end
        close(pipefd[0]);

        // Execute grep ".c"
        execlp("grep", "grep", ".c", (char *)NULL);

        perror("execlp");
        exit(EXIT_FAILURE);
    }

    // Parent process closes both pipe ends
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for child processes
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
