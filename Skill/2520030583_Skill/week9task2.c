#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    int saved_stdout;

    // Open output file
    // O_TRUNC removes previous contents
    fd = open("output.txt",
              O_WRONLY | O_CREAT | O_TRUNC,
              0644);

    if (fd == -1)
    {
        perror("Error opening output.txt");
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

    // Redirect stdout to output.txt
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 failed");
        close(fd);
        close(saved_stdout);
        return 1;
    }

    close(fd);

    // This goes into output.txt
    printf("This message is redirected to the output file.\n");
    printf("Standard output is currently redirected.\n");

    fflush(stdout);

    // Restore stdout
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    printf("Output redirection completed.\n");

    return 0;
}
