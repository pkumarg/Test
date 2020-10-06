#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define THREAD_COUNT 10
#define THREAD_NAME_LEN 20
#define THREAD_DATA_LEN_LOWER 10
#define THREAD_DATA_LEN_UPPER 50
#define THREAD_DATA_LEN ((rand() % (THREAD_DATA_LEN_UPPER - THREAD_DATA_LEN_LOWER + 1)) + THREAD_DATA_LEN_LOWER)
#define TEST_ITERATION 20

sem_t semTH[THREAD_COUNT];
uint8_t threadDone = 0;
int isPreviousThreadDone = -1;

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
            gThInfo[iter].thData[dataIter] = iter;
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

void removePreviousThread(uint8_t tid) {
    // Consider B is quitting in A<->B<->C
    ThreadId_t* B = gThInfo[tid].thId;

    ThreadId_t* A = B->prev;
    ThreadId_t* C = B->next;

    if(B->next == B->prev)
        return;

    A->next = C;
    C->prev = A;

    printf("Removed thread=%d\n", isPreviousThreadDone);
    isPreviousThreadDone = -1;
}

void *threadFunc(void *thArg)
{
    int iter = 0;
    ThreadInfo_t *pThInfo = (ThreadInfo_t *)thArg;

    ThreadId_t *pThId = pThInfo->thId;

    while(iter < pThInfo->thDataLen)
    {
        if(pThId->prev != pThId->next)
            sem_wait(&semTH[pThId->id]);

        // If previous thread was finished remove it from
        // loop
        if(isPreviousThreadDone >= 0)
            removePreviousThread(isPreviousThreadDone);

        //printf("Thread Handle=%lu Name=%s Data=%u\n", pThInfo->thHandle,
                //pThInfo->thName, pThInfo->thData[iter]);

        if(pThId->prev != pThId->next)
            sem_post(&semTH[pThId->next->id]);

        iter++;
    }

    isPreviousThreadDone = (int)pThId->id;

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
