#ifndef BELT_H
#define BELT_H

#include <queue>
#include <string>

using namespace std;

//struct to hold belt buffer information and queue
typedef struct 
{
    //variables for printing belt information
    int numFrogs;
    int numEscargot;
    int onBelt;
    //queue to hold names of candies on belt so consumers
    //know which candies they are consuming
    queue <string> buffer;
} BELT;

#endif