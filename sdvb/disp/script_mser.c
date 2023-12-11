/********************************
Author: Sravanthi Kota Venkata
********************************/

#include "mser.h"

#define min(a,b) (a<b)?a:b
#define max(a,b) (a>b)?a:b

int benchmark_mser()
{

    int which_image;
    int i, j, k;
    I2D *idx;
    I2D *I;
    I2D *It;
    int rows=196, cols=98;
    int minVal = 1000;
    int maxVal = -1000;
    int lev = 10;

    unsigned int* start, *endC, *elapsed;
    char im1[100], im2[100];
    




    I = readImage("/ram/mser/1.bmp");
    rows = I->height;
    cols = I->width;

    It = readImage("/ram/mser/1.bmp");

    k = 0;
    for(i=0; i<cols; i++)
    {
        for(j=0; j<rows; j++)
        {
            asubsref(It,k++) = subsref(I,j,i);
        }
    }
    
    start = photonStartTiming();
    idx = mser(It, 2);
    endC = photonEndTiming();
    elapsed = photonReportTiming(start, endC);

    FF_PRINTF("Input size\t\t- (%dx%d)\n", rows, cols);
#ifdef CHECK   
    /** Self checking - use expected.txt from data directory  **/
    {
        int tol, ret=0;
        tol = 1;
#ifdef GENERATE_OUTPUT
        writeMatrix(idx, argv[1]);
#endif
        ret = selfCheck(idx, argv[1], tol);
        if (ret == -1)
            FF_PRINTF("Error in MSER\n");
    }
    /** Self checking done **/
#endif    
    photonPrintTiming(elapsed);
   
    vPortFree(start); 
    vPortFree(endC); 
    vPortFree(elapsed); 
    iFreeHandle(idx);
    iFreeHandle(I);
    iFreeHandle(It);
    
    return 0;
}

