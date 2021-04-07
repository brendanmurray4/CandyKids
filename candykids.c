#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "bbuff.h"
#include "stats.h"

//Struct to record factory number and creation time of each individual candy we insert
typedef struct {
    int factory_number;
    double creation_ts_ms;
} candy_t;

//Bool to check if we are to stop creating candy.
_Bool stop_thread = 0;

//Function returns time in ms
double current_time_in_ms(void)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec *1000.0 + now.tv_nsec/1000000.0;
}

//Our factory function
void* factory_function(void* param)
{
    //Cast back our integer input
    int factnumber = *((int *)param);
    //The factory randomly generates a time to wait, then creates a piece of candy, initializes it, and stores it in the buffer
    //It then records stats and waits the time
    while(!stop_thread)
    {
        int wait = rand()%4;
        printf("Factory %d ships candy and waits %ds\n", factnumber, wait);
        candy_t *candy = malloc(sizeof(candy_t));
        if(candy == NULL)
        {
            printf("Memory allocation of candy structure failed in factory %d. Terminating.",factnumber);
            exit(-1);
        }
        candy->factory_number = factnumber;
        candy->creation_ts_ms = current_time_in_ms();
        bbuff_blocking_insert(candy);
        stats_record_produced(factnumber);
        sleep(wait);
    }
   
   printf("Candy factory %d done\n", factnumber);
   return NULL;
}

//The kid function pulls the candy out of the buffer, records that it consumed it, then frees the memory and sleeps for a random amount of time
void* kid_function(void* args)
{
    while(1)
    {
        candy_t* candy = (candy_t*) bbuff_blocking_extract();
        stats_record_consumed(candy->factory_number, current_time_in_ms()- (candy->creation_ts_ms));
        free(candy);
        //printf("Candy made from factory %d at time %f", candy->factory_number, candy->creation_ts_ms);
        sleep(rand()%2);
    }
}

int main(int argc, char *argv[])
{
    //Seed RNG with the time we start the program
    srand(time(NULL));
    int args[3];

    //Check for too many args
    if(argc != 4)
    {
        perror("ERROR: Incorrect number of arguments. Must have 3 arguments. Terminating. \n");
        exit(-1);
    }
    //Extracting our arguments
    for(int i=1; i<4; i++)
    {
        args[i-1] = atoi(argv[i]);
        if(args[i-1] < 0)
        {
            perror("ERROR: Argument was negative. All arguments must be > 0. Terminating. \n");
            exit(-1);
        }
    }

    //Initializing our buffer, stats, and thread attributes, and setting up arrays to store tids
    bbuff_init();
    stats_init(args[0]);
    pthread_t factory_tids[args[0]];
    pthread_t kid_tids[args[1]];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int facarray[args[0]];

    //Creating our factory threads
    for(int i = 0; i < args[0]; i++)
    {
        facarray[i] = i;
        if(pthread_create(&factory_tids[i], &attr, factory_function, &facarray[i]) != 0)
        {
            printf("ERROR: Failed to start factory thread %d. Terminating", i);
            exit(-1);
        }
    }

    //Creating our kid threads
    for(int i = 0; i< args[1]; i++)
    {
        if(pthread_create(&kid_tids[i], &attr, kid_function, NULL)!=0)
        {
            printf("ERROR: Failed to start kid thread %d. Terminating", i);
            exit(-1);
        }
    }

    //Waiting the specified amount of time
    for(int i = 0; i <args[2]; i++)
    {
        sleep(1);
        printf("Time %ds\n", i+1);
    }

    //Stopping factory threads
    stop_thread = 1;
    for(int i = 0; i< args[0]; i++)
    {
        pthread_join(factory_tids[i], NULL);
    }

    //Waiting for all candy to be consumed in a loop
    while(!bbuff_is_empty())
    {
        printf("Waiting for all candy to be consumed\n");
        sleep(1);
    }
    
    //Cancelling kid threads and joining them
    for(int i = 0; i< args[1]; i++)
    {
        pthread_cancel(kid_tids[i]);
        pthread_join(kid_tids[i], NULL);
    }

    //Displaying stats and freeing the last of the allocated memory
    stats_display();
    stats_cleanup();
    return 0;
}
