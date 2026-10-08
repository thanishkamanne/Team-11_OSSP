#include <stdio.h>
#include <string.h>

#define MAX_JOBS 5

struct Job
{
    int job_id;
    int process_group;
    char command[50];
    char state[20];
};

struct Job jobs[MAX_JOBS];
int job_count = 0;

/* Add a job */
void add_job(int pgid, char command[])
{
    if (job_count >= MAX_JOBS)
    {
        printf("Job table is full.\n");
        return;
    }

    jobs[job_count].job_id = job_count + 1;
    jobs[job_count].process_group = pgid;

    strcpy(jobs[job_count].command, command);
    strcpy(jobs[job_count].state, "Running");

    job_count++;

    printf("Job added successfully.\n");
}

/* Display jobs */
void display_jobs()
{
    int i;

    printf("\nJob Table\n");
    printf("---------------------------------------------\n");
    printf("ID\tPGID\tCommand\t\tState\n");
    printf("---------------------------------------------\n");

    for (i = 0; i < job_count; i++)
    {
        printf("%d\t%d\t%-15s\t%s\n",
               jobs[i].job_id,
               jobs[i].process_group,
               jobs[i].command,
               jobs[i].state);
    }
}

/* Update job state */
void update_job(int id, char new_state[])
{
    int i;

    for (i = 0; i < job_count; i++)
    {
        if (jobs[i].job_id == id)
        {
            strcpy(jobs[i].state, new_state);
            printf("Job %d updated to %s.\n",
                   id, new_state);
            return;
        }
    }

    printf("Job not found.\n");
}

/* Remove completed job */
void remove_job(int id)
{
    int i, j;

    for (i = 0; i < job_count; i++)
    {
        if (jobs[i].job_id == id)
        {
            for (j = i; j < job_count - 1; j++)
            {
                jobs[j] = jobs[j + 1];
            }

            job_count--;

            printf("Job %d removed.\n", id);
            return;
        }
    }

    printf("Job not found.\n");
}

int main()
{
    /* Add jobs */
    add_job(1001, "sleep10");
    add_job(1002, "programA");
    add_job(1003, "programB");

    display_jobs();

    /* Update a job */
    update_job(2, "Stopped");

    display_jobs();

    /* Remove completed job */
    remove_job(1);

    display_jobs();

    return 0;
}
