#pragma once

#include <time.h>


static inline double cputime_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    return 1.0e3 * (double)ts.tv_sec + 1.0e-6 * (double)ts.tv_nsec; 
}
