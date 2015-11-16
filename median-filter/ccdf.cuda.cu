#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BLOCKSIZE 16

extern "C" {
#include "ccdf.h"
}

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true) {
   if (code != cudaSuccess) {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

__device__ int get_median_ccdf(
    int* sm,
    int sidx,
    int filt_rad,
    int sm_filt_len)
{
    int min = sm[sidx];
    int max = sm[sidx];
    // get min and max
    for (int i = -filt_rad; i <= filt_rad; i++)
    {
        for (int j = -filt_rad; j <= filt_rad; j++)
        {
            if (sm[sidx + i * sm_filt_len + j] < min)
                min = sm[sidx + i * sm_filt_len + j];
            else if (sm[sidx + i * sm_filt_len + j] > max)
                max = sm[sidx + i * sm_filt_len + j];
        }
    }
    // binary search on values for pivot
    int mid = (min + max) / 2;
    int count;
    while (min < max)
    {
        count = 0;
        for (int i = -filt_rad; i <= filt_rad; i++)
        {
            for (int j = -filt_rad; j <= filt_rad; j++)
            {
                if (sm[sidx + i * sm_filt_len + j] < mid) count++;
            }
        }
        if (count == (filt_rad / 2)) break;
        else if (count < (filt_rad / 2)) min = mid + 1;
        else max = mid - 1;
        mid = (min + max) / 2;
    }
    // get index of closest item in window
    int median = sm[sidx];
    int dist = abs(median - mid);
    for (int i = -filt_rad; i <= filt_rad; i++)
    {
        for (int j = -filt_rad; j <= filt_rad; j++)
        {
            int dist2 = abs(sm[sidx + i * sm_filt_len + j] - mid);
            if (dist2 < dist)
            {
                 median = sm[sidx + i * sm_filt_len + j];
                 dist = dist2;
            }
        }
    }
    // return median
    return median;
}

__global__ void median_filter_ccdf(
    int* d_in,
    int* d_out,
    int gridLen,
    int grid_filt_len,
    int filt_rad,
    int sm_filt_len)
{
    extern __shared__ int sm[];
    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int sidx = sm_filt_len * (filt_rad + ty) + (filt_rad + tx);

    int x = blockDim.x * blockIdx.x + tx;
    int y = blockDim.y * blockIdx.y + ty;
    int idx = grid_filt_len * (filt_rad + y) + (filt_rad + x);

    __syncthreads();

    if (x < gridLen && y < gridLen)
    {

        // load pixels into shared memory, padded on host
        sm[sidx] = d_in[idx];

        // border pixels
        if (tx == 0)
        { // left border
            if (ty == 0)
            { // top corner
                for (int i = 1; i <= filt_rad; i++) for (int j = 1; j <= filt_rad; j++)
                {
                    sm[sidx - i - j * sm_filt_len] = d_in[idx - i - j * grid_filt_len];
                }
            }
            else if (ty == blockDim.y - 1 || y == gridLen - 1);
            { // bottom corner
                for (int i = 1; i <= filt_rad; i++) for (int j = 1; j <= filt_rad; j++)
                {
                    sm[sidx - i + j * sm_filt_len] = d_in[idx - i + j * grid_filt_len];
                }
            }
            for (int i = 1; i <= filt_rad; i++)
            { // left padding
                sm[sidx - i] = d_in[idx - i];
            }
        }
        else if (tx == blockDim.x - 1 || x == gridLen - 1);
        { // right border
            if (ty == 0)
            { // top corner
                for (int i = 1; i <= filt_rad; i++) for (int j = 1; j <= filt_rad; j++)
                {
                    sm[sidx + i - j * sm_filt_len] = d_in[idx + i - j * grid_filt_len];
                }
            }
            else if (ty == blockDim.y - 1 || y == gridLen - 1);
            { // bottom corner
                for (int i = 1; i <= filt_rad; i++) for (int j = 1; j <= filt_rad; j++)
                {
                    sm[sidx + i + j * sm_filt_len] = d_in[idx + i + j * grid_filt_len];
                }
            }
            for (int i = 1; i <= filt_rad; i++)
            { // right padding
                sm[sidx + i] = d_in[idx + i];
            }
        }
        if (ty == 0)
        { // top border
            for (int i = 1; i <= filt_rad; i++)
            { // top padding
                sm[sidx - i * sm_filt_len] = d_in[idx - i * grid_filt_len];
            }
        }
        else if (ty == blockDim.y - 1 || y == gridLen - 1);
        { // bottom border
            for (int i = 1; i <= filt_rad; i++)
            { // bottom padding
                sm[sidx + i * sm_filt_len] = d_in[idx + i * grid_filt_len];
            }
        }

        __syncthreads();

        /*
        if (ty == 0 && tx == 0)
        {
            printf("---\n");
            for (int i = 0; i < sm_filt_len; i++)
            {
                for (int j = 0; j < sm_filt_len; j++)
                {
                    printf("%d ", sm[i * sm_filt_len + j]);
                }
                printf("\n");
            }
            printf("---\n");
        }
        __syncthreads();
        */

        d_out[y * gridLen + x] = get_median_ccdf(sm, sidx, filt_rad, sm_filt_len);

    }

}

// ccdf based median filter
void median_filter(
    int* in,
    int* h_out,
    int gridLen,
    int filtLen)
{

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start);

    int filt_rad = filtLen / 2;
    int grid_filt_len = gridLen + filt_rad * 2;
    int grid_filt_px = grid_filt_len * grid_filt_len;
    int grid_filt_bytes = grid_filt_px * sizeof(int);

    int grid_px = gridLen * gridLen;
    int grid_bytes = grid_px * sizeof(int);

    // pad memory on the host before copying to the GPU
    int* h_in = (int*) malloc(grid_filt_bytes);
    memset(h_in, 0, grid_filt_bytes);
    int dest_offset = filt_rad * 2 + gridLen;
    int dest_i = dest_offset * filt_rad + filt_rad;
    int src_bytes = gridLen * sizeof(int);
    for (int src_i = 0; src_i < gridLen * gridLen; src_i += gridLen, dest_i += dest_offset)
    {
        memcpy(&h_in[dest_i], &in[src_i], src_bytes);
    }

    /*
    printf("H_IN, grid_filt_len: %d, filt_rad: %d\n", grid_filt_len, filt_rad);
    for (int i = 0; i < grid_filt_len; i++)
    {
        for (int j = 0; j < grid_filt_len; j++)
        {
            printf("%d ", h_in[i * grid_filt_len + j]);
        }
        printf("\n");
    }
    printf("END H_IN\n");
    */

    // copy to GPU device and execute
    int* d_in  = NULL;
    int* d_out = NULL;
    gpuErrchk( cudaMalloc(&d_in, grid_filt_bytes) );
    gpuErrchk( cudaMalloc(&d_out, grid_bytes) );
    gpuErrchk( cudaMemcpy(d_in, h_in, grid_filt_bytes, cudaMemcpyHostToDevice) );

    int blocksize = min(BLOCKSIZE, gridLen);

    dim3 dimBlock = dim3(blocksize, blocksize, 1);
    dim3 dimGrid = dim3(
        ceil( ((int)gridLen) / (float) dimBlock.x),
        ceil( ((int)gridLen) / (float) dimBlock.y),
        1
    );

    int sm_filt_len = blocksize + 2 * filt_rad;
    int sm_bytes = sm_filt_len * sm_filt_len * sizeof(int);

    median_filter_ccdf<<<dimGrid, dimBlock, sm_bytes>>>(
        d_in, d_out, gridLen, grid_filt_len, filt_rad, sm_filt_len
    );
    gpuErrchk( cudaPeekAtLastError() );

    gpuErrchk( cudaMemcpy(h_out, d_out, grid_bytes, cudaMemcpyDeviceToHost) );

    /*
    printf("H_OUT, grid_filt_len: %d, filt_rad: %d\n", grid_filt_len, filt_rad);
    for (int i = 0; i < grid_filt_len; i++)
    {
        for (int j = 0; j < grid_filt_len; j++)
        {
            printf("%d ", h_out[i * grid_filt_len + j]);
        }
        printf("\n");
    }
    printf("END H_OUT\n");
    */

    cudaFree(d_in);
    cudaFree(d_out);
    free(h_in);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    printf("%f", milliseconds);
}
