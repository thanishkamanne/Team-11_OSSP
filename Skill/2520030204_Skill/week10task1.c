#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int fd;
    int saved_stdout;
    int saved_stderr;

    /* Open output file */
    fd = open("combined_output.txt",
              O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    /* Save original stdout and stderr */
    saved_stdout = dup(STDOUT_FILENO);
    saved_stderr = dup(STDERR_FILENO);

    if (saved_stdout < 0 || saved_stderr < 0)
    {
        perror("dup");
        close(fd);
        return 1;
    }

    /*
     * Redirect stdout to the file
     */
    dup2(fd, STDOUT_FILENO);

    /*
     * Redirect stderr to the same file
     */
    dup2(fd, STDERR_FILENO);

    close(fd);

    printf("Message from standard output.\n");
    fflush(stdout);

    fprintf(stderr, "Message from standard error.\n");
    fflush(stderr);

    printf("Another stdout message.\n");
    fflush(stdout);

    fprintf(stderr, "Another stderr message.\n");
    fflush(stderr);

    /* Restore stdout and stderr */
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stderr, STDERR_FILENO);

    close(saved_stdout);
    close(saved_stderr);

    printf("Redirection completed successfully.\n");

    return 0;
}
