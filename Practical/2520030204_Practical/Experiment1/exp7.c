#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_var = 100;          // Global initialized variable
int global_uninit;             // Global uninitialized variable

static int static_var = 200;   // Static initialized variable
static int static_uninit;      // Static uninitialized variable

void code_function()
{
    printf("This is a function.\n");
}

int main()
{
    int stack_var = 300;       // Stack variable

    int *heap_var = malloc(sizeof(int));  // Heap variable

    if (heap_var == NULL)
    {
        printf("Heap allocation failed.\n");
        return 1;
    }

    *heap_var = 400;

    printf("============================================\n");
    printf("       LINUX PROCESS MEMORY LAYOUT\n");
    printf("============================================\n");

    printf("\nProcess ID (PID): %d\n", getpid());

    printf("\n--- Memory Addresses ---\n");

    // Code segment
    printf("Code / Function address       : %p\n",
           (void *)code_function);

    // Global variables
    printf("Global variable address       : %p\n",
           (void *)&global_var);

    printf("Global uninitialized address  : %p\n",
           (void *)&global_uninit);

    // Static variables
    printf("Static variable address       : %p\n",
           (void *)&static_var);

    printf("Static uninitialized address  : %p\n",
           (void *)&static_uninit);

    // Heap
    printf("Heap variable address         : %p\n",
           (void *)heap_var);

    // Stack
    printf("Stack variable address        : %p\n",
           (void *)&stack_var);

    printf("\n============================================\n");

    printf("\nProcess is running...\n");
    printf("Use another terminal to inspect:\n");
    printf("cat /proc/%d/maps\n", getpid());

    printf("\nPress ENTER to terminate...\n");
    getchar();

    free(heap_var);

    return 0;
}
