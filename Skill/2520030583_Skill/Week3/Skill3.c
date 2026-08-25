#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 10
#define SIZE 100

struct Node
{
    int data;
    struct Node *next;
};

int main()
{
    /* Command History */
    char history[MAX][SIZE];
    char command[SIZE];
    int count = 0;

    printf("=== Command History ===\n");

    while (count < MAX)
    {
        printf("Enter command (type exit to stop): ");
        fgets(command, SIZE, stdin);

        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0)
            break;

        strcpy(history[count], command);
        count++;

        printf("Command stored: %s\n", command);

        if (count > 1)
            printf("Previous command: %s\n", history[count - 2]);
        else
            printf("Previous command: None\n");

        printf("Next command: None\n");
    }

    printf("\n=== Command History List ===\n");

    for (int i = 0; i < count; i++)
        printf("%d. %s\n", i + 1, history[i]);

    /* Dynamic Array */
    printf("\n=== Dynamic Array ===\n");

    int size = 2;
    int n = 0;

    int *arr = malloc(size * sizeof(int));

    for (int i = 1; i <= 5; i++)
    {
        if (n == size)
        {
            size *= 2;
            arr = realloc(arr, size * sizeof(int));
        }

        arr[n] = i * 10;
        n++;
    }

    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);

    printf("\n");

    /* Linked List */
    printf("\n=== Linked List ===\n");

    struct Node *head = NULL;
    struct Node *temp;

    for (int i = 1; i <= 3; i++)
    {
        temp = malloc(sizeof(struct Node));

        temp->data = i * 100;
        temp->next = head;
        head = temp;
    }

    temp = head;

    while (temp != NULL)
    {
        printf("%d ", temp->data);
        temp = temp->next;
    }

    printf("\n");

    /* Release Linked List Memory */
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }

    /* Release Array Memory */
    free(arr);

    printf("\nMemory released successfully.\n");

    return 0;
}
