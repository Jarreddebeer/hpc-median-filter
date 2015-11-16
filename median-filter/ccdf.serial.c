#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ccdf.h"

int get_pivot_ccdf(
    int* in,
    int grid_filt_len,
    int filt_rad,
    int idx)
{
    int min = in[idx];
    int max = in[idx];
    // get min and max
    for (int i = -filt_rad; i <= filt_rad; i++)
    {
        for (int j = -filt_rad; j <= filt_rad; j++)
        {
            int val = in[idx + i * grid_filt_len + j];
            if (val < min) min = val;
            else if (val > max) max = val;
        }
    }
    // binary search on values for pivot
    int mid = (min + max) / 2;
    int count;
    int filt_rad_mid = filt_rad / 2;
    while (min < max)
    {
        count = 0;
        for (int i = -filt_rad; i <= filt_rad; i++)
        {
            for (int j = -filt_rad; j <= filt_rad; j++)
            {
                if (in[idx + i * grid_filt_len + j] < mid) count++;
            }
        }
        if (count == filt_rad_mid) break;
        else if (count < filt_rad_mid) min = mid + 1;
        else max = mid - 1;
        mid = (min + max) / 2;
    }
    // get index of closest item in window
    int median = in[idx];
    int dist = abs(median - mid);
    for (int i = -filt_rad; i <= filt_rad; i++)
    {
        for (int j = -filt_rad; j <= filt_rad; j++)
        {
            int dist2 = abs(in[idx + i * grid_filt_len + j] - mid);
            if (dist2 < dist)
            {
                 median = in[idx + i * grid_filt_len + j];
                 dist = dist2;
            }
        }
    }
    // return median
    return median;
}

void median_filter_ccdf(
    int* in,
    int* out,
    int gridLen,
    int grid_filt_len,
    int filt_rad)
{
    for (int y = 0; y < gridLen; y++)
    {
        for (int x = 0; x < gridLen; x++)
        {
            int idx = (y + filt_rad) * grid_filt_len + (x + filt_rad);
            int median = get_pivot_ccdf(in, grid_filt_len, filt_rad, idx);
            out[y * gridLen + x] = median;
        }
    }
}

// ccdf based median filter
void median_filter(
    int* in,
    int* out,
    int gridLen,
    int filtLen)
{
    int filt_rad = filtLen / 2;
    int grid_filt_len = gridLen + filt_rad * 2;
    int grid_filt_px = grid_filt_len * grid_filt_len;
    int grid_filt_bytes = grid_filt_px * sizeof(int);

    int grid_px = gridLen * gridLen;
    int grid_bytes = grid_px * sizeof(int);

    // pad memory on the host
    int* h_in = (int*) malloc(grid_filt_bytes);
    memset(h_in, 0, grid_filt_bytes);
    int dest_offset = filt_rad * 2 + gridLen;
    int dest_i = dest_offset * filt_rad + filt_rad;
    int src_bytes = gridLen * sizeof(int);
    for (int src_i = 0; src_i < gridLen * gridLen; src_i += gridLen, dest_i += dest_offset)
    {
        memcpy(&h_in[dest_i], &in[src_i], src_bytes);
    }

    median_filter_ccdf(
        h_in, out, gridLen, grid_filt_len, filt_rad
    );

    free(h_in);

}
