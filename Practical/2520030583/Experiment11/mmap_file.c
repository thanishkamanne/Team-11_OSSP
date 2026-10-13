#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

int main()
{
    int fd;
    struct stat file_info;
    char *mapped_file;

    // Open the file for reading and writing
    fd = open("mmap_data.txt", O_RDWR);

    if (fd == -1)
    {
        perror("Error opening file");
        return 1;
    }

    // Get file information
    if (fstat(fd, &file_info) == -1)
    {
        perror("fstat failed");
        close(fd);
        return 1;
    }

    // Map the file into memory
    mapped_file = mmap(NULL,
                       file_info.st_size,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED,
                       fd,
                       0);

    if (mapped_file == MAP_FAILED)
    {
        perror("mmap failed");
        close(fd);
        return 1;
    }

    // Read file contents using memory mapping
    printf("Original file contents:\n");
    printf("%.*s\n", (int)file_info.st_size, mapped_file);

    // Modify the file through memory
    strcpy(mapped_file, "Modified using mmap()!\n");

    // Make changes available to the file
    msync(mapped_file, file_info.st_size, MS_SYNC);

    // Remove mapping
    munmap(mapped_file, file_info.st_size);

    // Close file
    close(fd);

    printf("File modified successfully using mmap().\n");

    return 0;
}
