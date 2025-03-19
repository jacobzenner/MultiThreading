// Name : Jacob Zenner
// Class : CSC 456
// Instructor: Dr. Huang
// Assignment: Programming Assignment 2

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MILK_BUFFER_SIZE 9
#define CHEESE_BUFFER_SIZE 4

// Function prototypes
void initialize_semaphores();
void destroy_semaphores();
void* milk_producer(void* args);
void* cheese_producer(void* args);
void* cheeseburger_producer(void* args);

// Buffers and counts
int milk_buffer[MILK_BUFFER_SIZE];
int milk_count = 0;
int cheese_buffer[CHEESE_BUFFER_SIZE];
int cheese_count = 0;

// Mutex and semaphores
pthread_mutex_t milk_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cheese_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t milk_full, milk_empty;
sem_t cheese_full, cheese_empty;

// Arguments for threads
typedef struct {
    int id;
    int items_to_produce;
} producer_args_t;

int main() {
    int cheeseburgers = 0;

    // Initialize semaphores
    initialize_semaphores();

    printf("How many burgers do you want? ");
    scanf("%d", &cheeseburgers);
    // input validation
    while (cheeseburgers < 1){
        printf("\nPlease enter a positive number ");
        scanf("%d", &cheeseburgers);
    }

    // calculating amount of each resource that is needed
    int milk_to_produce = cheeseburgers * 6;
    int cheese_to_produce = cheeseburgers * 2;

    // Create threads
    pthread_t milk_threads[3];
    pthread_t cheese_threads[2];
    pthread_t burger_thread;

    // Create milk producer threads
    for (int i = 0; i < 3; i++) {
        // Initialize producer arguments
        producer_args_t* args = malloc(sizeof(producer_args_t));
        // the referenced id is i + 1 because the first id of the milk producer is 1, not zero
        args->id = i + 1;
        // Distribute extra items to threads to balance if the items to produce is not evenly divisible by 3
        args->items_to_produce = milk_to_produce / 3 + (i < milk_to_produce % 3);
        // Create milk producer thread
        pthread_create(&milk_threads[i], NULL, milk_producer, args);
    }

    // Create cheese producer threads
    for (int i = 0; i < 2; i++) {
        // Initialize producer arguments
        producer_args_t* args = malloc(sizeof(producer_args_t));
        // the referenced id is i + 1 because the first id of the cheese producer is 4, not zero
        args->id = i + 4;
        // Distribute extra items to threads to balance if the items to produce is not evenly divisible by 2
        args->items_to_produce = cheese_to_produce / 2 + (i < cheese_to_produce % 2);
        // Create cheese producer thread
        pthread_create(&cheese_threads[i], NULL, cheese_producer, args);
    }

    // Create cheeseburger producer thread
    pthread_create(&burger_thread, NULL, cheeseburger_producer, &cheeseburgers);

    // Join threads
    for (int i = 0; i < 3; i++) {
        pthread_join(milk_threads[i], NULL);
    }
    for (int i = 0; i < 2; i++) {
        pthread_join(cheese_threads[i], NULL);
    }
    pthread_join(burger_thread, NULL);

    // Destroy semaphores
    destroy_semaphores();

    return 0;
}

// Initialize semaphores
void initialize_semaphores() {
    sem_init(&milk_full, 0, 0);
    sem_init(&milk_empty, 0, MILK_BUFFER_SIZE);
    sem_init(&cheese_full, 0, 0);
    sem_init(&cheese_empty, 0, CHEESE_BUFFER_SIZE);
}

// Destroy semaphores
void destroy_semaphores() {
    sem_destroy(&milk_full);
    sem_destroy(&milk_empty);
    sem_destroy(&cheese_full);
    sem_destroy(&cheese_empty);
}

/**
 * Milk producer thread function.
 * 
 * Role:
 * - Produces bottles of milk and places them in the shared milk buffer.
 * - If the buffer is full, the producer waits (using a semaphore).
 * 
 * Workflow:
 * 1. Waits for an empty slot in the milk buffer (using `milk_empty` semaphore).
 * 2. Locks the milk buffer to safely add a milk bottle.
 * 3. Produces a milk bottle and records it with the producer's ID.
 * 4. Unlocks the buffer and signals that a slot is filled (using `milk_full` semaphore).
 * 
 * Parameters:
 * - args: Pointer to `producer_args_t` structure containing the producer's ID and items to produce.
 * 
 * Returns:
 * - NULL
 */
void* milk_producer(void* args) {
    // Cast the argument to the producer_args_t structure
    producer_args_t* producer = (producer_args_t*) args;
    // Extract the producer ID from the arguments
    int id = producer->id;
    // Extract the number of items to produce from the arguments
    int items_to_produce = producer->items_to_produce;

    for (int i = 0; i < items_to_produce; i++) {
        // Wait for an empty slot in the milk buffer
        sem_wait(&milk_empty);
        // Lock the milk buffer to safely add a milk bottle
        pthread_mutex_lock(&milk_mutex);

        // Produce milk
        // Add the produced milk bottle to the buffer
        milk_buffer[milk_count++] = id;
        // Print the status of milk production
        printf("Milk producer %d produced milk. Total resources in milk buffer: %d\n", id, milk_count);

        // Unlock the milk buffer after adding the milk bottle
        pthread_mutex_unlock(&milk_mutex);
        // Signal that a new milk bottle has been added to the buffer
        sem_post(&milk_full);
    }
    // Free the allocated memory for producer arguments
    free(producer);
    return NULL;
}

/**
 * Cheese producer thread function.
 * 
 * Role:
 * - Consumes three bottles of milk and produces a single slice of cheese.
 * - Places the cheese slice into the shared cheese buffer.
 * - If there are fewer than three bottles of milk, the producer waits (using semaphores).
 * - If the cheese buffer is full, the producer also waits.
 * 
 * Workflow:
 * 1. Waits until three milk bottles are available (using `full_milk` semaphore three times).
 * 2. Locks the milk buffer to safely remove milk bottles.
 * 3. Consumes three bottles of milk and calculates the cheese ID.
 * 4. Signals that three milk slots are now empty (using `empty_milk` semaphore).
 * 5. Waits for an empty slot in the cheese buffer (using `empty_cheese` semaphore).
 * 6. Locks the cheese buffer to safely add the cheese slice.
 * 7. Adds the cheese slice with the calculated ID and signals that the cheese buffer has a filled slot.
 * 
 * Parameters:
 * - arg: Pointer to `producer_args_t` structure containing the producer's ID and items to produce.
 * 
 * Returns:
 * - NULL
 */
void* cheese_producer(void* args) {
    producer_args_t* producer = (producer_args_t*) args;
    int id = producer->id;
    int items_to_produce = producer->items_to_produce;

    for (int i = 0; i < items_to_produce; i++) {
        // Wait for three milk bottles
        sem_wait(&milk_full);
        sem_wait(&milk_full);
        sem_wait(&milk_full);

        // lock the milk mutex to access the milk resource
        pthread_mutex_lock(&milk_mutex);

        // Consume 3 milk bottles
        int milk1 = milk_buffer[--milk_count];
        int milk2 = milk_buffer[--milk_count];
        int milk3 = milk_buffer[--milk_count];

        // Print the status of milk consumption
        printf("Cheese producer %d consumed milk from producers %d, %d, %d.\n", id, milk1, milk2, milk3);

        // unlock the milk mutex, changes are made to the resource
        pthread_mutex_unlock(&milk_mutex);
        // signal empty spots in the milk buffer
        sem_post(&milk_empty);
        sem_post(&milk_empty);
        sem_post(&milk_empty);

        // Wait for space in cheese buffer
        sem_wait(&cheese_empty);
        // Lock the cheese buffer to safely add a cheese slice
        pthread_mutex_lock(&cheese_mutex);
        // Logic to calculate ID. ex: Milk bottles are 2, 1, and 3, and the producer is 4. ID will be 2134
        int cheese = milk1 * 1000 + milk2 * 100 + milk3 * 10 + id;
        // Add the produced cheese slice to the cheese buffer
        cheese_buffer[cheese_count++] = cheese;
        // Print the status of cheese production
        printf("Cheese producer %d produced cheese: %d. Total resources in cheese buffer: %d\n", id, cheese, cheese_count);
        // Unlock the cheese buffer after adding the cheese slice
        pthread_mutex_unlock(&cheese_mutex);
        // Signal that a new cheese slice has been added to the cheese buffer
        sem_post(&cheese_full);
    }

    // Free the allocated memory for producer arguments
    free(producer);
    return NULL;
}

/**
 * Cheeseburger producer thread function.
 * 
 * Role:
 * - Consumes two slices of cheese and produces a cheeseburger.
 * - If there are fewer than two slices of cheese, the producer waits (using semaphores).
 * 
 * Workflow:
 * 1. Waits until two cheese slices are available (using `full_cheese` semaphore twice).
 * 2. Locks the cheese buffer to safely remove cheese slices.
 * 3. Consumes two slices of cheese and calculates the cheeseburger ID.
 * 4. Signals that two slots in the cheese buffer are now empty (using `empty_cheese` semaphore).
 * 5. Prints the cheeseburger ID to the console.
 * 
 * Parameters:
 * - arg: Pointer to `producer_args_t` structure containing the producer's ID and items to produce.
 * 
 * Returns:
 * - NULL
 */
void* cheeseburger_producer(void* args) {
    // Extract the number of cheeseburgers to produce from the arguments
    int items_to_produce = *(int*) args;

    for (int i = 0; i < items_to_produce; i++) {
        // Wait for two cheese slices
        sem_wait(&cheese_full);
        printf("Recieved one cheese slice\n");
        sem_wait(&cheese_full);
        printf("Recieved second cheese slice\n");

        // lock the cheese mutex to access the resource
        pthread_mutex_lock(&cheese_mutex);

        // Consume 2 cheese slices from the buffer
        int cheese1 = cheese_buffer[--cheese_count];
        int cheese2 = cheese_buffer[--cheese_count];
        // Print the status of cheese consumption
        printf("Cheeseburger producer consumed cheese slices: %d and %d.\n", cheese1, cheese2);

        // unlock the cheese mutex, the buffer was adjusted after consuming the resources
        pthread_mutex_unlock(&cheese_mutex);
        // Signal that two slots in the cheese buffer are empty
        sem_post(&cheese_empty);
        sem_post(&cheese_empty);

        // Print the status of cheeseburger production
        printf("Cheeseburger produced with slices %d and %d: %d%d\n", cheese1, cheese2, cheese1, cheese2);
    }

    return NULL;
}
