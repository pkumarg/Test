#include <stdio.h>
#include <stdlib.h>

#define UINT8_T unsigned char
#define UINT32_T unsigned int
#define UINT64_T unsigned long long int

//#define PROG_DBG
#ifdef PROG_DBG
#define DBG_LOG(...) fprintf(stderr, ##__VA_ARGS__);
#else
#define DBG_LOG(...)
#endif
typedef struct Toilet_t
{
    UINT32_T toiletIdx;
    UINT32_T empId;
    UINT32_T nextFree;
    struct Toilet_t *nextToilet;
}Toilet;

UINT32_T M = 0; // Number of toilets
Toilet *gpToilet;
UINT64_T reserveCount;
UINT8_T firstToiletFree = 1;


void insertAfterToilet(Toilet *pNode, UINT32_T empId)
{
    Toilet *pNewToilet = (Toilet *) malloc(sizeof(Toilet));

    pNewToilet->empId = empId;
    if((pNode->toiletIdx + pNode->nextFree) == (M - 1))
    {
        pNewToilet->toiletIdx = M - 1;
        pNewToilet->nextFree = 0;
        pNode->nextFree -= 1;
    }
    else
    {
        pNewToilet->toiletIdx = pNode->toiletIdx + ((pNode->nextFree + 1) / 2);
        pNewToilet->nextFree = pNode->nextToilet->toiletIdx - pNewToilet->toiletIdx - 1;
        pNode->nextFree = pNewToilet->toiletIdx - pNode->toiletIdx - 1;
    }

    // Inserting node now
    Toilet *tempToilet = pNode->nextToilet;
    pNode->nextToilet = pNewToilet;
    pNewToilet->nextToilet = tempToilet;

    // Updating reserveCount for inserted node
    reserveCount += pNewToilet->toiletIdx + 1;
    DBG_LOG("Reserved Toilet=%u\n", (pNewToilet->toiletIdx + 1));
}

Toilet *findMaxToiletSpace()
{
    Toilet *pNode = gpToilet;
    Toilet *pMaxNode = NULL;

    UINT64_T maxVal = 0;

    while(pNode)
    {
        if(maxVal < pNode->nextFree)
        {
            maxVal = pNode->nextFree;
            pMaxNode = pNode;
        }

        pNode = pNode->nextToilet;
    }

    if(pMaxNode)
    {
        DBG_LOG("MAX Node Idx=%u NextFree=%u\n", pMaxNode->toiletIdx, pMaxNode->nextFree);
    }
    return pMaxNode;
}

void reserveToilet(UINT32_T empId)
{
    Toilet *pMaxNode = findMaxToiletSpace();

    if((pMaxNode == gpToilet) && firstToiletFree)
    {
        pMaxNode->empId = empId;
        pMaxNode->nextFree -= 1;

        reserveCount += 1;
        firstToiletFree = 0;

        DBG_LOG("Reserved Idx=%u\n", 1);
    }
    else
    {
        insertAfterToilet(pMaxNode, empId);
    }
}

void leaveToilet(UINT32_T empId)
{
    Toilet *pToilet = gpToilet;
    Toilet *pPrevToilet = gpToilet;

    if(!pToilet)
        return;

    do{
        if(pToilet->empId == empId)
        {
            if(pToilet == gpToilet)
            {
                pToilet->nextFree += 1;
                firstToiletFree = 1;

                DBG_LOG("Left firstNode FirstNextFree=%u\n", pToilet->nextFree);
            }
            else
            {
                pPrevToilet->nextFree += pToilet->nextFree + 1;
                pPrevToilet->nextToilet = pToilet->nextToilet;
                DBG_LOG("Left Toilet Idx=%u PrevNextFree=%u\n", pToilet->toiletIdx, pPrevToilet->nextFree);
                free(pToilet);
            }

            return;
        }

        pPrevToilet = pToilet;
        pToilet = pToilet->nextToilet;
    }while(pToilet);
}

void freeToiletNodes()
{
    Toilet *pTempNode = gpToilet->nextToilet;
    Toilet *pFreeNode = pTempNode;

    while(pTempNode)
    {
        pTempNode = pFreeNode->nextToilet;
        free(pFreeNode);
        pFreeNode = pTempNode;
    }

    gpToilet->nextToilet = NULL;
}

int main(void)
{
    UINT32_T T = 0; // Number of test cases
    UINT32_T N = 0; // Test case input number
    UINT32_T caseIter = 0;
    UINT32_T testIter = 0;
    UINT32_T testChoice = 0;
    UINT32_T empId = 0;

    // Allocate first & last Toilet
    gpToilet = (Toilet *) malloc(sizeof(Toilet));

    scanf("%u", &T);
    for(; caseIter < T; caseIter++)
    {
        scanf("%u%u", &M, &N);
        gpToilet->nextFree = M;
        gpToilet->toiletIdx = 0;

        for(testIter = 0; testIter < N; testIter++)
        {
            scanf("%u%u", &testChoice, &empId);

            switch(testChoice)
            {
                case 1:
                    reserveToilet(empId);
                    break;
                case 2:
                    leaveToilet(empId);
                    break;
                default:
                    printf("Error in input\n");
                    break;
            }
        }
        fprintf(stderr, "%u# %llu\n", (caseIter + 1), reserveCount);

        // Global clean-up overhead every test case -- Gotta find some other method
        freeToiletNodes();
        firstToiletFree = 1;
        reserveCount = 0;
    }

    // Finally free first node also
    free(gpToilet);
}
