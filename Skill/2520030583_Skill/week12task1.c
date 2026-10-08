#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

/* Signal handler */
void signal_handler(int sig, siginfo_t *info, void *context)
{
    printf("\nSignal received!\n");
    printf("Signal number: %d\n", sig);
    printf("Sender PID: %d\n", info->si_pid);

    if (sig == SIGINT)
    {
        printf("SIGINT received. Process will continue running.\n");
    }
    else if (sig == SIGTERM)
    {
        printf("SIGTERM received. Process is terminating.\n");
        exit(0);
    }
}

int main()
{
    struct sigaction action;

    printf("Signal Action Demonstration\n");
    printf("Process PID: %d\n", getpid());

    /* Clear structure */
    sigemptyset(&action.sa_mask);

    /* Use SA_SIGINFO to receive signal context */
    action.sa_flags = SA_SIGINFO;

    /* Register signal handler */
    action.sa_sigaction = signal_handler;

    /*
     * Register handler for SIGINT
     */
    if (sigaction(SIGINT, &action, NULL) < 0)
    {
        perror("sigaction");
        return 1;
    }

    /*
     * Register handler for SIGTERM
     */
    if (sigaction(SIGTERM, &action, NULL) < 0)
    {
        perror("sigaction");
        return 1;
    }

    printf("\nSignal handlers registered.\n");
    printf("Press Ctrl+C to generate SIGINT.\n");
    printf("Program will remain active.\n\n");

    while (1)
    {
        printf("Process is running...\n");
        sleep(3);
    }

    return 0;
}
