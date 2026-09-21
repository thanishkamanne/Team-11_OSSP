#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 1000

// Function to process escape sequences
void processEscapes(char *input)
{
    char output[MAX];
    int i = 0, j = 0;

    while (input[i] != '\0' && j < MAX - 1)
    {
        if (input[i] == '\\')
        {
            i++;

            if (input[i] == 'n')
            {
                output[j++] = '\n';
            }
            else if (input[i] == 't')
            {
                output[j++] = '\t';
            }
            else if (input[i] == '\\')
            {
                output[j++] = '\\';
            }
            else if (input[i] == ' ')
            {
                output[j++] = ' ';
            }
            else if (input[i] == '"')
            {
                output[j++] = '"';
            }
            else if (input[i] == '\'')
            {
                output[j++] = '\'';
            }
            else
            {
                // Preserve unknown escape characters
                output[j++] = '\\';
                output[j++] = input[i];
            }
        }
        else
        {
            output[j++] = input[i];
        }

        i++;
    }

    output[j] = '\0';

    printf("\nParsed Output:\n");
    printf("%s\n", output);
}

// Function to validate the input
int validateInput(char *input)
{
    int i;

    for (i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == '\\')
        {
            if (input[i + 1] == '\0')
            {
                printf("\nInvalid Input: Escape character at the end.\n");
                return 0;
            }
        }
    }

    printf("\nInput validation successful.\n");
    return 1;
}

int main()
{
    char input[MAX];

    printf("====================================\n");
    printf("     PROCESS ESCAPE SEQUENCE PARSER\n");
    printf("====================================\n");

    printf("\nEnter a command/string:\n");
    fgets(input, MAX, stdin);

    // Remove newline added by fgets
    input[strcspn(input, "\n")] = '\0';

    printf("\nOriginal Input:\n%s\n", input);

    if (validateInput(input))
    {
        processEscapes(input);
    }

    return 0;
}
