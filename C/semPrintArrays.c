#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define THREAD_COUNT 3
#define THREAD_NAME_LEN 20
#define THREAD_DATA_LEN_LOWER 5
#define THREAD_DATA_LEN_UPPER 50
#define THREAD_DATA_LEN ((rand() % (THREAD_DATA_LEN_UPPER - THREAD_DATA_LEN_LOWER + 1)) + THREAD_DATA_LEN_LOWER)

typedef struct ThreadId_t
{
    uint8_t id;
    struct ThreadId_t *next;
    struct ThreadId_t *prev;
}ThreadId_t;

typedef struct ThreadInfo_t
{
    pthread_t thHandle;
    ThreadId_t *thId;
    char thName[THREAD_NAME_LEN];
    uint8_t thData[THREAD_DATA_LEN_UPPER];
    uint8_t thDataLen;
}ThreadInfo_t;

ThreadInfo_t gThInfo[THREAD_COUNT];
sem_t semTH[THREAD_COUNT];
uint8_t threadDone = 0;
ThreadId_t* previousThreadDone_p = NULL;

void initThreadData()
{
    ThreadId_t *firstThId = NULL;
    ThreadId_t *prevThId = NULL;

    for(uint8_t iter = 0; iter < THREAD_COUNT; iter++)
    {
        snprintf(gThInfo[iter].thName, THREAD_NAME_LEN, "THREAD_%u", iter);

        uint8_t dataIter = 0;

        for(; dataIter < THREAD_DATA_LEN; dataIter++)
        {
            gThInfo[iter].thData[dataIter] = dataIter;
        }

        gThInfo[iter].thDataLen = dataIter;
        printf("ThreadId=%u ThreadDataLen=%u\n", iter, dataIter);

        // Update Id data
        ThreadId_t *newThId = (ThreadId_t *) malloc(sizeof(ThreadId_t));
        newThId->id = iter;
        newThId->next = NULL;
        newThId->prev = prevThId;

        if(prevThId)
            prevThId->next = newThId;

        prevThId = newThId;
        gThInfo[iter].thId = newThId;

        if(0 == iter)
        {
            firstThId = prevThId;
        }
    }

    // Make a circle
    firstThId->prev = prevThId;
    prevThId->next = firstThId;
}

void removeThreadFromLoop(ThreadId_t* thisTh_p)
{
    // Consider thisTh_p is quitting in A<->thisTh_p<->C
    ThreadId_t* A = thisTh_p->prev;
    ThreadId_t* C = thisTh_p->next;

    if((thisTh_p == thisTh_p->prev) || (thisTh_p == thisTh_p->next))
    {
        return;
    }

    A->next = C;
    C->prev = A;

    printf("Removed thread=%u\n", thisTh_p->id);
    free(thisTh_p);
    previousThreadDone_p = NULL;
}

void *threadFunc(void *thArg)
{
    int iter = 0;
    ThreadInfo_t *pThInfo = (ThreadInfo_t *)thArg;

    ThreadId_t *thId_p = pThInfo->thId;
    ThreadId_t *nextTh_p;

    printf("Started thread=%u thDataLen=%u\n",
            pThInfo->thId->id, pThInfo->thDataLen);

    while(iter < pThInfo->thDataLen)
    {
        if(thId_p != thId_p->next)
            sem_wait(&semTH[thId_p->id]);

        nextTh_p = thId_p->next;

        printf("Thread Handle=%lu Name=%s Data=%u\n", pThInfo->thHandle,
                pThInfo->thName, pThInfo->thData[iter]);

        if(iter == (pThInfo->thDataLen - 1))
            removeThreadFromLoop(thId_p);

        if(thId_p != nextTh_p)
        {
            sem_post(&semTH[nextTh_p->id]);
        }

        iter++;
    }

    return thArg;
}

void createThread(pthread_attr_t *thAttr, uint8_t thId)
{
    int retVal = pthread_create(&gThInfo[thId].thHandle,
            thAttr, &threadFunc, (void *)&gThInfo[thId]);
    if(retVal)
    {
        printf("Thread create failed thId=%u\n", thId);
        exit(1);
    }
}

void initSemaphores()
{
    for(uint8_t iter = 0; iter < THREAD_COUNT; iter++)
    {
        if(-1 == sem_init(&semTH[iter], 0, 0))
        {
            printf("TH1 seminit failed(%d)\n", errno);
        }
    }
}

int main()
{
    pthread_attr_t thAttr;
    ThreadInfo_t *pRetVal = NULL;

    memset(gThInfo, 0, THREAD_COUNT * sizeof(ThreadInfo_t));

    initThreadData();

    if(pthread_attr_init(&thAttr))
    {
        printf("Thread creation failed.\n");
        exit(1);
    }

    for(uint8_t iter = 0; iter < THREAD_COUNT; iter++)
        createThread(&thAttr, iter);

    // Post on very first thread to execute chain
    sem_post(&semTH[0]);

    for(uint8_t iter = 0; iter < THREAD_COUNT; iter++)
    {
        pthread_join(gThInfo[iter].thHandle, (void **)&pRetVal);

        if(pRetVal)
        {
            printf("%s(%lu) exited.\n", pRetVal->thName, gThInfo[iter].thHandle);
        }
    }
}
