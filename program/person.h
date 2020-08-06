#ifndef PERSON_H
#define PERSON_H

#include "belt.h"

#include <string>
#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;

//struct to house consumer information
typedef struct 
{
    //variables defining information about consumer
    string name;
    int consumptionTime;
    int numFrogs;
    int numEscargot;
    int numTotal;

    //pointers to shared variables
    int * totalCandiesProduced;
    BELT * belt;
    
    //wait time information
    struct timespec personSleep;
    
    //print function for production report
    void print()
    {
        cout << name << " consumed " << numFrogs << " crunchy frog bites + "
             << numEscargot << " everlasting escargot suckers = " << numTotal << endl;
    }
} PERSON;

#endif