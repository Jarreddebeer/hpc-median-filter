#include <stdio.h>
#include "qs.helpers.h"

// perform serial median filter
// Uses O(n) quick select function

void median_filter(
    int* in,
    int* out,
    int* window,
    int gridLen,
    int filtLen)
{
    int filtRad = filtLen / 2;
    for (int y = 0; y < gridLen; y++)
    {
        for (int x = 0; x < gridLen; x++)
        {
            int w_idx = 0;
            for (int dy = -filtRad; dy <= filtRad; dy++)
            {
                for (int dx = -filtRad; dx <= filtRad; dx++)
                {
                    // gather the values in the window
                    int gy, gx;
                    if (y + dy < 0) {gy = 0;}
                    else if (y + dy >= gridLen) {gy = gridLen - 1;}
                    else {gy = y + dy;}
                    if (x + dx < 0) {gx = 0;}
                    else if (x + dx >= gridLen) {gx = gridLen - 1;}
                    else {gx = x + dx;}
                    window[w_idx] = in[gy * gridLen + gx];
                    w_idx++;
                }
            }
            int median = getMedian(window, filtLen * filtLen);
            out[y * gridLen + x] = median;
        }
    }
}
