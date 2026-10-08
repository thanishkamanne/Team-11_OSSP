#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    int saved_stderr;

    // Open error output file
    fd = open("errors.txt",
              O_WRONLY | O_CREAT | O_TRUNC,
              0644);

    if (fd == -1)
    {
        perror("Error opening errors.txt");
        return 1;
    }

    // Save original stderr
    saved_stderr = dup(STDERR_FILENO);

    if (saved_stderr == -1)
    {
        perror("dup failed");
        close(fd);
        return 1;
    }

    // Redirect stderr to errors.txt
    if (dup2(fd, STDERR_FILENO) == -1)
    {
        perror("dup2 failed");
        close(fd);
        close(saved_stderr);
        return 1;
    }

    close(fd);

    // These errors go into errors.txt
    fprintf(stderr, "Error: File not found.\n");
    fprintf(stderr, "Error: Unable to open requested resource.\n");

    fflush(stderr);

    // Restore stderr
    dup2(saved_stderr, STDERR_FILENO);
    close(saved_stderr);

    printf("stderr redirection completed.\n");

    return 0;
}
