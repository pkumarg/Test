#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define THREAD_COUNT 3
#define THREAD_NAME_LEN 20
#define THREAD_DATA_LEN_LOWER 5
#define THREAD_DATA_LEN_UPPER 50
#define THREAD_DATA_LEN ((rand() % (THREAD_DATA_LEN_UPPER - THREAD_DATA_LEN_LOWER + 1)) + THREAD_DATA_LEN_LOWER)

typedef struct ThreadId_t
{
  uint8_t threadId;
  struct ThreadId_t *next;
  struct ThreadId_t *prev;
}ThreadId_t;

typedef struct ThreadInfo_t
{
  pthread_t thHandle;
  ThreadId_t *pThNode;
  char thName[THREAD_NAME_LEN];
  uint8_t thData[THREAD_DATA_LEN_UPPER];
  uint8_t thDataLen;
}ThreadInfo_t;

ThreadInfo_t *gThInfo[THREAD_COUNT];
sem_t semTH[THREAD_COUNT];
uint8_t threadDone = 0;

void initThreadData()
{
  ThreadId_t *pFirstThNode = NULL;
  ThreadId_t *pPrevThNode = NULL;

  for(uint8_t iter = 0; iter < THREAD_COUNT; iter++)
  {
    ThreadInfo_t *pThInfo = malloc(sizeof(ThreadInfo_t));
    memset(pThInfo, 0, sizeof(ThreadInfo_t));
    gThInfo[iter] = pThInfo;

    snprintf(pThInfo->thName, THREAD_NAME_LEN, "THREAD_%u", iter);

    uint8_t dataIter = 0;

    for(; dataIter < THREAD_DATA_LEN; dataIter++)
    {
      pThInfo->thData[dataIter] = dataIter;
    }

    pThInfo->thDataLen = dataIter;

    // Update thread data
    ThreadId_t *pNewThNode = (ThreadId_t *) malloc(sizeof(ThreadId_t));
    pNewThNode->next = NULL;
    pNewThNode->threadId = iter;
    pNewThNode->prev = pPrevThNode;

    if(pPrevThNode)
      pPrevThNode->next = pNewThNode;

    pPrevThNode = pNewThNode;
    pThInfo->pThNode = pNewThNode;

    if(0 == iter)
    {
      pFirstThNode = pPrevThNode;
    }
  }

  // Make a circle
  pFirstThNode->prev = pPrevThNode;
  pPrevThNode->next = pFirstThNode;
}

void removeThreadFromLoop(ThreadId_t* pThNode)
{
  // Consider pThNode is quitting in A<->pThNode<->C
  ThreadId_t* A = pThNode->prev;
  ThreadId_t* C = pThNode->next;

  if((pThNode == pThNode->prev) || (pThNode == pThNode->next))
  {
    return;
  }

  A->next = C;
  C->prev = A;

  printf("Thread(%u) out of loop...\n", pThNode->threadId);
}

void *threadFunc(void *thArg)
{
  int iter = 0;
  ThreadInfo_t *pThInfo = (ThreadInfo_t *)thArg;

  ThreadId_t *pThNode = pThInfo->pThNode;
  ThreadId_t *pThNextNode;

  printf("Started thread=%u thDataLen=%u\n",
      pThInfo->pThNode->threadId, pThInfo->thDataLen);

  while(iter < pThInfo->thDataLen)
  {
    if(pThNode != pThNode->next)
    {
      sem_wait(&semTH[pThNode->threadId]);
    }

    pThNextNode = pThNode->next;

    printf("\nThread(%u) working on data=%u\n", pThNode->threadId,
        pThInfo->thData[iter]);
    if(iter == (pThInfo->thDataLen - 1))
      removeThreadFromLoop(pThNode);

    if(pThNode != pThNextNode)
    {
      printf("Thread(%u) enabling thread(%u)\n", pThNode->threadId,
          pThNextNode->threadId);
      sem_post(&semTH[pThNextNode->threadId]);
    }

    iter++;
  }

  return thArg;
}

void createThread(pthread_attr_t *thAttr, uint8_t thId)
{
  int retVal = pthread_create(&gThInfo[thId]->thHandle,
      thAttr, &threadFunc, (void *)gThInfo[thId]);
  if(retVal)
  {
    printf("Thread create failed thId=%u\n", thId);
    exit(1);
  }

  printf("Thread(%u) created...\n", thId);
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

void semPrintArrayTest()
{
  pthread_attr_t thAttr;
  ThreadInfo_t *pThInfo = NULL;

  initThreadData();
  initSemaphores();

  if(pthread_attr_init(&thAttr))
  {
    printf("Thread creation failed.\n");
    exit(1);
  }

  for(uint8_t iter = 0; iter < THREAD_COUNT; iter++)
    createThread(&thAttr, iter);
  printf("\n");

  // Post on very first thread to execute chain
  sem_post(&semTH[0]);

  for(uint8_t iter = 0; iter < THREAD_COUNT; iter++)
  {
    pthread_join(gThInfo[iter]->thHandle, (void **)&pThInfo);

    if(pThInfo)
    {
      printf("Thread(%u) Name=%s(%lu) exited.\n\n",
          pThInfo->pThNode->threadId, pThInfo->thName,
          pThInfo->thHandle);

      // Free thread node
      if(pThInfo->pThNode)
      {
        free(pThInfo->pThNode);
        pThInfo->pThNode = NULL;
      }

      // Now finally free thread info
      free(pThInfo);
      pThInfo = NULL;
    }
  }
}
