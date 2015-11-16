#include <stdio.h>
#include <string.h>
#include <omp.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// populate histogram with points

void populateHistogram(
    int* in,
    float* buff,
    long buffSize,
    float histLen,
    int gridLen,
    int gridBytes)
{

#pragma omp parallel for
    for (int i = 0; i < buffSize; i += 2)
    {
        int x = (int) MIN( (buff[i] * gridLen),   gridLen - 1);
        int y = (int) MIN( (buff[i+1] * gridLen), gridLen - 1);
        in[y * gridLen + x]++;
    }
}

/*
// show method, memory intensive
printf("Using %d threads\n", omp_get_max_threads());
#pragma omp parallel
{
    int in_local[gridBytes];
    memset(in_local, 0, gridBytes);
    #pragma omp for nowait
    for (int i = 0; i < buffSize; i += 2)
    {
        int x = (int) (buff[i] / histLen);
        int y = (int) (buff[i+1] / histLen);
        if (x == gridLen) x--;
        if (y == gridLen) y--;
        in_local[y * gridLen + x]++;
    }
    #pragma omp critical
    {
        for (int i = 0; i < gridLen * gridLen; i++) {
            in[i] += in_local[i];
        }
    }
} // omp parallel
*/
