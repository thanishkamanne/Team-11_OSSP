#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

pid_t child_pid;

/* Handle SIGTSTP */
void handle_sigtstp(int sig)
{
    printf("\nShell received SIGTSTP.\n");

    if (child_pid > 0)
    {
        printf("Sending SIGSTOP to foreground job...\n");

        /*
         * Stop the child
         */
        kill(child_pid, SIGSTOP);
    }
}

int main()
{
    int status;

    /*
     * Install SIGTSTP handler
     */
    signal(SIGTSTP, handle_sigtstp);

    printf("SIGTSTP Job Control Demonstration\n");
    printf("Shell PID: %d\n", getpid());

    child_pid = fork();

    if (child_pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (child_pid == 0)
    {
        /*
         * Child process
         */
        printf("Foreground process started.\n");
        printf("PID: %d\n", getpid());

        while (1)
        {
            printf("Process is running...\n");
            sleep(2);
        }
    }
    else
    {
        printf("Foreground job PID = %d\n", child_pid);
        printf("Press Ctrl+Z to stop the job.\n");

        /*
         * Wait for child state change
         */
        waitpid(child_pid, &status, WUNTRACED);

        if (WIFSTOPPED(status))
        {
            printf("\nJob has been stopped.\n");
            printf("Job table state: STOPPED\n");

            printf("\nTo resume the job later, use SIGCONT.\n");

            /*
             * Resume child
             */
            kill(child_pid, SIGCONT);

            printf("SIGCONT sent.\n");
            printf("Job table state: RUNNING\n");

            sleep(3);

            /*
             * Terminate child after demonstration
             */
            kill(child_pid, SIGTERM);

            waitpid(child_pid, &status, 0);

            printf("Job demonstration completed.\n");
        }
    }

    return 0;
}
