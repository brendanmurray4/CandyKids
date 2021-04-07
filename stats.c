#include "stats.h"
#include <stdlib.h>
#include <stdio.h>

//Struct to hold all the different data values we need for each factory
typedef struct {
    int nummade;
    int numeaten;
    double min_delay;
    double max_delay;
    double delaysum;
    _Bool first;
} stats_t;

//Array to store our stats and a number of factories
int numfactories;
stats_t* statsarray;

//Initialize our structs for each factory
void stats_init(int num_producers)
{
    statsarray = malloc(num_producers*sizeof(stats_t));
    for(int i = 0; i < num_producers; i++)
    {
        statsarray[i].nummade = 0;
        statsarray[i].numeaten = 0;
        statsarray[i].min_delay = 0.0;
        statsarray[i].max_delay = 0.0;
        statsarray[i].delaysum = 0.0;
        statsarray[i].first = 1;
    }
    numfactories = num_producers;
}

//Free the memory we allocated for statsarray
void stats_cleanup(void)
{
    free(statsarray);
}

//Increment the number of candies made
void stats_record_produced(int factory_number)
{
    statsarray[factory_number].nummade++;
}

//Record that a candy was consumed, and also update max or min delay if necessary
void stats_record_consumed(int factory_number, double delay_in_ms)
{
    statsarray[factory_number].numeaten++;
    statsarray[factory_number].delaysum += delay_in_ms;

    if(delay_in_ms > statsarray[factory_number].max_delay)
    {
        statsarray[factory_number].max_delay = delay_in_ms;
    }
    if(delay_in_ms < statsarray[factory_number].min_delay || statsarray[factory_number].first)
    {
        statsarray[factory_number].min_delay = delay_in_ms;
        if(statsarray[factory_number].first)
        {
            statsarray[factory_number].first = 0;
        }
    }
}

//Display our stats in nice table formatted as specified in the assignment
void stats_display(void)
{
    printf("Statistics:\nFactory#   #Made  #Eaten  Min Delay[ms]  Avg Delay[ms]  Max Delay[ms]\n");
    for(int i =0; i< numfactories; i++)
    {
        printf("%8d%8d%8d", i, statsarray[i].nummade, statsarray[i].numeaten);
        printf("%15.5f%15.5f%15.5f\n", statsarray[i].min_delay, statsarray[i].delaysum/statsarray[i].numeaten, statsarray[i].max_delay);
        if(statsarray[i].nummade != statsarray[i].numeaten)
        {
            printf("ERROR: Mismatch between number made and eaten.\n");
        }
    }
}
