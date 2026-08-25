#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 5
#define MAX_INPUT 1024

// Node for command history
typedef struct HistoryNode {
    char *command;
    struct HistoryNode *next;
} HistoryNode;

// Add command to history
void addHistory(HistoryNode **head, const char *command)
{
    HistoryNode *newNode;

    newNode = (HistoryNode *)malloc(sizeof(HistoryNode));

    if (newNode == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    newNode->command = malloc(strlen(command) + 1);

    if (newNode->command == NULL)
    {
        free(newNode);
        printf("Memory allocation failed!\n");
        return;
    }

    strcpy(newNode->command, command);
    newNode->next = NULL;

    if (*head == NULL)
    {
        *head = newNode;
    }
    else
    {
        HistoryNode *temp = *head;

        while (temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = newNode;
    }
}

// Display history
void displayHistory(HistoryNode *head)
{
    int count = 1;

    printf("\n----- Command History -----\n");

    while (head != NULL)
    {
        printf("%d  %s\n", count, head->command);
        head = head->next;
        count++;
    }

    printf("----------------------------\n");
}

// Free history memory
void freeHistory(HistoryNode *head)
{
    HistoryNode *temp;

    while (head != NULL)
    {
        temp = head;
        head = head->next;

        free(temp->command);
        free(temp);
    }
}

int main()
{
    char *input;
    int bufferSize = INITIAL_SIZE;
    int length = 0;
    int ch;

    HistoryNode *history = NULL;

    // Dynamically allocate input buffer
    input = malloc(bufferSize);

    if (input == NULL)
    {
        printf("Memory allocation failed!\n");
        return 1;
    }

    printf("Simple Command History\n");
    printf("Type 'history' to display history\n");
    printf("Type 'exit' to quit\n\n");

    while (1)
    {
        printf("shell> ");

        length = 0;

        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            // Resize buffer when necessary
            if (length + 1 >= bufferSize)
            {
                bufferSize *= 2;

                input = realloc(input, bufferSize);

                if (input == NULL)
                {
                    printf("Memory allocation failed!\n");
                    return 1;
                }
            }

            input[length++] = ch;
        }

        input[length] = '\0';

        // Empty command
        if (length == 0)
        {
            continue;
        }

        // Exit
        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        // Display history
        if (strcmp(input, "history") == 0)
        {
            displayHistory(history);
            continue;
        }

        // Store command
        addHistory(&history, input);

        printf("Command stored: %s\n", input);
    }

    // Release dynamically allocated memory
    free(input);
    freeHistory(history);

    printf("\nMemory released successfully.\n");

    return 0;
}
