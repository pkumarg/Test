#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#define SHM_SZ 0x4000   // MAX SHM capacity
#define SHM_Q_ELE_SZ 64 // MAX size of one queue element 
#define SHM_Q_SZ 240    // MAX number of elements in queue

// Globals
key_t shm_key = 0xFF00FF00;
void *gp_shmAddr = NULL;
int gShmId = 0;

typedef struct shmQElement
{
    uint32_t size;
    char pData[SHM_Q_ELE_SZ];
}shmQElement;

typedef struct shmQ
{
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t endMark;
    shmQElement pShmEle[SHM_Q_SZ];
}shmQ;

// Global Q pointer
shmQ *gp_ShmQ = NULL;

void initShmQ()
{
    if((SHM_Q_SZ * SHM_Q_ELE_SZ) > SHM_SZ)
    {
        printf("Size requested is out of SHM capacity\n");
        exit(1);
    }

    gp_ShmQ = (shmQ *)gp_shmAddr;
    gp_ShmQ->head = 0;
    gp_ShmQ->tail = 0;
    gp_ShmQ->endMark = 0;
    gp_ShmQ->pShmEle[0].size = 0;
}

#define BUSY_WAIT(head, tail)\
{\
    while(((head + 1) % SHM_Q_SZ) == tail)\
    {\
    }\
}

void writeShmQ(void *pData, uint32_t size)
{
    uint32_t head = gp_ShmQ->head;
    uint32_t currSize = gp_ShmQ->pShmEle[head].size;
    void *pCurrQBuff = gp_ShmQ->pShmEle[head].pData + currSize;
    uint32_t remainSize = SHM_Q_ELE_SZ - currSize;

    while(size)
    {
        if(remainSize)
        {
            if(size >= remainSize)
            {
                memcpy(pCurrQBuff, pData, remainSize);
                gp_ShmQ->pShmEle[head].size = SHM_Q_ELE_SZ;
                pData += remainSize;
                size -= remainSize;

                if(size)
                    BUSY_WAIT(head, gp_ShmQ->tail);
                head = (head + 1) % SHM_Q_SZ;
                gp_ShmQ->head = head;

                pCurrQBuff = gp_ShmQ->pShmEle[head].pData;
                remainSize = SHM_Q_ELE_SZ;
            }
            else
            {
                memcpy(pCurrQBuff, pData, size);
                gp_ShmQ->pShmEle[head].size += size;
                size = 0;
            }
        }
    }
}

uint32_t readShmQ(void *pBuff)
{
    uint32_t head = gp_ShmQ->head;
    uint32_t tail = gp_ShmQ->tail;
    uint32_t readSize = 0;

    if(head == tail)
    {
        //printf("Nothing to read...\n");
    }
    else
    {
        readSize = gp_ShmQ->pShmEle[tail].size;

        if(readSize > SHM_Q_ELE_SZ)
            readSize = SHM_Q_ELE_SZ;

        memcpy(pBuff, gp_ShmQ->pShmEle[tail].pData, readSize);
        tail = (tail + 1) + SHM_Q_SZ;
        gp_ShmQ->tail = tail;

        printf("SHM read size=%u\n", readSize);
    }

    return readSize;
}

void initSharedMem()
{
    gShmId = shmget(shm_key, SHM_SZ, IPC_CREAT | IPC_EXCL | 0644);

    if(-1 == gShmId)
    {
        printf("shmget() failed: %s\n", strerror(errno));
        exit(1);
    }
    printf("Created shared mem seg shmId=%d\n", gShmId);

    if((gp_shmAddr = shmat(gShmId, NULL, 0)) == (void *)-1)
    {
        printf("Failed to attach shm: %s\n", strerror(errno));
        exit(1);
    }
}

void deInitSharedMem()
{
    if(-1 == shmdt(gp_shmAddr))
    {
        printf("Failed to detach shm: %s\n", strerror(errno));
    }

    if(-1 == shmctl(gShmId, IPC_RMID, NULL))
    {
        printf("shmctl() failed: %s\n", strerror(errno));
    }
}

void createThread(pthread_attr_t *pThAttr,
        pthread_t *thHandle,
        void * (*threadFunc)(void *), uint8_t thId)
{
    uint8_t *pThId = malloc(sizeof(uint8_t));

    *pThId = thId;

    int retVal = pthread_create(thHandle,
            pThAttr, threadFunc, (void *)pThId);
    if(retVal)
    {
        printf("Thread create failed(%d) thId=%u\n",
                retVal, thId);
        exit(1);
    }

    printf("Created thread thId=%u\n", thId);
}

void openFiles()
{
}

void *recvThread(void *pArg)
{
    char recvBuff[SHM_Q_ELE_SZ];

    while(1)
    {
        if(readShmQ(recvBuff))
        {
        }
        else if(gp_ShmQ->endMark == 0xFF00FF00)
        {
            break;
        }
        else
        {
            continue;
        }
    }

    printf("Receive thread done thId=%u\n", *((uint8_t *)pArg));
    return pArg;
}

void *sendThread(void *pArg)
{
    printf("Send thread done thId=%u\n", *((uint8_t *)pArg));
    return pArg;
}

void shmQSendRecvTest()
{
    pthread_attr_t thAttr;
    pthread_t thread0, thread1;

    uint8_t *pRetVal = NULL;

    initSharedMem();
    initShmQ();

    if(pthread_attr_init(&thAttr))
    {
        printf("Thread creation failed.\n");
        exit(1);
    }

    // Create Thread 0
    createThread(&thAttr, &thread0, &sendThread, 0);

    // Create Thread 1
    createThread(&thAttr, &thread1, &recvThread, 1);

    pthread_join(thread0, (void **)&pRetVal);
    if(pRetVal)
    {
        printf("Thread exited thId=%u\n", *pRetVal);
    }

    pthread_join(thread1, (void **)&pRetVal);
    if(pRetVal)
    {
        printf("Thread exited thId=%u\n", *pRetVal);
    }

    printf("Size shmQ=%lu\n", sizeof(shmQ));
    printf("Size shmQElement=%lu\n", sizeof(shmQElement));

    deInitSharedMem();
}
