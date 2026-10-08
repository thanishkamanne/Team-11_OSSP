#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    FILE *source, *destination;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    if (argc != 3)
    {
        printf("Usage: %s <source_file> <destination_file>\n", argv[0]);
        return 1;
    }

    // Open source file
    source = fopen(argv[1], "rb");

    if (source == NULL)
    {
        perror("Error opening source file");
        return 1;
    }

    // Open destination file
    destination = fopen(argv[2], "wb");

    if (destination == NULL)
    {
        perror("Error opening destination file");
        fclose(source);
        return 1;
    }

    // Copy using fread() and fwrite()
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, source)) > 0)
    {
        fwrite(buffer, 1, bytes_read, destination);
    }

    fclose(source);
    fclose(destination);

    printf("File copied successfully using standard library functions!\n");

    return 0;
}
