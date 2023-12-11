/********************************
Author: Sravanthi Kota Venkata
********************************/

#include <stdio.h>
#include <stdlib.h>
#include "disparity.h"

int benchmark_disp()
{
    int rows = 32;
    int cols = 32;
    I2D *imleft, *imright, *retDisparity;

    unsigned int *start, *endC, *elapsed;
    
    int i, j;
    char im1[100], im2[100], timFile[100];
    int WIN_SZ=8, SHIFT=64;
    FILE* fp;
    

    
    imleft = readImage("/ram/disp/1.bmp");
    imright = readImage("/ram/disp/2.bmp");

    rows = imleft->height;
    cols = imleft->width;

#ifdef test
    WIN_SZ = 2;
    SHIFT = 1;
#endif
#ifdef sim_fast
    WIN_SZ = 4;
    SHIFT = 4;
#endif
#ifdef sim
    WIN_SZ = 4;
    SHIFT = 8;
#endif

    start = photonStartTiming();
    retDisparity = getDisparity(imleft, imright, WIN_SZ, SHIFT);
    endC = photonEndTiming();

    FF_PRINTF("Input size\t\t- (%dx%d)\n", rows, cols);
#ifdef CHECK   
    /** Self checking - use expected.txt from data directory  **/
    {
        int tol, ret=0;
        tol = 2;
#ifdef GENERATE_OUTPUT
        writeMatrix(retDisparity, argv[1]);
#endif
        ret = selfCheck(retDisparity, argv[1], tol);
        if (ret == -1)
            FF_PRINTF("Error in Disparity Map\n");
    }
    /** Self checking done **/
#endif

    elapsed = photonReportTiming(start, endC);
    photonPrintTiming(elapsed);
    
    iFreeHandle(imleft);
    iFreeHandle(imright);
    iFreeHandle(retDisparity);
    vPortFree(start);
    vPortFree(endC);
    vPortFree(elapsed);

    return 0;
}
