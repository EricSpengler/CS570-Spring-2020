#ifndef CANDY_H
#define CANDY_H

#include "belt.h"

#include <string>
#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;

//struct to house producer information
typedef struct 
{
    //variables defining information about producer
    string name;
    int productionTime;
    int numProduced;

    //pointers to shared variables
    int * totalCandiesProduced;
    BELT * belt;

    //wait time information
    struct timespec candySleep;

    //print function for production report
    void print()
    {
        cout << name << " producer generated " << numProduced << " candies" << endl;
    }
} CANDY;

#endif