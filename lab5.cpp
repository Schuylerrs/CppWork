/***********************************************************************
* Program:
*    Lab ProducerConsumer
*    Brother Jones, CS 345
* Author:
*    Jordan Reed
*
* Conclusions:
*    This lab helped me understand how semaphores protect critical
*    sections from other processes.  When I would create a lot more
*    consumers than producers (or more producers than consumers) it
*    it was clear that they were doing their job because so many
*    consumers are trying to get access to the critical section.
*
*    The lab handout was well written and very helpful.  Most of
*    the code was given, so it wasn't as difficult to implement
*    as I expected.
*
* Summary:
*    This program uses semaphores and a mutex to create a producer
*    consumer simulation.
************************************************************************/
#include <iostream>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <iomanip>
using namespace std;

typedef int bufferItem;
#define BUFFER_SIZE 5
#define RANDOM_MIN 0
#define RANDOM_UPPER_LIMIT 1000
bufferItem buffer[BUFFER_SIZE];

pthread_mutex_t mtx;
sem_t empty, full;

int fronti, backi;

void *producer(void *param)
{
    long threadID = (long) param;

    bufferItem itemProduced;
    while (true)
    {
        /* sleep for a random period of time */
        usleep(rand() % 150000);

        /* generate a random number */
        itemProduced = RANDOM_MIN + (rand() % RANDOM_UPPER_LIMIT);

        /* insert item into shared global buffer and print what was done */
        sem_wait(&empty);
        pthread_mutex_lock(&mtx);

        // Critical section
        buffer[backi++] = itemProduced;
        backi %= BUFFER_SIZE;
        cout << setw(5) << itemProduced << setw(6) << "P" << threadID << endl;

        pthread_mutex_unlock(&mtx);
        sem_post(&full);
    }
}

void *consumer(void *param)
{
    long threadID = (long) param;

    bufferItem consumedItem;
    while (true)
    {
        /* sleep for a random period of time */
        usleep(rand() % 150000);

        /* consume item from shared global buffer and print what was done */
        sem_wait(&full);
        pthread_mutex_lock(&mtx);

        // Critical section
        consumedItem = buffer[fronti];
        buffer[fronti] = -1;
        fronti++;
        fronti %= BUFFER_SIZE;
        cout << setw(20) << consumedItem << setw(6) << "C" << threadID << endl;

        pthread_mutex_unlock(&mtx);
        sem_post(&empty);
    }
}

int main (int argc, char *argv[])
{
    /* 1. Check and get command line arguments argv[1], argv[2], argv[3] */
    if (argc < 4)
    {
       cout << "Usage: secondsToSleep numProducers numConsumers\n";
       exit(0);
    }
    int numSecondsToSleep  = atoi(argv[1]);
    int numProducerThreads = atoi(argv[2]);
    int numConsumerThreads = atoi(argv[3]);
    int result = 0;

    /* 2. Initialize buffer [good for error checking but not really needed]*/
    for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = -1;

    /* 3. Initialize the mutex lock and semaphores */
    result = pthread_mutex_init(&mtx, NULL);
    if (result)
    {
       cout << "Could not initialize mutex\n";
       return -1;
    }
    result = sem_init(&empty, 0, BUFFER_SIZE);
    if (result)
    {
       cout << "Could not initialize semaphore empty\n";
       return -1;
    }
    result = sem_init(&full,  0, 0);
    if (result)
    {
       cout << "Could not initialize semaphore empty\n";
       return -1;
    }

    int fronti = 0;
    int backi = 0;

    cout << "Produced by P# Consumed by C#\n"
         << "======== ===== ======== =====\n";

    /* 4. Create producer threads(s) */
    vector<pthread_t> threads;
    for (int i = 0; i < numProducerThreads; i++)
    {
        pthread_t thread;
        result = pthread_create(&thread, NULL, &producer, (void*)(i + 1));
        if (result)
        {
            cout << "Could not create thread\n";
            return -1;
        }
        threads.push_back(thread);
    }

    /* 5. Create consumer threads(s) */
    vector<pthread_t> consumerThreads;
    for (int i = 0; i < numConsumerThreads; i++)
    {
        pthread_t thread;
        result = pthread_create(&thread, NULL, &consumer, (void*)(i + 1));
        if (result)
        {
            cout << "Could not create thread\n";
            return -1;
        }
        threads.push_back(thread);
    }

    /* 6. Sleep [ to read manual page, do: man 3 sleep ]*/
    //usleep(numSecondsToSleep * 1000 * 1000);
    sleep(numSecondsToSleep);

    /* 7. Cancel threads [ pthread_cancel() ] */
    for (int i = 0; i < threads.size(); i++)
    {
        result = pthread_cancel(threads[i]);
        if (result)
        {
           cout << "Could not cancel thread\n";
           return -1;
        }
    }

    /* 8. Exit */
    result = pthread_mutex_destroy(&mtx);
    if (result)
    {
       cout << "Could not destroy mutex\n";
       return -1;
    }
    result = sem_destroy(&empty);
    if (result)
    {
       cout << "Could not destroy semaphore\n";
       return -1;
    }
    result = sem_destroy(&full);
    if (result)
    {
       cout << "Could not destroy semaphore\n";
       return -1;
    }
    return 0;
}
