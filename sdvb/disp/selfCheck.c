/********************************
Author: Sravanthi Kota Venkata
********************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sdvbs_common.h"

int selfCheck(I2D* in1, char* path, int tol)
{
    int r1, c1, ret=1;
    FILE* fd;
    int count=0, *buffer, i, j;
    char file[100];
    int* data = in1->data;
    
    r1 = in1->height;
    c1 = in1->width;

    buffer = (int*)pvPortMalloc(sizeof(int)*r1*c1);

    sprintf(file, "%s/expected_C.txt", path);
    fd = ff_fopen(file, "r");
    if(fd == NULL)
    {
        FF_PRINTF("Error: Expected file not opened \n");
        return -1;
    }
   
    while(!feof(fd))
    {
        fscanf(fd, "%d", &buffer[count]);
        count++;
    }
    count--;

    if(count < (r1*c1))
    {
        FF_PRINTF("Checking error: dimensions mismatch. Expected = %d, Observed = %d \n", count, (r1*c1));
        return -1;
    }
    
    for(i=0; i<r1*c1; i++)
    {
        if((abs(data[i])-abs(buffer[i]))>tol || (abs(buffer[i])-abs(data[i]))>tol)
        {
            FF_PRINTF("Checking error: Values mismtach at %d element\n", i);
            FF_PRINTF("Expected value = %d, observed = %d\n", buffer[i], data[i] );
            return -1;
        }
    }
   
    ff_fclose(fd);
    vPortFree(buffer); 
    FF_PRINTF("Verification\t\t- Successful\n");
    return ret;
}







