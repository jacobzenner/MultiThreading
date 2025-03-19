Multi-Threaded Production Simulation
This program simulates the production of milk, cheese, and cheeseburgers using multiple producer and consumer threads. It demonstrates inter-thread synchronization using semaphores and mutex locks.

Instructions to Run Code
Compilation
Compile the code using the following command: gcc -std=c11 -pthread multiThread.c

Execution
Run the compiled program: ./a.out

When prompted, enter the number of cheeseburgers you want to produce. The program will create the necessary producer and consumer threads accordingly.

Program Design Description
The program's architecture is based on a producer-consumer model with multiple buffers and thread synchronization.

Components
Milk Producer Threads: Produce bottles of milk and add them to a shared milk buffer.
Cheese Producer Threads: Consume milk bottles from the milk buffer to produce cheese slices, then add these slices to a shared cheese buffer.
Cheeseburger Producer Thread: Consumes cheese slices from the cheese buffer to produce cheeseburgers.
Buffer Management
Each buffer is controlled using two semaphores and a mutex lock to ensure safe concurrent access:

Milk Buffer:

milk_empty: Tracks the number of empty slots available.
milk_full: Tracks the number of milk bottles currently available.
Cheese Buffer:

cheese_empty: Tracks the number of empty slots available.
cheese_full: Tracks the number of cheese slices currently available.
Synchronization Mechanisms
Semaphores:

Producers:
Wait on the empty semaphore to ensure space is available before adding an item. After production, they signal the full semaphore to indicate that an item is ready for consumption.
Consumers:
Wait on the full semaphore to ensure that an item is available before consuming it. After consumption, they signal the empty semaphore to indicate that space has been freed in the buffer.
Mutex Locks:

Milk Mutex (milk_mutex) and Cheese Mutex (cheese_mutex):
Ensure that only one thread can modify a buffer at any given time, preventing race conditions during simultaneous access by multiple threads.
