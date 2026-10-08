#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
    pid_t child_pid;
    pid_t shell_pgid;

    shell_pgid = getpgrp();

    printf("Shell PID  : %d\n", getpid());
    printf("Shell PGID : %d\n", shell_pgid);

    child_pid = fork();

    if (child_pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (child_pid == 0)
    {
        /*
         * Child creates a new process group
         */
        setpgid(0, 0);

        printf("\nChild process created.\n");
        printf("Child PID  : %d\n", getpid());
        printf("Child PGID : %d\n", getpgrp());

        printf("Child process is running...\n");

        sleep(5);

        printf("Child process completed.\n");

        exit(0);
    }
    else
    {
        /*
         * Parent also ensures child has its own group
         */
        setpgid(child_pid, child_pid);

        printf("\nNew process group created.\n");
        printf("Child PID  : %d\n", child_pid);
        printf("Child PGID : %d\n", child_pid);

        /*
         * Demonstrate signal to process group
         */
        sleep(1);

        printf("\nSending SIGCONT to child process group...\n");

        kill(-child_pid, SIGCONT);

        printf("Signal sent to process group.\n");

        /*
         * Wait for child
         */
        waitpid(child_pid, NULL, 0);

        printf("\nChild process group completed.\n");
        printf("Terminal control can now return to the shell.\n");
    }

    return 0;
}
