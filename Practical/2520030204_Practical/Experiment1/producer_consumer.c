#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define TOTAL_ITEMS 20

int buffer[BUFFER_SIZE];

int in = 0;
int out = 0;

// Counting semaphores
sem_t empty;
sem_t full;

// Mutex for protecting buffer
pthread_mutex_t mutex;

void *producer(void *arg)
{
    for (int i = 1; i <= TOTAL_ITEMS; i++)
    {
        // Wait for an empty space
        sem_wait(&empty);

        // Enter critical section
        pthread_mutex_lock(&mutex);

        buffer[in] = i;

        printf("Producer produced: %d\n", i);

        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);

        // Increase number of full slots
        sem_post(&full);

        // Small delay
        usleep(100000);
    }

    return NULL;
}

void *consumer(void *arg)
{
    for (int i = 1; i <= TOTAL_ITEMS; i++)
    {
        // Wait for an available item
        sem_wait(&full);

        // Enter critical section
        pthread_mutex_lock(&mutex);

        int item = buffer[out];

        printf("Consumer consumed: %d\n", item);

        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);

        // Increase number of empty slots
        sem_post(&empty);

        // Small delay
        usleep(150000);
    }

    return NULL;
}

int main()
{
    pthread_t producer_thread;
    pthread_t consumer_thread;

    // Initialize semaphores
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    // Create producer
    pthread_create(&producer_thread,
                   NULL,
                   producer,
                   NULL);

    // Create consumer
    pthread_create(&consumer_thread,
                   NULL,
                   consumer,
                   NULL);

    // Wait for both threads
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // Destroy synchronization objects
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    printf("\nProducer-Consumer execution completed.\n");

    return 0;
}
