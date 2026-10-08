#include <stdio.h>
#include <string.h>

int main()
{
    char command[200];

    printf("Enter a command: ");
    fgets(command, sizeof(command), stdin);

    /* Remove newline */
    command[strcspn(command, "\n")] = '\0';

    printf("\nCommand entered: %s\n", command);

    /* Basic syntax checking */

    if (strlen(command) == 0)
    {
        printf("Error: Empty command.\n");
        return 1;
    }

    if (strstr(command, "||") != NULL)
    {
        printf("Error: Invalid pipe syntax.\n");
        return 1;
    }

    if (strstr(command, "|") != NULL)
    {
        printf("Pipe detected.\n");
    }

    if (strstr(command, ">") != NULL)
    {
        printf("Output redirection detected.\n");
    }

    if (strstr(command, "<") != NULL)
    {
        printf("Input redirection detected.\n");
    }

    if (strstr(command, ">>") != NULL)
    {
        printf("Append redirection detected.\n");
    }

    printf("\nExecution Plan:\n");

    if (strstr(command, "|") != NULL)
    {
        printf("1. Create pipe(s).\n");
        printf("2. Create child processes.\n");
        printf("3. Connect processes using dup2().\n");
    }

    if (strstr(command, "<") != NULL)
    {
        printf("4. Redirect standard input using dup2().\n");
    }

    if (strstr(command, ">") != NULL)
    {
        printf("5. Redirect standard output using dup2().\n");
    }

    if (strstr(command, ">>") != NULL)
    {
        printf("6. Open output file in append mode.\n");
    }

    printf("7. Execute commands using exec().\n");

    return 0;
}
