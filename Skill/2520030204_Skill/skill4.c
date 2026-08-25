#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 1024
#define INITIAL_TOKENS 10

// Token structure
typedef struct Token {
    char *value;
    struct Token *next;
} Token;

// Execution structure
typedef struct Command {
    char *program;
    char **arguments;
    int argumentCount;
} Command;

// Create a token
Token *createToken(const char *value)
{
    Token *newToken;

    newToken = malloc(sizeof(Token));

    if (newToken == NULL)
    {
        return NULL;
    }

    newToken->value = malloc(strlen(value) + 1);

    if (newToken->value == NULL)
    {
        free(newToken);
        return NULL;
    }

    strcpy(newToken->value, value);
    newToken->next = NULL;

    return newToken;
}

// Add token to linked list
void addToken(Token **head, const char *value)
{
    Token *newToken = createToken(value);

    if (newToken == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    if (*head == NULL)
    {
        *head = newToken;
    }
    else
    {
        Token *temp = *head;

        while (temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = newToken;
    }
}

// Tokenize input
Token *tokenize(char *input)
{
    Token *head = NULL;

    char buffer[MAX_INPUT];
    int index = 0;

    for (int i = 0; ; i++)
    {
        char c = input[i];

        // End of input
        if (c == '\0')
        {
            if (index > 0)
            {
                buffer[index] = '\0';
                addToken(&head, buffer);
            }

            break;
        }

        // Whitespace delimiter
        if (isspace(c))
        {
            if (index > 0)
            {
                buffer[index] = '\0';
                addToken(&head, buffer);
                index = 0;
            }
        }

        // Special shell operators
        else if (c == '|' || c == '<' || c == '>')
        {
            if (index > 0)
            {
                buffer[index] = '\0';
                addToken(&head, buffer);
                index = 0;
            }

            buffer[0] = c;
            buffer[1] = '\0';

            addToken(&head, buffer);
        }

        else
        {
            if (index < MAX_INPUT - 1)
            {
                buffer[index++] = c;
            }
        }
    }

    return head;
}

// Display tokens
void displayTokens(Token *head)
{
    printf("\n----- Tokens -----\n");

    int count = 1;

    while (head != NULL)
    {
        printf("Token %d: [%s]\n", count, head->value);

        head = head->next;
        count++;
    }

    printf("------------------\n");
}

// Validate tokens
int validateTokens(Token *head)
{
    if (head == NULL)
    {
        printf("Error: Empty command!\n");
        return 0;
    }

    Token *temp = head;

    while (temp != NULL)
    {
        // Pipe cannot be first
        if (strcmp(temp->value, "|") == 0)
        {
            if (temp == head)
            {
                printf("Syntax Error: Pipe cannot be first.\n");
                return 0;
            }

            // Pipe cannot be last
            if (temp->next == NULL)
            {
                printf("Syntax Error: Pipe cannot be last.\n");
                return 0;
            }

            // Two pipes cannot be together
            if (strcmp(temp->next->value, "|") == 0)
            {
                printf("Syntax Error: Consecutive pipes.\n");
                return 0;
            }
        }

        temp = temp->next;
    }

    return 1;
}

// Create execution structure
Command *createCommand(Token *head)
{
    Command *cmd;

    cmd = malloc(sizeof(Command));

    if (cmd == NULL)
    {
        return NULL;
    }

    cmd->argumentCount = 0;

    // Count arguments
    Token *temp = head;

    while (temp != NULL && strcmp(temp->value, "|") != 0)
    {
        cmd->argumentCount++;
        temp = temp->next;
    }

    cmd->arguments = malloc(
        sizeof(char *) * (cmd->argumentCount + 1)
    );

    if (cmd->arguments == NULL)
    {
        free(cmd);
        return NULL;
    }

    temp = head;

    int i = 0;

    while (temp != NULL &&
           strcmp(temp->value, "|") != 0)
    {
        cmd->arguments[i] = malloc(
            strlen(temp->value) + 1
        );

        strcpy(cmd->arguments[i], temp->value);

        i++;
        temp = temp->next;
    }

    cmd->arguments[i] = NULL;

    if (cmd->argumentCount > 0)
    {
        cmd->program = cmd->arguments[0];
    }
    else
    {
        cmd->program = NULL;
    }

    return cmd;
}

// Display execution structure
void displayCommand(Command *cmd)
{
    printf("\n----- Execution Structure -----\n");

    if (cmd->program != NULL)
    {
        printf("Program: %s\n", cmd->program);

        printf("Arguments:\n");

        for (int i = 0; i < cmd->argumentCount; i++)
        {
            printf("  argv[%d] = %s\n",
                   i,
                   cmd->arguments[i]);
        }
    }

    printf("--------------------------------\n");
}

// Free tokens
void freeTokens(Token *head)
{
    Token *temp;

    while (head != NULL)
    {
        temp = head;
        head = head->next;

        free(temp->value);
        free(temp);
    }
}

// Free command structure
void freeCommand(Command *cmd)
{
    if (cmd == NULL)
    {
        return;
    }

    for (int i = 0; i < cmd->argumentCount; i++)
    {
        free(cmd->arguments[i]);
    }

    free(cmd->arguments);
    free(cmd);
}

int main()
{
    char input[MAX_INPUT];

    printf("Simple Shell Tokenizer and Parser\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        printf("shell> ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        // Exit
        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        // Tokenize
        Token *tokens = tokenize(input);

        // Display tokens
        displayTokens(tokens);

        // Validate
        if (!validateTokens(tokens))
        {
            freeTokens(tokens);
            continue;
        }

        printf("Syntax is valid.\n");

        // Create execution structure
        Command *cmd = createCommand(tokens);

        if (cmd != NULL)
        {
            displayCommand(cmd);
            freeCommand(cmd);
        }

        // Free token memory
        freeTokens(tokens);
    }

    return 0;
}
