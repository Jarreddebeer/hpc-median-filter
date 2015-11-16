#include <stdio.h>
#include <string.h>

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
    for (int i = 0; i < buffSize; i += 2)
    {
        int x = (int) MIN( (buff[i] * gridLen),   gridLen - 1);
        int y = (int) MIN( (buff[i+1] * gridLen), gridLen - 1);
        in[y * gridLen + x]++;
    }
}
