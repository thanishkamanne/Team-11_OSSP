#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        printf("Fork failed!\n");
    }
    else if (pid == 0) {
        printf("Child process created.\n");
        printf("Child PID = %d\n", getpid());

        execlp("ls", "ls", "-l", NULL);

        printf("exec failed!\n");
    }
    else {
        printf("Parent process.\n");
        printf("Parent PID = %d\n", getpid());
        printf("Child PID = %d\n", pid);
    }

    return 0;
}

