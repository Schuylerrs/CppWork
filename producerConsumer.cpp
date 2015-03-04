/***********************************************************************
* Program:
*    Lab ProducerConsumer  
*    Brother Jones, CS 345
* Author:
*    Schuyler Summers
*
* Conclusions:
*    Threads may seem like they are a really technical and difficult
*    thing to wrap your head around by really they aren't that hard
*    at least once you get past the inputs.
*
* Summary:
*    This is a program that uses threads as producers and consumors
************************************************************************
*
* Changes made to my code for the re-submission:
*   - Changed the producers/consumers to start at 1 rather than 0
*   - Added a return in the if statement so that it quits if you don't put in
*     exactly 4 inputs
*   - Removed the hard coded 5 in the semephore creation and added the global
*   - Changed the random number to be between 0 and 999 rather than 0 - 9999
*   - Added a constant for max thread sleep time
*
*****************************************************************************/
   

#include <iostream>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>
#include <iomanip>

using namespace std;

typedef int bufferItem;
#define BUFFER_SIZE 5
#define THREAD_SLEEP 300000
bufferItem buffer[BUFFER_SIZE];
bufferItem consumeAt = 0;
bufferItem insertAt = 0;

pthread_mutex_t mutex;
sem_t full;
sem_t empty;

/***********************************************************************
 * Inserts an item into the buffer
 ***********************************************************************/
void insertItem(int item)
{
   buffer[insertAt] = item;
   insertAt = ++insertAt % BUFFER_SIZE;
}

/***********************************************************************
 * Takes an items out of the buffer
 ***********************************************************************/
int consumeItem()
{
   int item = buffer[consumeAt];

   // So I know if I am consuming bad data
   buffer[consumeAt] = -1;
   consumeAt = ++consumeAt % BUFFER_SIZE;

   return item;
}
   
/***********************************************************************
 * Produces items to be consumed
 ***********************************************************************/
void *producer (void *param)
{
   int* temp = (int*)param;
   int ID = *temp;
   
   delete temp;   // Free up dynamically allocated data

   bufferItem itemProduced;
   while (true)
   {
      /* sleep for a random period of time */
      usleep(rand() % THREAD_SLEEP);
      
      /* generate a random number */
      itemProduced = (rand() % 1000);

      /* insert item into shared global buffer and print what was done */
      sem_wait(&empty);
      pthread_mutex_lock(&mutex);

      insertItem(itemProduced);
      cout << setw(8) << itemProduced << setw(6) << "P" << ID << "\n";
      
      pthread_mutex_unlock(&mutex);
      sem_post(&full);
   }
}

/***********************************************************************
 * Consumes items from the buffer
 ***********************************************************************/
void *consumer(void *param)
{
   int* temp = (int*)param;
   int ID = *temp;

   delete temp; // Free up dynamically allocated memory
   
   bufferItem consumedItem;
   while (true)
   {
      /* sleep for a random period of time */
      usleep(rand() % THREAD_SLEEP);

      /* consume item from shared global buffer and print what was done */
      sem_wait(&full);
      pthread_mutex_lock(&mutex);

      consumedItem = consumeItem();
      cout << setw(22) << consumedItem << setw(6) << "C" << ID << "\n";
      
      pthread_mutex_unlock(&mutex);
      sem_post(&empty);
   }
} 

/***********************************************************************
 * Creates the producer and consumer threads, lets them do their thing,
 * goes to sleep for a time set by input, then wakes up and kills them
 * all
 * (In other words this is where the magic happens)
 ***********************************************************************/
int main (int argc, char *argv[])
{
   int sleepTime;
   int numProducers;
   int numConsumers;

   /* 1. Check and get command line arguments argv[1], argv[2], argv[3] */
   if (argc != 4)
   {
      cout << "Useage: producerConsumer SleepTime Producers Consumers\n";
      return -1;
   }
   else
   {
      sleepTime = atoi(argv[1]);
      numProducers = atoi(argv[2]);
      numConsumers = atoi(argv[3]);

      srand (time(NULL));
      cout << "Produced by P# Consumed by C#\n"
           << "======== ===== ======== =====\n";
   }

   /* 2. Initialize buffer [good for error checking but not really needed]*/
   for (int i = 0; i < BUFFER_SIZE; ++i)
      buffer[i] = -1;

   /* 3. Initialize the mutex lock and semaphores */
   pthread_mutex_init(&mutex, NULL);
   sem_init(&empty, 0, BUFFER_SIZE);
   sem_init(&full, 0, 0);

   /* 4. Create producer threads(s) */
   pthread_t* producerThreads = new pthread_t[numProducers];
   for (int i = 0; i < numProducers; ++i)
   {
      int* ID = new int(i + 1);
      if (pthread_create(&producerThreads[i], NULL, producer, (void *) ID))
      {
         cerr << "Error creating producer thread " << i << endl;
         // In case the thread it not created the memory needs to be cleaned up
         delete ID;
         return -2;
      }  
   }
   
   /* 5. Create consumer threads(s) */
   pthread_t* consumerThreads = new pthread_t[numConsumers];
   for (int i = 0; i < numConsumers; ++i)
   {
      // Dealocated within the thread
      int* ID = new int(i + 1);
      if (pthread_create(&consumerThreads[i], NULL, consumer, (void *) ID))
      {
         cerr << "Error creating consumer thread " << i << endl;
         return -3;
      }
   }

   /* 6. Sleep [ to read manual page, do: man 3 sleep ]*/
   sleep(sleepTime);

   /* 7. Cancel threads [ pthread_cancel() ] */
   for (int i = 0; i < numProducers; ++i)
      pthread_cancel(producerThreads[i]);
   for (int i = 0; i < numConsumers; ++i)
      pthread_cancel(consumerThreads[i]);

   // Free up data
   delete [] consumerThreads;
   delete [] producerThreads;

   /* 8. Exit */
   return 0;
}

