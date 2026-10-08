#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void handle_signal(int signal_number)
{
    if (signal_number == SIGINT)
    {
        printf("\nSIGINT received.\n");
        printf("Ctrl+C was pressed.\n");
    }
    else if (signal_number == SIGTERM)
    {
        printf("\nSIGTERM received.\n");
    }
}

int main()
{
    sigset_t set;

    printf("Signal Demonstration\n");
    printf("PID  = %d\n", getpid());
    printf("PGID = %d\n", getpgrp());

    /*
     * Install signal handlers
     */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /*
     * Create signal set
     */
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    /*
     * Block SIGINT
     */
    sigprocmask(SIG_BLOCK, &set, NULL);

    printf("\nSIGINT is now blocked.\n");
    printf("Press Ctrl+C now.\n");

    sleep(5);

    /*
     * Unblock SIGINT
     */
    printf("\nUnblocking SIGINT...\n");

    sigprocmask(SIG_UNBLOCK, &set, NULL);

    printf("SIGINT is unblocked.\n");
    printf("Press Ctrl+C again or wait.\n");

    sleep(5);

    printf("\nProgram completed.\n");

    return 0;
}
