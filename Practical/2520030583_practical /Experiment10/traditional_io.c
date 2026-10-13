#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd;
    char buffer[100];

    // Open file for reading and writing
    fd = open("traditional_data.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (fd == -1)
    {
        perror("open failed");
        return 1;
    }

    // Write data using write()
    char message[] = "Original data using read/write!\n";

    write(fd, message, strlen(message));

    // Move file pointer to beginning
    lseek(fd, 0, SEEK_SET);

    // Read data using read()
    int bytes = read(fd, buffer, sizeof(buffer) - 1);

    buffer[bytes] = '\0';

    printf("File contents:\n");
    printf("%s", buffer);

    close(fd);

    return 0;
}
