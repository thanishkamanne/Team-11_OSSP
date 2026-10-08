#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int fd;
    int saved_stdin;

    // Open input file
    fd = open("input.txt", O_RDONLY);

    if (fd == -1)
    {
        perror("Error opening input.txt");
        return 1;
    }

    // Save original stdin
    saved_stdin = dup(STDIN_FILENO);

    if (saved_stdin == -1)
    {
        perror("dup failed");
        close(fd);
        return 1;
    }

    // Redirect stdin to input.txt
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("dup2 failed");
        close(fd);
        close(saved_stdin);
        return 1;
    }

    close(fd);

    printf("Reading from input.txt:\n");

    // Read from redirected stdin
    char buffer[100];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        printf("%s", buffer);
    }

    // Restore original stdin
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);

    printf("\nInput redirection completed.\n");

    return 0;
}
