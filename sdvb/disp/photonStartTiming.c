/********************************
Author: Sravanthi Kota Venkata
********************************/

/** C File **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "timingUtils.h"
#include "sdvbs_common.h"

unsigned int* photonStartTiming()
{
	unsigned int *array;

    array = (unsigned int*)pvPortMalloc(sizeof(unsigned int)*2);
	magic_timing_begin(array[0], array[1]);
    return array;
}

/** End of C Code **/
