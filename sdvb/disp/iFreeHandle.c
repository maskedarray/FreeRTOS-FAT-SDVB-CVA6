/********************************
Author: Sravanthi Kota Venkata
********************************/

#include <stdio.h>
#include <stdlib.h>
#include "sdvbs_common.h"

void iFreeHandle(I2D* out)
{
    if(out != NULL)
        vPortFree(out);

    return;
}

