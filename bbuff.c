#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include "bbuff.h"

sem_t empty, full, mut;
void* buff[BUFFER_SIZE];
int emptyint = BUFFER_SIZE;

void bbuff_init(void)
{
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    sem_init(&mut, 0, 1);
    return;
}

void bbuff_blocking_insert(void* item)
{  
    //Checking if it is full, and waiting
    sem_wait(&empty);
    //Checking if some other thread is making changes to the buffer
    sem_wait(&mut);
    //Store value in the array and decrement our index
    //printf("Before inserting\n");
    buff[BUFFER_SIZE- emptyint] = item;
    //printf("after inserting\n");
    emptyint--;
    //Allow other threads waiting to extract to continue
    sem_post(&full);
    //Free the mutex
    sem_post(&mut);
    
    return;
}
void* bbuff_blocking_extract(void)
{
    //Checking if it is empty, and waiting
    sem_wait(&full);
    //Checking if some other thread is making changes to the buffer
    sem_wait(&mut);

    //Extract value in the array and increment our index
    void *temp = buff[BUFFER_SIZE - emptyint -1];
    buff[BUFFER_SIZE-emptyint-1] = NULL;
    emptyint++;
    //Allow other threads waiting to insert to continue
    sem_post(&mut);
    //Free the mutex
    sem_post(&empty);
    return temp;
}
_Bool bbuff_is_empty(void)
{
    if(emptyint == BUFFER_SIZE)
        return 1;
        
    return 0;
}
