#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
    int fd[2];

    pid_t child1, child2;

    // Create pipe
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    // Create first child
    child1 = fork();

    if (child1 == -1)
    {
        perror("fork");
        return 1;
    }

    if (child1 == 0)
    {
        // Child 1: ls

        // Close unused read end
        close(fd[0]);

        // Redirect stdout to pipe
        dup2(fd[1], STDOUT_FILENO);

        // Close original descriptor
        close(fd[1]);

        // Execute ls
        execlp("ls", "ls", NULL);

        perror("execlp ls");
        exit(1);
    }

    // Create second child
    child2 = fork();

    if (child2 == -1)
    {
        perror("fork");
        return 1;
    }

    if (child2 == 0)
    {
        // Child 2: wc -l

        // Close unused write end
        close(fd[1]);

        // Redirect stdin from pipe
        dup2(fd[0], STDIN_FILENO);

        // Close original descriptor
        close(fd[0]);

        // Execute wc -l
        execlp("wc", "wc", "-l", NULL);

        perror("execlp wc");
        exit(1);
    }

    // Parent closes both pipe descriptors
    close(fd[0]);
    close(fd[1]);

    // Wait for both children
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    printf("Pipeline execution completed.\n");

    return 0;
}
