#include <stdio.h>
#include <stdlib.h>

int main()
{
    int i;

    printf("========================================\n");
    printf("   DYNAMIC MEMORY ALLOCATION DEMO\n");
    printf("========================================\n");

    // ------------------------------------------------
    // 1. malloc()
    // ------------------------------------------------

    printf("\n1. Using malloc()\n");

    int *arr1 = (int *)malloc(5 * sizeof(int));

    if (arr1 == NULL)
    {
        printf("malloc failed.\n");
        return 1;
    }

    for (i = 0; i < 5; i++)
    {
        arr1[i] = (i + 1) * 10;
    }

    printf("malloc values: ");

    for (i = 0; i < 5; i++)
    {
        printf("%d ", arr1[i]);
    }

    printf("\n");

    // ------------------------------------------------
    // 2. calloc()
    // ------------------------------------------------

    printf("\n2. Using calloc()\n");

    int *arr2 = (int *)calloc(5, sizeof(int));

    if (arr2 == NULL)
    {
        printf("calloc failed.\n");
        free(arr1);
        return 1;
    }

    printf("Initial calloc values: ");

    for (i = 0; i < 5; i++)
    {
        printf("%d ", arr2[i]);
    }

    printf("\n");

    // Store values
    for (i = 0; i < 5; i++)
    {
        arr2[i] = (i + 1) * 100;
    }

    printf("After storing values: ");

    for (i = 0; i < 5; i++)
    {
        printf("%d ", arr2[i]);
    }

    printf("\n");

    // ------------------------------------------------
    // 3. realloc()
    // ------------------------------------------------

    printf("\n3. Using realloc()\n");

    int *temp = (int *)realloc(arr1, 10 * sizeof(int));

    if (temp == NULL)
    {
        printf("realloc failed.\n");
        free(arr1);
        free(arr2);
        return 1;
    }

    arr1 = temp;

    // Add new values
    for (i = 5; i < 10; i++)
    {
        arr1[i] = (i + 1) * 10;
    }

    printf("After reallocating to 10 integers: ");

    for (i = 0; i < 10; i++)
    {
        printf("%d ", arr1[i]);
    }

    printf("\n");

    // ------------------------------------------------
    // 4. free()
    // ------------------------------------------------

    printf("\n4. Using free()\n");

    free(arr1);
    free(arr2);

    arr1 = NULL;
    arr2 = NULL;

    printf("Allocated memory successfully released.\n");

    printf("\nProgram completed successfully.\n");

    return 0;
}
