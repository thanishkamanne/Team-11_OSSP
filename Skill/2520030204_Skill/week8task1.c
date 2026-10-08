#include <stdio.h>
#include <string.h>

#define MAX_HISTORY 5
#define MAX_COMMAND 100

char history[MAX_HISTORY][MAX_COMMAND];
int count = 0;

/* Add a command to history */
void add_command(char command[])
{
    // If buffer is full, remove the oldest command
    if (count == MAX_HISTORY)
    {
        for (int i = 1; i < MAX_HISTORY; i++)
        {
            strcpy(history[i - 1], history[i]);
        }

        count--;
    }

    strcpy(history[count], command);
    count++;
}

/* Display history */
void display_history()
{
    printf("\nCommand History:\n");

    for (int i = 0; i < count; i++)
    {
        printf("%d  %s\n", i + 1, history[i]);
    }
}

/* Retrieve a command */
void retrieve_command(int number)
{
    if (number < 1 || number > count)
    {
        printf("Invalid history number.\n");
        return;
    }

    printf("Command %d: %s\n", number, history[number - 1]);
}

int main()
{
    char command[MAX_COMMAND];
    int choice;
    int number;

    while (1)
    {
        printf("\n1. Add Command");
        printf("\n2. Display History");
        printf("\n3. Retrieve Command");
        printf("\n4. Exit");

        printf("\nEnter choice: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 1)
        {
            printf("Enter command: ");
            fgets(command, MAX_COMMAND, stdin);

            command[strcspn(command, "\n")] = '\0';

            add_command(command);
            printf("Command stored successfully.\n");
        }
        else if (choice == 2)
        {
            display_history();
        }
        else if (choice == 3)
        {
            printf("Enter history number: ");
            scanf("%d", &number);

            retrieve_command(number);
        }
        else if (choice == 4)
        {
            break;
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
