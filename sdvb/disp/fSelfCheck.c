/********************************
Author: Sravanthi Kota Venkata
********************************/

#include "sdvbs_common.h"

int fSelfCheck(F2D* in1, char* path, float tol)
{
    int r1, c1, ret=1;
    float *buffer;
    FILE* fd;
    int count=0, i, j;
    char file[256];
    
    r1 = in1->height;
    c1 = in1->width;

    buffer = (float*)pvPortMalloc(sizeof(float)*r1*c1);

    sprintf(file, "%s/expected_C.txt", path);
    fd = ff_fopen(file, "r");
    
    if(fd == NULL)
    {
        FF_PRINTF("Error: Expected file not opened %s\n", file);
        return -1;
    }
   
    while(!feof(fd))
    {
        fscanf(fd, "%f", &buffer[count]);
        count++;
    }
    count--;

    if(count != (r1*c1))
    {
        FF_PRINTF("Checking error: dimensions mismatch. Expected = %d, Observed = %d \n", count, (r1*c1));
        return -1;
    }
    
    for(i=0; i<r1*c1; i++)
    {
        float inVal = asubsref(in1,i);

        if( (inVal-buffer[i])>tol || (buffer[i]-inVal)>tol )
        {
            FF_PRINTF("Mismatch %d: (%f, %f)\n", i, buffer[i], inVal);
            return -1;
        }
    }

    ff_fclose(fd);
    FF_PRINTF("Verification\t\t- Successful\n");
    vPortFree(buffer);
    return ret;
}


