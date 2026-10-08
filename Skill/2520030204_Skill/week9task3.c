#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    int saved_stdout;

    // Open file in append mode
    fd = open("append.txt",
              O_WRONLY | O_CREAT | O_APPEND,
              0644);

    if (fd == -1)
    {
        perror("Error opening append.txt");
        return 1;
    }

    // Save original stdout
    saved_stdout = dup(STDOUT_FILENO);

    if (saved_stdout == -1)
    {
        perror("dup failed");
        close(fd);
        return 1;
    }

    // Redirect stdout to append.txt
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 failed");
        close(fd);
        close(saved_stdout);
        return 1;
    }

    close(fd);

    // These messages are appended
    printf("New line added to the file.\n");
    printf("Another line added using append mode.\n");

    fflush(stdout);

    // Restore stdout
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    printf("Append operation completed.\n");

    return 0;
}
