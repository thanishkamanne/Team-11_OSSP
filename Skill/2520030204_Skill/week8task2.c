#include <stdio.h>
#include <string.h>

#define MAX_COMMANDS 5
#define MAX_LENGTH 100

struct Pipeline
{
    char commands[MAX_COMMANDS][MAX_LENGTH];
    int count;
};

/* Add command to pipeline */
void add_command(struct Pipeline *p, char command[])
{
    if (p->count >= MAX_COMMANDS)
    {
        printf("Pipeline capacity exceeded.\n");
        return;
    }

    strcpy(p->commands[p->count], command);
    p->count++;
}

/* Display pipeline */
void display_pipeline(struct Pipeline *p)
{
    printf("\nPipeline Structure:\n");

    for (int i = 0; i < p->count; i++)
    {
        printf("[%d] %s", i + 1, p->commands[i]);

        if (i < p->count - 1)
        {
            printf("  -->  ");
        }
    }

    printf("\n");
}

/* Validate pipeline */
void validate_pipeline(struct Pipeline *p)
{
    if (p->count == 0)
    {
        printf("Pipeline is empty.\n");
    }
    else if (p->count == 1)
    {
        printf("Valid: Single command pipeline.\n");
    }
    else
    {
        printf("Valid: Pipeline contains %d commands.\n",
               p->count);
    }
}

int main()
{
    struct Pipeline pipeline;
    pipeline.count = 0;

    char command[MAX_LENGTH];

    printf("Enter pipeline commands.\n");
    printf("Type 'done' when finished.\n\n");

    while (pipeline.count < MAX_COMMANDS)
    {
        printf("Command %d: ", pipeline.count + 1);

        fgets(command, MAX_LENGTH, stdin);

        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "done") == 0)
        {
            break;
        }

        add_command(&pipeline, command);
    }

    display_pipeline(&pipeline);

    validate_pipeline(&pipeline);

    return 0;
}
