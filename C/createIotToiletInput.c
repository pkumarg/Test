#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>


FILE *fp_OutputFile = NULL;

void openFile()
{
    fp_OutputFile = fopen("./InputFile", "w+");

    if(!fp_OutputFile)
    {
        printf("File open failed.\n");
        exit(1);
    }
}

void closeFile()
{
    if(fp_OutputFile)
        fclose(fp_OutputFile);
}

int main()
{
    uint32_t T, M;

    openFile();

    scanf("%u%u",  &T, &M);

    fprintf(fp_OutputFile, "%u",  T);
    fprintf(fp_OutputFile, "\n%u %u",  M, 2*M);

    // Write reserve
    for(uint32_t iterCase = 1; iterCase <= M; iterCase++)
    {
        fprintf(fp_OutputFile, "\n%u %u",  1, iterCase);
    }

    // Write leave
    for(uint32_t iterCase = 1; iterCase <= M; iterCase++)
    {
        fprintf(fp_OutputFile, "\n%u %u",  2, iterCase);
    }

    return 0;
}
