#include "person.h"
#include "candy.h"
#include "belt.h"

#include <iostream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <queue>
#include <thread>
#include <semaphore.h>

using namespace std;

//macros for sleep
#define NSPERMS 1000000
#define MSPERSEC 1000

//general macros
#define PROD_LIMIT 100
#define BELT_LIMIT 10
#define FROG_LIMIT 3
#define ZERO 0

//semaphores for frog limit, belt limit, unconsumed items on
//the belt, a mutex, and synchronization with main
sem_t cfbSem;
sem_t beltSem;
sem_t unconsumedSem;
sem_t mutex;
sem_t mainSem;

//function prototypes for thread functions
void * producer(void * pPtr);
void * consumer(void * cPtr);

int main(int argc, char **argv)
{
    //variables to hold wait time arguments from cmd line
    int args = 0;
    int ethelTime = 0;
    int lucyTime = 0;
    int frogTime = 0;
    int escargotTime = 0;

    //parser for the command line arguments
    while ((args = getopt(argc, argv, "E:L:f:e:")) != -1)
    {
        switch (args)
        {
            case 'E':
            {
                ethelTime = atoi(optarg);
                break;
            }

            case 'L':
            {
                lucyTime = atoi(optarg);
                break;
            }

            case 'f':
            {
                frogTime = atoi(optarg);
                break;
            }

            case 'e':
            {
                escargotTime = atoi(optarg);
                break;
            }

            default:
            {
                cout << "Error with arguments passed." << endl;
                exit(0);
            }
        }
    }
 
    //shared variable to track number of candies produced
    int candiesProduced = ZERO;

    //shared belt struct which is the buffer
    BELT * belt = new BELT();
    belt->numFrogs = ZERO;
    belt->numEscargot = ZERO;
    belt->onBelt = ZERO;

    //initializing all the information for ethel
    PERSON * ethel = new PERSON();
    ethel->name = "Ethel";
    ethel->consumptionTime = ethelTime;
    ethel->totalCandiesProduced = &candiesProduced;
    ethel->numFrogs = ZERO;
    ethel->numEscargot = ZERO;
    ethel->numTotal = ZERO;
    ethel->belt = belt;
    ethel->personSleep.tv_sec = ethelTime / MSPERSEC;
    ethel->personSleep.tv_nsec = (ethelTime % MSPERSEC) * NSPERMS;

    //intializing all the information for lucy
    PERSON * lucy = new PERSON();
    lucy->name = "Lucy";
    lucy->consumptionTime = lucyTime;
    lucy->totalCandiesProduced = &candiesProduced;
    lucy->numFrogs = ZERO;
    lucy->numEscargot = ZERO;
    lucy->numTotal = ZERO;
    lucy->belt = belt;
    lucy->personSleep.tv_sec = lucyTime / MSPERSEC;
    lucy->personSleep.tv_nsec = (lucyTime % MSPERSEC) * NSPERMS;

    //initializing all the information for frog candy
    CANDY * frog = new CANDY();
    frog->name = "Crunchy Frog Bites";
    frog->productionTime = frogTime;
    frog->numProduced = ZERO;
    frog->totalCandiesProduced = &candiesProduced;
    frog->belt = belt;
    frog->candySleep.tv_sec = frogTime / MSPERSEC;
    frog->candySleep.tv_nsec = (frogTime % MSPERSEC) * NSPERMS;

    //intiializing all the information for escargot candy
    CANDY * escargot = new CANDY();
    escargot->name = "Everlasting Escargot Suckers";
    escargot->productionTime = escargotTime;
    escargot->numProduced = ZERO;
    escargot->totalCandiesProduced = &candiesProduced;
    escargot->belt = belt;
    escargot->candySleep.tv_sec = escargotTime / MSPERSEC;
    escargot->candySleep.tv_nsec = (escargotTime % MSPERSEC) * NSPERMS;

    //initializing the semaphores
    sem_init(&cfbSem, 0, FROG_LIMIT);
    sem_init(&beltSem, 0, BELT_LIMIT);
    sem_init(&unconsumedSem, 0, 0);
    sem_init(&mutex, 0, 1);
    sem_init(&mainSem, 0, 0);

    //creating the threads
    thread frogThread(producer, (void*) frog);
    thread escargotThread(producer, (void*) escargot);
    thread lucyThread(consumer, (void*) lucy);
    thread ethelThread(consumer, (void*) ethel);

    //main thread synchronization begin
    sem_wait(&mainSem);

    //production report for the end
    cout << endl << "PRODUCTION REPORT" << endl;
    cout << "--------------------------------------------------" << endl;
    frog->print();
    escargot->print();
    lucy->print();
    ethel->print();

    exit(0);
}

//producer thread function
void * producer(void * pPtr)
{
    //creating a variable to hold data input by the function
    CANDY * c;
    c = (CANDY*) pPtr;

    //checking which candy thread is being used
    if(c->name == "Crunchy Frog Bites")
    {
        //infinite loop to keep producing before being broken out of
        while(true)
        {
            //checking for wait time to sleep
            if(c->productionTime > 0)
            {
                nanosleep(&c->candySleep, NULL);
            }            

            //beginning critical section enter
            sem_wait(&cfbSem);
            sem_wait(&beltSem);
            sem_wait(&mutex);

            //checking to see if production limit has been hit
            if(*(c->totalCandiesProduced) < PROD_LIMIT)
            {
                //incrememnting production limit, belt variables, and specific candy
                //production variable for end tracking
                *(c->totalCandiesProduced) += 1;
                c->numProduced += 1;
                c->belt->numFrogs += 1;
                c->belt->onBelt += 1;
                c->belt->buffer.push(c->name);

                //outputing log of belt and what was produced
                cout << "Belt: " << c->belt->numFrogs << " frogs + " << c->belt->numEscargot
                     << " escargots = " << c->belt->onBelt << ". produced: " << *(c->totalCandiesProduced)
                     << "\t Added " << c->name << "." << endl;

            }
            //else condition if production limit was met to give back semaphores
            //and break out of loop
            else
            {
                sem_post(&mutex);
                sem_post(&beltSem);
                sem_post(&cfbSem);

                break;
            }

            //ending critical section
            sem_post(&mutex);
            sem_post(&unconsumedSem);
        }
    }
    //checking if other candy
    else if(c->name == "Everlasting Escargot Suckers")
    {
        //infinite loop to keep producing before being broken out of
        while(true)
        {
            //checking for wait time to sleep
            if(c->productionTime > 0)
            {
                nanosleep(&c->candySleep, NULL);
            } 

            //beginning critical section enter
            sem_wait(&beltSem);
            sem_wait(&mutex);

            //checking to see if production limit has been hit
            if(*(c->totalCandiesProduced) < PROD_LIMIT)
            {
                //incrememnting production limit, belt variables, and specific candy
                //production variable for end tracking
                *(c->totalCandiesProduced) += 1;
                c->numProduced += 1;
                c->belt->numEscargot += 1;
                c->belt->onBelt += 1;
                c->belt->buffer.push(c->name);

                //outputing log of belt and what was produced
                cout << "Belt: " << c->belt->numFrogs << " frogs + " << c->belt->numEscargot
                     << " escargots = " << c->belt->onBelt << ". produced: " << *(c->totalCandiesProduced)
                     << "\t Added " << c->name << "." << endl;

            }
            //else condition if production limit was met to give back semaphores
            //and break out of loop
            else
            {
                sem_post(&mutex);
                sem_post(&beltSem);

                break;
            }

            //ending critical section
            sem_post(&mutex);
            sem_post(&unconsumedSem);
        }
    }

    return NULL;
}

//consumer thread function
void * consumer(void * cPtr)
{
    //creating a variable to hold data input by the function
    PERSON * p;
    p = (PERSON*) cPtr;

    //temp string for passing buffer information to consumer
    string temp;

    //infinite loop to keep consuming until broken out of
    while(true)
    {
        //checking for wait time to sleep
        if(p->consumptionTime > 0)
        {
            nanosleep(&p->personSleep, NULL);
        } 
        
        //beginning critical section enter
        sem_wait(&unconsumedSem);
        sem_wait(&mutex);

        //checking to see if production limit has been hit and buffer is empty
        if(*(p->totalCandiesProduced) >= PROD_LIMIT && p->belt->buffer.empty())
        {
            //giving back semaphores
            sem_post(&mutex);
            sem_post(&unconsumedSem);

            //completing main thread synchronization
            sem_post(&mainSem);
        }
        else
        {
            //assigning front belt item to temp string and removing item from belt
            temp = p->belt->buffer.front();
            p->belt->buffer.pop();

            //checking which candy was on belt
            if(temp == "Crunchy Frog Bites")
            {
                //incrementing consumer candy data and decrementing the belt totals
                p->belt->numFrogs -= 1;
                p->belt->onBelt -= 1;
                p->numFrogs += 1;
                p->numTotal += 1;

                //outputing the log information
                cout << "Belt: " << p->belt->numFrogs << " frogs + " << p->belt->numEscargot
                     << " escargots = " << p->belt->onBelt << ". produced: " << *(p->totalCandiesProduced)
                     << "\t " << p->name << " consumed " << temp << "." << endl;
            
                //giving back frog semaphore
                sem_post(&cfbSem);
            }
            //if other candy
            else if(temp == "Everlasting Escargot Suckers")
            {
                //incrementing consumer candy data and decrementing the belt totals
                p->belt->numEscargot -= 1;
                p->belt->onBelt -= 1;
                p->numEscargot += 1;
                p->numTotal += 1;

                //outputing the log information
                cout << "Belt: " << p->belt->numFrogs << " frogs + " << p->belt->numEscargot
                     << " escargots = " << p->belt->onBelt << ". produced: " << *(p->totalCandiesProduced)
                     << "\t " << p->name << " consumed " << temp << "." << endl;
            }

        }

        //ending critical section
        sem_post(&mutex);
        sem_post(&beltSem);

        //entering critical section to check if production limit was hit and belt is empty
        sem_wait(&mutex);
        if(*(p->totalCandiesProduced) >= PROD_LIMIT && p->belt->buffer.empty())
        {
            //completing main thread synchronization
            sem_post(&mainSem);
        }
        //ending critical section
        sem_post(&mutex);
    }

    return NULL;
}