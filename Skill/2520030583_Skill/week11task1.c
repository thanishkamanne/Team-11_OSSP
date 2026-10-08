#include <stdio.h>
#include <string.h>

#define MAX_JOBS 5

struct Job
{
    int job_id;
    int pid;
    char command[50];
    char state[20];
};

struct Job jobs[MAX_JOBS] =
{
    {1, 1234, "sleep 20", "Running"},
    {2, 1235, "vim file.txt", "Stopped"},
    {3, 1236, "programA", "Running"}
};

int job_count = 3;

/* Display all active jobs */
void list_jobs()
{
    int i;

    printf("\nActive Jobs\n");
    printf("------------------------------------------------\n");
    printf("Job ID\tPID\tCommand\t\tState\n");
    printf("------------------------------------------------\n");

    for (i = 0; i < job_count; i++)
    {
        printf("[%d]\t%d\t%-15s\t%s\n",
               jobs[i].job_id,
               jobs[i].pid,
               jobs[i].command,
               jobs[i].state);
    }

    printf("------------------------------------------------\n");
}

/* Update job state */
void update_state(int job_id, char new_state[])
{
    int i;

    for (i = 0; i < job_count; i++)
    {
        if (jobs[i].job_id == job_id)
        {
            strcpy(jobs[i].state, new_state);

            printf("\nJob [%d] state updated to %s.\n",
                   job_id, new_state);

            return;
        }
    }

    printf("\nJob [%d] not found.\n", job_id);
}

int main()
{
    printf("Shell Job Listing Demonstration\n");

    list_jobs();

    /* Update a job */
    update_state(2, "Running");

    printf("\nAfter updating job state:\n");

    list_jobs();

    return 0;
}
