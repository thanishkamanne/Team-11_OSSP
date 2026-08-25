#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 100

struct Token
{
    char value[20];
};

struct Node
{
    char value[20];
    struct Node *left;
    struct Node *right;
};

int main()
{
    char input[MAX];
    struct Token tokens[20];
    int count = 0;

    printf("Enter command: ");
    fgets(input, MAX, stdin);

    input[strcspn(input, "\n")] = '\0';

    /* Handle empty command */
    if (strlen(input) == 0)
    {
        printf("Empty command!\n");
        return 0;
    }

    /* Split input into tokens */
    char *token = strtok(input, " \t");

    while (token != NULL && count < 20)
    {
        strcpy(tokens[count].value, token);
        count++;
        token = strtok(NULL, " \t");
    }

    /* Display tokens */
    printf("\nTokens:\n");

    for (int i = 0; i < count; i++)
    {
        printf("Token %d: %s\n", i + 1, tokens[i].value);
    }

    /* Validate token stream */
    if (count == 0)
    {
        printf("Invalid token stream.\n");
        return 0;
    }

    printf("\nToken stream is valid.\n");

    /* Simple parser */
    struct Node *root = malloc(sizeof(struct Node));
    strcpy(root->value, tokens[0].value);
    root->left = NULL;
    root->right = NULL;

    if (count > 1)
    {
        root->right = malloc(sizeof(struct Node));
        strcpy(root->right->value, tokens[1].value);
        root->right->left = NULL;
        root->right->right = NULL;
    }

    /* Parse tree */
    printf("\nParse Tree:\n");
    printf("Root: %s\n", root->value);

    if (root->right != NULL)
        printf("  -> %s\n", root->right->value);

    /* Execution structure */
    printf("\nExecution Structure:\n");

    for (int i = 0; i < count; i++)
        printf("[%s] ", tokens[i].value);

    printf("\n");

    free(root->right);
    free(root);

    return 0;
}
