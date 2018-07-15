#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define THREAD_COUNT 10
#define THREAD_NAME_LEN 20
#define THREAD_DATA_LEN 50
#define TEST_ITERATION 20

sem_t semTH[THREAD_COUNT];

typedef struct ThreadInfo_t
{
    pthread_t thHandle;
    uint8_t thId;
    char thName[THREAD_NAME_LEN];
    uint8_t thData[THREAD_DATA_LEN];
}ThreadInfo_t;

ThreadInfo_t gThInfo[THREAD_COUNT];

void initThreadData()
{
    for(uint8_t iter = 0; iter < THREAD_COUNT; iter++)
    {
	gThInfo[iter].thId = iter;
	snprintf(gThInfo[iter].thName, THREAD_NAME_LEN, "THREAD_%u", iter);

	for(uint8_t dataIter = 0; dataIter < THREAD_DATA_LEN; dataIter++)
	{
	    gThInfo[iter].thData[dataIter] = iter;
	}
    }
}

void *threadFunc(void *thArg)
{
    int iter = 0;
    ThreadInfo_t *pThInfo = (ThreadInfo_t *)thArg;

    while(iter < THREAD_DATA_LEN)
    {
	sem_wait(&semTH[pThInfo->thId]);
	printf("%s : %u\n", pThInfo->thName, pThInfo->thData[iter]);
	sem_post(&semTH[(pThInfo->thId + 1) % THREAD_COUNT]);

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
	    printf("%s exited.\n", pRetVal->thName);
	}
    }
}
